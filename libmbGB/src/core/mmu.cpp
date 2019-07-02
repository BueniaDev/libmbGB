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

	joypad = 0xCF;

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
	    return rom[addr];
	}
	else if (addr < 0xA000)
	{
	    return vram[addr - 0x8000];
	}
	else if (addr < 0xC000)
	{
	    return 0xFF;
	}
	else if (addr < 0xD000)
	{
	    return wram[addr - 0xC000];
	}
	else if (addr < 0xE000)
	{
	    return wram[addr - 0xC000];
	}
	else if (addr < 0xFEA0)
	{
	    return wram[addr - 0xE000];
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
	if (addr < 0x4000)
	{
	    return;
	}
	else if (addr < 0x8000)
	{
	    return;
	}
	else if (addr < 0xA000)
	{
	    vram[addr - 0x8000] = value;
	}
	else if (addr < 0xC000)
	{
	    return;
	}
	else if (addr < 0xD000)
	{
	    wram[addr - 0xC000] = value;
	}
	else if (addr < 0xE000)
	{
	    wram[addr - 0xC000] = value;
	}
	else if (addr < 0xFEA0)
	{
	    wram[addr - 0xE000] = value;
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
	    case 0x0F: temp = (interruptflags | 0xE0); break;
	    case 0x40: temp = lcdc; break;
	    case 0x41: temp = (stat | 0x80); break;
	    case 0x42: temp = scrolly; break;
	    case 0x43: temp = scrollx; break;
	    case 0x44: temp = ly; break;
	    case 0x45: temp = lyc; break;
	    case 0x47: temp = bgpalette; break;
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
	    case 0x0F: writeif(value); break;
	    case 0x40: writelcdc(value); break;
	    case 0x41: writestat(value); break;
	    case 0x42: scrolly = value; break;
	    case 0x43: scrollx = value; break;
	    case 0x44: break; // LY should not be written to
	    case 0x45: lyc = value; break;
	    case 0x47: bgpalette = value; break;
	    default: break;
	}
    }

    string MMU::determinegametitle(vector<uint8_t>& rom)
    {
	stringstream temp;
	
	for (int i = 0x134; i < 0x0143; i++)
	{
	    temp << ((char)(int)(rom[i]));
	}

	return temp.str();
    }

    void MMU::determineramsize(vector<uint8_t>& rom)
    {
	switch (rom[0x0149])
	{
	    case 0: ramsize = 0; break;
	    case 1: ramsize = 0x800; break;
	    case 2: ramsize = 0x2000; break;
	    case 3: ramsize = 0x8000; break;
	    case 4: ramsize = 0x20000; break;
	    case 5: ramsize = 0x10000; break;
	    default: cout << "MMU::Error - Unrecognzied RAM quantity given in cartridge" << endl; exit(1); break;
	}
    }

    void MMU::determinembctype(vector<uint8_t>& rom)
    {
	switch (rom[0x0147])
	{
	    case 0: gbmbc = MBCType::None; externalrampres = false; mbctype = "ROM ONLY"; break;
	    case 1: gbmbc = MBCType::None; externalrampres = false; mbctype = "ROM ONLY"; break; // Hack to get test ROMS running, will be fixed later
	    case 2: gbmbc = MBCType::None; externalrampres = false; mbctype = "ROM ONLY"; break; // Hack to get test ROMS running, will be fixed later
	    case 8: gbmbc = MBCType::None; externalrampres = true; mbctype = "ROM + RAM"; break;
	    case 9: gbmbc = MBCType::None; externalrampres = true; mbctype = "ROM + RAM + BATTERY"; break;
	    default: cout << "MMU::Error - Unrecognized MBC type" << endl; exit(1); break;
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
	    int rombanks = ((0x8000 << cartmem[0x0148]) / 0x4000);
	    cout << "ROM banks: " << rombanks << endl;

	    if (size != (rombanks * 0x4000))
	    {
		cout << "MMU::Warning - Size of ROM does not match size in cartridge header." << endl;
	    }

	    determineramsize(cartmem);

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
