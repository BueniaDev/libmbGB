// This file is part of libmbGB.
// Copyright (C) 2019 Buenia.
//
// libmbGB is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// libmbGB is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with libmbGB.  If not, see <https://www.gnu.org/licenses/>.

#include "../../include/libmbGB/mmu.h"
using namespace gb;

namespace gb
{
    MMU::MMU()
    {
    }

    MMU::~MMU()
    {

    }

    void MMU::init()
    {
	rom.resize(0x8000, 0);
	vram.resize(0x4000, 0);
	sram.resize(0x2000, 0);
	wram.resize(0x8000, 0);
	oam.resize(0xA0, 0);
	hram.resize(0x7F, 0);
	rambanks.resize(0x8000, 0);

	joypad = 0xCF;
	divider = 0xABCC;

	cout << "MMU::Initialized" << endl;
    }

    void MMU::shutdown()
    {
	rom.clear();
	vram.clear();
	sram.clear();
	wram.clear();
	oam.clear();
	hram.clear();
	rambanks.clear();
	
	cout << "MMU::Shutting down..." << endl;
    }

    uint8_t MMU::readByte(uint16_t addr)
    {
	if (addr < 0x4000)
	{
	    if ((biosload) && (addr < 0x100))
	    {
		return bios[addr];
	    }
	    else if ((biosload) && (gameboy == Console::CGB) && ((addr >= 0x200) && (addr < 0x900)))
	    {
		return bios[addr];
	    }
	    else if (biosload && addr == 0x100)
	    {
		exitbios();
		return rom[addr];
	    }
	    else
	    {
		return rom[addr];
	    }
	}
	else if (addr < 0x8000)
	{
	    uint8_t temp = 0;

	    switch (gbmbc)
	    {
		case MBCType::None: temp = rom[addr]; break;
		case MBCType::MBC1: temp = mbc1read(addr); break;
		case MBCType::MBC2: temp = 0xFF; break;
	    }

	    return temp;
	}
	else if (addr < 0xA000)
	{
	    return vram[addr - 0x8000];
	}
	else if (addr < 0xC000)
	{
	    uint8_t temp = 0;

	    switch (gbmbc)
	    {
		case MBCType::None: temp = 0xFF; break;
		case MBCType::MBC1: temp = mbc1read(addr); break;
		case MBCType::MBC2: temp = 0xFF; break;
	    }

	    return temp;
	}
	else if (addr < 0xD000)
	{
	    return wram[addr - 0xC000];
	}
	else if (addr < 0xE000)
	{
	    return wram[addr - 0xC000];
	}
	else if (addr < 0xFE00)
	{
	    return wram[addr - 0xE000];
	}
	else if (addr < 0xFEA0)
	{
	    return oam[addr - 0xFE00];
	}
	else if (addr < 0xFF00)
	{
	    return 0x00;
	}
	else if (addr < 0xFF80)
	{
	    return readIO(addr);
	}
	else if (addr < 0xFFFF)
	{
	    return hram[addr - 0xFF80];
	}
	else
	{
	    return interruptenabled;
	}
    }

    void MMU::writeByte(uint16_t addr, uint8_t value)
    {
	if (addr < 0x8000)
	{
	    switch (gbmbc)
	    {
		case MBCType::None: return; break;
		case MBCType::MBC1: mbc1write(addr, value); break;
		case MBCType::MBC2: return; break;
	    }
	}
	else if (addr < 0xA000)
	{
	    vram[addr - 0x8000] = value;
	}
	else if (addr < 0xC000)
	{
	    switch (gbmbc)
	    {
		case MBCType::None: return; break;
		case MBCType::MBC1: mbc1write(addr, value); break;
		case MBCType::MBC2: return; break;
	    }
	}
	else if (addr < 0xD000)
	{
	    wram[addr - 0xC000] = value;
	}
	else if (addr < 0xE000)
	{
	    wram[addr - 0xC000] = value;
	}
	else if (addr < 0xFE00)
	{
	    wram[addr - 0xE000] = value;
	}
	else if (addr < 0xFEA0)
	{
	    oam[addr - 0xFE00] = value;
	}
	else if (addr < 0xFF00)
	{
	    return;
	}
	else if (addr < 0xFF80)
	{
	    writeIO(addr, value);
	}
	else if (addr < 0xFFFF)
	{
	    hram[addr - 0xFF80] = value;
	}
	else
	{
	    interruptenabled = value;
	}
    }

    uint16_t MMU::readWord(uint16_t addr)
    {
	return ((readByte(addr + 1) << 8) | (readByte(addr)));
    }

    void MMU::writeWord(uint16_t addr, uint16_t val)
    {
	writeByte(addr , (val & 0xFF));
	writeByte((addr + 1), (val >> 8));
    }

    uint8_t MMU::readIO(uint16_t addr)
    {
	uint8_t temp = 0;
	
	switch ((addr & 0xFF))
	{
	    case 0x00: temp = (joypad | 0xC0); break;
	    case 0x04: temp = (divider >> 8); break;
	    case 0x05: temp = timercounter; break;
	    case 0x06: temp = timermodulo; break;
	    case 0x07: temp = (timercontrol | 0xF8); break;
	    case 0x0F: temp = (interruptflags | 0xE0); break;
	    case 0x40: temp = lcdc; break;
	    case 0x41: temp = (stat | 0x80); break;
	    case 0x42: temp = scrolly; break;
	    case 0x43: temp = scrollx; break;
	    case 0x44: temp = ly; break;
	    case 0x45: temp = lyc; break;
	    case 0x47: temp = bgpalette; break;
	    case 0x48: temp = objpalette0; break;
	    case 0x49: temp = objpalette1; break;
	    case 0x4A: temp = windowy; break;
	    case 0x4B: temp = windowx; break;
	    default: temp = 0xFF; break;
	}
	
	return temp;
    }

    void MMU::writeIO(uint16_t addr, uint8_t value)
    {
	switch ((addr & 0xFF))
	{
	    case 0x00: writejoypad(value); break;
	    case 0x01: sb = value; break;
	    case 0x02: writesc(value); break;
	    case 0x04: writediv(); break;
	    case 0x05: timercounter = value; break;
	    case 0x06: timermodulo = value; break;
	    case 0x07: timercontrol = (value & 0x07); break;
	    case 0x0F: writeif(value); break;
	    case 0x40: lcdc = value; break;
	    case 0x41: writestat(value); break;
	    case 0x42: scrolly = value; break;
	    case 0x43: scrollx = value; break;
	    case 0x44: break; // LY should not be written to
	    case 0x45: lyc = value; break;
	    case 0x46: dodmatransfer(value); break;
	    case 0x47: bgpalette = value; break;
	    case 0x48: objpalette0 = value; break;
	    case 0x49: objpalette1 = value; break;
	    case 0x4A: windowy = value; break;
	    case 0x4B: windowx = value; break;
	    default: break;
	}
    }

    bool MMU::loadROM(string filename)
    {
	cout << "MMU::Loading ROM " << filename << "..." << endl;
	ifstream file(filename.c_str(), ios::in | ios::binary | ios::ate);

	if (file.is_open())
	{
	    streampos size = file.tellg();

	    cartmem.resize(size, 0);

	    file.seekg(0, ios::beg);
	    file.read((char*)&cartmem[0], size);
	    file.close();

	    bool cgbflag = ((cartmem[0x0143] == 0xC0) || (cartmem[0x0143] == 0x80));

	    if (gameboy == Console::Default)
	    {
		if (cgbflag)
		{
		    gameboy = Console::CGB;
		}
		else
		{
		    gameboy = Console::DMG;
		}
	    }
	    

	    if (gameboy == Console::CGB && cgbflag)
	    {
		gbmode = Mode::CGB;
	    }
	    else
	    {
		gbmode = Mode::DMG;
	    }

	    cout << "Title: " << determinegametitle(cartmem) << endl;
	    determinembctype(cartmem);
	    cout << "MBC type: " << mbctype << endl;
	    numrombanks = getrombanks(cartmem);
	    cout << "ROM size: " << romsize << endl;
	    numrambanks = getrambanks(cartmem);
	    cout << "RAM size: " << ramsize << endl;

	    if (gbmbc != MBCType::None && size != (numrombanks * 0x4000))
	    {
		cout << "MMU::Warning - Size of ROM does not match size in cartridge header." << endl;
	    }

	    for (int i = 0; i < 0x8000; i++)
	    {
		rom[i] = cartmem[i];
	    }

	    cout << "MMU::" << filename << " succesfully loaded." << endl;
	    return true;
	}
	else
	{
	    cout << "MMU::Error - " << filename << " could not be opened." << endl;
	    return false;
	}
    }

    bool MMU::loadBIOS(string filename)
    {
	cout << "MMU::Loading BIOS..." << endl;
	ifstream file(filename.c_str(), ios::in | ios::binary | ios::ate);

	if (file.is_open())
	{
	    streampos size = file.tellg();

	    if (size == 0x100)
	    {
		gameboy = Console::DMG;
		bios.resize(0x100, 0);
	    }
	    else if (size == 0x900)
	    {
		gameboy = Console::CGB;
		bios.resize(0x900, 0);
	    }
	    else
	    {
		cout << "MMU::Error - BIOS size does not match sizes of the official BIOS." << endl;
		return false;
	    }

	    file.seekg(0, ios::beg);
	    file.read((char*)&bios[0], size);
	    cout << "MMU::BIOS succesfully loaded." << endl;
	    file.close();
	    return true;
	}
	else
	{
	    cout << "MMU::Error - BIOS could not be opened." << endl;
	    return false;
	}
    }
};
