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
	gbcbgpalette.resize(0x40, 0);
	gbcobjpalette.resize(0x40, 0);

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

    bool MMU::loadmmu(int offset, string filename)
    {
	vram.clear();
	sram.clear();
	wram.clear();
	oam.clear();
	hram.clear();
	rambanks.clear();

	vram.resize(0x4000, 0);
	sram.resize(0x2000, 0);
	wram.resize(0x8000, 0);
	oam.resize(0xA0, 0);
	hram.resize(0x7F, 0);
	rambanks.resize(0x8000, 0);
	gbcbgpalette.resize(0x40, 0);
	gbcobjpalette.resize(0x40, 0);
	
	ifstream file(filename.c_str(), ios::binary);

	if (!file.is_open())
	{
	    cout << "CPU::Error opening CPU state" << endl;
	    return false;
	}

	cout << hex << (int)(interruptenabled) << endl;

	file.seekg(offset);

	file.read((char*)&cartmem[0], sizeof(cartmem));
	file.read((char*)&rambanks[0], 0x8000);
	file.read((char*)&vram[0], 0x4000);
	file.read((char*)&wram[0], 0x8000);
	file.read((char*)&oam[0], 0xA0);
	file.read((char*)&hram[0], 0x7F);
	readio(file);
	file.read((char*)&gbcbgpalette[0], 0x40);
	file.read((char*)&gbcobjpalette[0], 0x40);
	file.read((char*)&doublespeed, sizeof(doublespeed));
	file.read((char*)&currentrombank, sizeof(currentrombank));
	file.read((char*)&currentrambank, sizeof(currentrambank));
	file.read((char*)&higherrombankbits, sizeof(higherrombankbits));
	file.read((char*)&rommode, sizeof(rommode));
	file.read((char*)&ramenabled, sizeof(ramenabled));
	cout << hex << (int)(interruptenabled) << endl;
	file.close();
	return true;
    }

    bool MMU::savemmu(string filename)
    {
	ofstream file(filename.c_str(), ios::binary | ios::app);

	if (!file.is_open())
	{
	    cout << "CPU::Error opening CPU state" << endl;
	    return false;
	}

	file.write((char*)&cartmem[0], sizeof(cartmem));
	file.write((char*)&rambanks[0], 0x8000);
	file.write((char*)&vram[0], 0x4000);
	file.write((char*)&wram[0], 0x8000);
	file.write((char*)&oam[0], 0xA0);
	file.write((char*)&hram[0], 0x7F);
	writeio(file);
	file.write((char*)&gbcbgpalette[0], 0x40);
	file.write((char*)&gbcobjpalette[0], 0x40);
	file.write((char*)&doublespeed, sizeof(doublespeed));
	file.write((char*)&currentrombank, sizeof(currentrombank));
	file.write((char*)&currentrambank, sizeof(currentrambank));
	file.write((char*)&higherrombankbits, sizeof(higherrombankbits));
	file.write((char*)&rommode, sizeof(rommode));
	file.write((char*)&ramenabled, sizeof(ramenabled));	
	cout << hex << (int)(interruptenabled) << endl;
	file.close();
	return true;
    }

    bool MMU::loadbackup(string filename)
    {
	bool success = false;	

	if (batteryenabled)
	{
	    fstream sram(filename.c_str(), ios::in | ios::binary);

	    if (!sram.is_open())
	    {
		cout << "MMU::Save data could not be loaded." << endl;
		success = false;
	    }
	    else
	    {
		if (gbmbc != MBCType::None && batteryenabled)
		{
		    sram.read((char*)&rambanks[0], 0x8000);
		    cout << "MMU::Save data succesfully loaded." << endl;
		    sram.close();
		    success = true;
		}
		else
		{
		    success = false;
		}
	    }
	}

	return success;
    }

    bool MMU::savebackup(string filename)
    {
	bool success = false;
	
	if (batteryenabled)
	{
	    fstream sram(filename.c_str(), ios::out | ios::binary);

	    if (!sram.is_open())
	    {
		cout << "MMU::Save data could not be written." << endl;
		success = false;
	    }
	    else
	    {
		if (gbmbc != MBCType::None && batteryenabled)
		{
		    sram.write((char*)&rambanks[0], 0x8000);
		    cout << "MMU::Save data succesfully stored." << endl;
		    sram.close();
		    success = true;
		}
		else
		{
		    success = false;
		}
	    }
	}

	return success;
    }

    uint8_t MMU::readByte(uint16_t addr)
    {
	if (addr < 0x4000)
	{
	    if (biosload == true)
	    {
		if ((biossize == 0x900) && (addr > 0x100) && (addr < 0x200))
		{
		    return rom[addr];
		}
		else if (addr == 0x100)
		{
		    exitbios();
		    return rom[addr];
		}
		else if (addr < biossize)
		{
		    return bios[addr];
		}
		else
		{
		    return rom[addr];
		}
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
		case MBCType::MBC2: temp = mbc2read(addr); break;
		case MBCType::MBC3: temp = mbc3read(addr); break;
		case MBCType::MBC5: temp = mbc5read(addr); break;
	    }

	    return temp;
	}
	else if (addr < 0xA000)
	{
	    return vram[(addr - 0x8000) + (vrambank * 0x2000)];
	}
	else if (addr < 0xC000)
	{
	    uint8_t temp = 0;

	    switch (gbmbc)
	    {
		case MBCType::None: temp = 0xFF; break;
		case MBCType::MBC1: temp = mbc1read(addr); break;
		case MBCType::MBC2: temp = mbc2read(addr); break;
		case MBCType::MBC3: temp = mbc3read(addr); break;
		case MBCType::MBC5: temp = mbc5read(addr); break;
	    }

	    return temp;
	}
	else if (addr < 0xD000)
	{
	    return wram[addr - 0xC000];
	}
	else if (addr < 0xE000)
	{
	    uint16_t offset = (0x1000 * ((wrambank == 0) ? 0 : (wrambank -1)));
	    return wram[addr - 0xC000 + offset];
	}
	else if (addr < 0xF000)
	{
	    return wram[addr - 0xE000];
	}
	else if (addr < 0xFE00)
	{
	    uint16_t offset = (0x1000 * ((wrambank == 0) ? 0 : (wrambank -1)));
	    return wram[addr - 0xE000 + offset];
	}
	else if (addr < 0xFEA0)
	{
	    return oam[addr - 0xFE00];
	}
	else if (addr < 0xFF00)
	{
	    return 0x00;
	}
	else if (addr < 0xFF30)
	{
	    return readIO(addr);
	}
	else if (addr < 0xFF40)
	{
	    return waveram[addr - 0xFF30];
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
		case MBCType::MBC2: mbc2write(addr, value); break;
		case MBCType::MBC3: mbc3write(addr, value); break;
		case MBCType::MBC5: mbc5write(addr, value); break;
	    }
	}
	else if (addr < 0xA000)
	{
	    vram[(addr - 0x8000) + (vrambank * 0x2000)] = value;
	}
	else if (addr < 0xC000)
	{
	    switch (gbmbc)
	    {
		case MBCType::None: return; break;
		case MBCType::MBC1: mbc1write(addr, value); break;
		case MBCType::MBC2: mbc2write(addr, value); break;
		case MBCType::MBC3: mbc3write(addr, value); break;
		case MBCType::MBC5: mbc5write(addr, value); break;
	    }
	}
	else if (addr < 0xD000)
	{
	    wram[addr - 0xC000] = value;
	}
	else if (addr < 0xE000)
	{
	    uint16_t offset = (0x1000 * ((wrambank == 0) ? 0 : (wrambank -1)));
	    wram[addr - 0xC000 + offset] = value;
	}
	else if (addr < 0xF000)
	{
	    wram[addr - 0xE000] = value;
	}
	else if (addr < 0xFE00)
	{
	    uint16_t offset = (0x1000 * ((wrambank == 0) ? 0 : (wrambank -1)));
	    wram[addr - 0xE000 + offset] = value;
	}
	else if (addr < 0xFEA0)
	{
	    oam[addr - 0xFE00] = value;
	}
	else if (addr < 0xFF00)
	{
	    return;
	}
	else if (addr < 0xFF30)
	{
	    writeIO(addr, value);
	}
	else if (addr < 0xFF40)
	{
	    waveram[addr - 0xFF30] = value;
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
	    case 0x01: temp = sb; break;
	    case 0x02: temp = readsc(); break;
	    case 0x04: temp = (divider >> 8); break;
	    case 0x05: temp = tima; break;
	    case 0x06: temp = tma; break;
	    case 0x07: temp = (tac | 0xF8); break;
	    case 0x0F: temp = (interruptflags | 0xE0); break;
	    case 0x10: temp = ((s1sweepshift) | (s1sweepnegate << 3) | (s1sweepperiodload << 4) | 0x80); break;
	    case 0x11: temp = (((s1lengthload & 0x3F) | ((s1duty & 0x3) << 6)) | 0x3F); break;
	    case 0x12: temp = ((s1envperiodload & 0x7) | (s1envaddmode << 3) | ((s1volumeload & 0xF) << 4)); break;
	    case 0x13: temp = ((s1freq & 0xFF) | 0xFF); break;
	    case 0x14: temp = ((((s1freq >> 8) & 0x7) | (s1lengthenabled << 6) | (s1triggerbit << 7)) | 0xBF); break;
	    case 0x16: temp = (((s2lengthload & 0x3F) | ((s2duty & 0x3) << 6)) | 0x3F); break;
	    case 0x17: temp = ((s2envperiodload & 0x7) | (s2envaddmode << 3) | ((s2volumeload & 0xF) << 4)); break;
	    case 0x18: temp = ((s2freq & 0xFF) | 0xFF); break;
	    case 0x19: temp = ((((s2freq >> 8) & 0x7) | (s2lengthenabled << 6) | (s2triggerbit << 7)) | 0xBF); break;
	    case 0x1A: temp = ((wavedacenabled) << 7); break;
	    case 0x1B: temp = wavelengthload; break;
	    case 0x1C: temp = (wavevolumecode << 5); break;
	    case 0x1D: temp = (wavefreq & 0xFF); break;
	    case 0x1E: temp = (((wavefreq >> 8) & 0x7) | (wavelengthenabled << 6) | (wavetriggerbit << 7)); break;
	    case 0x24: temp = ((rightvol) | (vinrightenable << 3) | (leftvol << 4) | (vinleftenable << 7)); break;
	    case 0x25:
	    {
		for (int i = 0; i < 4; i++)
		{
		    temp |= (rightenables[i] << i);
		}

		for (int i = 0; i < 4; i++)
		{
		    temp |= (leftenables[i] << (i + 4));
		}
	    }
	    break;
	    case 0x26: temp = getsoundenabled(); break;
	    case 0x40: temp = lcdc; break;
	    case 0x41: temp = (stat | 0x80); break;
	    case 0x42: temp = scrolly; break;
	    case 0x43: temp = scrollx; break;
	    case 0x44: temp = ly; break;
	    case 0x45: temp = lyc; break;
	    case 0x46: temp = dma; break;
	    case 0x47: temp = bgpalette; break;
	    case 0x48: temp = objpalette0; break;
	    case 0x49: temp = objpalette1; break;
	    case 0x4A: temp = windowy; break;
	    case 0x4B: temp = windowx; break;
	    case 0x4D: temp = key1; break;
	    case 0x4F: temp = (vrambank | 0xFE); break;
	    case 0x51: temp = (hdmasource >> 8); break;
	    case 0x52: temp = (hdmasource & 0xFF); break;
	    case 0x53: temp = (hdmadest >> 8); break;
	    case 0x54: temp = (hdmadest & 0xFF); break;
	    case 0x55: temp = ((hdmalength & 0x7F) | ((hdmaactive ? 0 : 1) << 7)); break;
	    case 0x68: temp = (isgbcconsole()) ? gbcbgpaletteindex : 0xFF; break;
	    case 0x69: temp = (isgbcconsole()) ? gbcbgpalette[gbcbgpaletteindex] : 0xFF; break;
	    case 0x6A: temp = (isgbcconsole()) ? gbcobjpaletteindex : 0xFF; break;
	    case 0x6B: temp = (isgbcconsole()) ? gbcobjpalette[gbcobjpaletteindex] : 0xFF; break;
	    case 0x70: temp = (wrambank | 0xF8); break;
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
	    case 0x05: tima = value; break;
	    case 0x06: tma = value; break;
	    case 0x07: tac = (value & 0x07); break;
	    case 0x0F: writeif(value); break;
	    case 0x10: 
	    {
		s1sweepshift = (value & 0x7);
		s1sweepnegate = TestBit(value, 3);
		s1sweepperiodload = ((value >> 4) & 0x7);
		s1sweepperiod = s1sweepperiodload;
	    }
	    break;
	    case 0x11: 
	    {
		s1lengthload = (value & 0x3F);
		s1duty = ((value >> 6) & 0x3);
	    }
	    break;
	    case 0x12:
	    {
		s1dacenabled = ((value & 0xF8) != 0);
		s1volumeload = ((value >> 4) & 0xF);
		s1envaddmode = TestBit(value, 3);
		s1envperiodload = (value & 0x7);
		s1envperiod = s1envperiodload;
		s1volume = s1volumeload;
	    }
	    break;
	    case 0x13: s1freq = ((s1freq & 0x700) | (value & 0xFF)); break;
	    case 0x14: 
	    {
		s1freq = (((s1freq & 0xFF) | ((value & 0x7) << 8)));
		s1lengthenabled = TestBit(value, 6);
		s1triggerbit = TestBit(value, 7);

		if (TestBit(value, 7))
		{
		    s1trigger();
		}
	    }
	    break;
	    case 0x16: 
	    {
		s2lengthload = (value & 0x3F);
		s2duty = ((value >> 6) & 0x3);
	    }
	    break;
	    case 0x17:
	    {
		s2dacenabled = ((value & 0xF8) != 0);
		s2volumeload = ((value >> 4) & 0xF);
		s2envaddmode = TestBit(value, 3);
		s2envperiodload = (value & 0x7);
		s2envperiod = s2envperiodload;
		s2volume = s2volumeload;
	    }
	    break;
	    case 0x18: s2freq = ((s2freq & 0x700) | (value & 0xFF)); break;
	    case 0x19: 
	    {
		s2freq = (((s2freq & 0xFF) | ((value & 0x7) << 8)));
		s2lengthenabled = TestBit(value, 6);
		s2triggerbit = TestBit(value, 7);

		if (TestBit(value, 7))
		{
		    s2trigger();
		}
	    }
	    break;
	    case 0x1A: wavedacenabled = TestBit(value, 7); break;
	    case 0x1B: wavelengthload = value; break;
	    case 0x1C: wavevolumecode = ((value >> 5) & 0x3); break;
	    case 0x1D: wavefreq = ((wavefreq & 0x700) | (value)); break;
	    case 0x1E:
	    {
		wavefreq = ((wavefreq & 0xFF) | ((value & 0x7) << 8));
		wavelengthenabled = TestBit(value, 6);
		wavetriggerbit = TestBit(value, 7);

		if (TestBit(value, 7))
		{
		    wavetrigger();
		}
	    }
	    break;
	    case 0x20: noiselengthload = (value & 0x3F); break;
	    case 0x21:
	    {
		noisedacenabled = ((value & 0xF8) != 0);
		noisevolumeload = ((value >> 4) & 0xF);
		noiseenvaddmode = TestBit(value, 3);
		noiseenvperiodload = (value & 0x7);
		noiseenvperiod = noiseenvperiodload;
		noisevolume = noisevolumeload;
	    }
	    break;
	    case 0x22:
	    {
		noisedivisor = (value & 0x7);
		noisewidthmode = TestBit(value, 3);
		noiseclockshift = ((value >> 4) & 0xF);
	    }
	    break;
	    case 0x23:
	    {
		noiselengthenabled = TestBit(value, 6);
		noisetriggerbit = TestBit(value, 7);

		if (TestBit(value, 7))
		{
		    noisetrigger();
		}
	    }
	    break;
	    case 0x24:
	    {
		if (!soundenabled)
		{
		    return;
		}

		rightvol = (value & 0x7);
		vinrightenable = TestBit(value, 3);
		leftvol = ((value >> 4) & 0x7);
		vinleftenable = TestBit(value, 7);
	    }
	    break;
	    case 0x25:
	    {
		if (!soundenabled)
		{
		    return;
		}

		for (int i = 0; i < 4; i++)
		{
		    rightenables[i] = TestBit((value >> i), 0);
		}

		for (int i = 0; i < 4; i++)
		{
		    leftenables[i] = TestBit((value >> (i + 4)), 0);
		}
	    }
	    break;
	    case 0x26: soundenabled = TestBit(value, 7); break;
	    case 0x40: writelcdc(value); break;
	    case 0x41: writestat(value); break;
	    case 0x42: scrolly = value; break;
	    case 0x43: scrollx = value; break;
	    case 0x44: break; // LY should not be written to
	    case 0x45: lyc = value; break;
	    case 0x46: writedma(value); break;
	    case 0x47: bgpalette = value; break;
	    case 0x48: objpalette0 = value; break;
	    case 0x49: objpalette1 = value; break;
	    case 0x4A: windowy = value; break;
	    case 0x4B: windowx = value; break;
	    case 0x4D: key1 = value; break;
	    case 0x4F: 
	    {
		vrambank = (isgbcconsole()) ? BitGetVal(value, 0) : 0;
	    }
	    break;
	    case 0x51:
	    {
		if (!isgbcconsole())
		{
		    return;
		}	

		hdmasource = ((hdmasource & 0xFF) | (value << 8));
	    }
	    break;
	    case 0x52:
	    {
		if (!isgbcconsole())
		{
		    return;
		}	

		hdmasource = ((hdmasource & 0xFF00) | (value & 0xF0));
	    }
	    break;
	    case 0x53:
	    {
		if (!isgbcconsole())
		{
		    return;
		}	

		hdmadest = ((hdmadest & 0xFF) | (((value & 0x1F) | 0x80) << 8));
	    }
	    break;
	    case 0x54:
	    {
		if (!isgbcconsole())
		{
		    return;
		}	

		hdmadest = ((hdmadest & 0xFF00) | (value & 0xF0));
	    }
	    break;
	    case 0x55:
	    {
		if (!isgbcconsole())
		{
		    return;
		}	

		hdmalength = (value & 0x7F);

		if (!TestBit(value, 7) && !hdmaactive)
		{
		    for (int i = 0; i <= (hdmalength & 0x7F); i++)
		    {
			hdmatransfer();
		    }

		    hdmalength = 0xFF;
		    hdmaactive = false;
		}
		else if (!TestBit(value, 7) && hdmaactive)
		{
		    hdmaactive = false;
		}
		else
		{
		    hdmaactive = true;
		}
	    }
	    break;
	    case 0x68:
	    {
		if (!isgbcconsole())
		{
		    return;
		}	

		gbcbgpaletteindex = (value & 0x3F);
		gbcbgpalinc = TestBit(value, 7);
	    }
	    break;
	    case 0x69:
	    {
		if (!isgbcconsole())
		{
		    return;
		}		

		gbcbgpalette[gbcbgpaletteindex] = value;

		if (gbcbgpalinc)
		{
		    gbcbgpaletteindex = ((gbcbgpaletteindex + 1) & 0x3F);
		}
	    }
	    break;
	    case 0x6A:
	    {
		if (!isgbcconsole())
		{
		    return;
		}	

		gbcobjpaletteindex = (value & 0x3F);
		gbcobjpalinc = TestBit(value, 7);
	    }
	    break;
	    case 0x6B:
	    {
		if (!isgbcconsole())
		{
		    return;
		}		

		gbcobjpalette[gbcobjpaletteindex] = value;

		if (gbcbgpalinc)
		{
		    gbcobjpaletteindex = ((gbcobjpaletteindex + 1) & 0x3F);
		}
	    }
	    break;
	    case 0x70: 
	    {
		if (!isgbcconsole())
		{
		    return;
		}		

		wrambank = (value & 0x07);
	    }
	    break;
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
	    

	    if (gameboy == Console::CGB && cgbflag && gbmode == Mode::Default)
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

	    memcpy(&rom[0], &cartmem[0], 0x8000);
	    cout << "MMU::" << filename << " succesfully loaded." << endl;
	    return true;
	}
	else
	{
	    cout << "MMU::Error - " << filename << " could not be opened." << endl;
	    return false;
	}
    }

    bool MMU::loadROM(const char *filename, const uint8_t* buffer, int size)
    {
	cout << "MMU::Loading ROM " << filename << "..." << endl;

	if ((buffer) != NULL)
	{
	    cartmem.resize(size, 0);

	    memcpy(&cartmem[0], &buffer[0], size);

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
	    

	    if (gameboy == Console::CGB && cgbflag && gbmode == Mode::Default)
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

	    memcpy(&rom[0], &cartmem[0], 0x8000);
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

	    biossize = size;

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
