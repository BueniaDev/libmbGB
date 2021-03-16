// This file is part of libmbGB.
// Copyright (C) 2021 Buenia.
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
using namespace std::placeholders;

namespace gb
{
    MMU::MMU()
    {
	addmemoryreadhandler(0xFF03, bind(&MMU::readempty, this, _1));
	addmemorywritehandler(0xFF03, bind(&MMU::writeempty, this, _1, _2));

	for (int i = 8; i < 0xF; i++)
	{
	    addmemoryreadhandler((0xFF00 + i), bind(&MMU::readempty, this, _1));
	    addmemorywritehandler((0xFF00 + i), bind(&MMU::writeempty, this, _1, _2));
	}
    }

    MMU::~MMU()
    {

    }

    void MMU::init()
    {
	vram.resize(0x4000, 0);
	sram.resize(0x2000, 0);
	wram.resize(0x8000, 0);
	oam.resize(0xA0, 0);
	hram.resize(0x7F, 0);
	rambanks.resize(0x10000, 0);
	gbcbgpalette.resize(0x40, 0xFF);
	gbcobjpalette.resize(0x40, 0xFF);
	fill(rom.begin(), rom.end(), 0);
	fill(vram.begin(), vram.end(), 0);
	fill(wram.begin(), wram.end(), 0);
	fill(oam.begin(), oam.end(), 0);
	fill(hram.begin(), hram.end(), 0);

	cout << "MMU::Initialized" << endl;

	gbmode = Mode::Default; // DO NOT DISABLE THIS! Doing so breaks the emulator on reset!
	gameboy = Console::Default; // DO NOT DISABLE THIS! Doing so breaks the emulator on reset!
    }

    void MMU::initvram()
    {
	if (isdmgmode()) 
	{
	    int inittilemap = 0x19;
	    vram[0x1910] = inittilemap--;
	    for (int addr = 0x192F; addr >= 0x1924; --addr) 
	    {
		vram[addr] = inittilemap--;
	    }

	    for (int addr = 0x190F; addr >= 0x1904; --addr) 
	    {
		vram[addr] = inittilemap--;
	    }
	}

        array<uint8_t, 200> inittiledata{{
  	    0xF0, 0xF0, 0xFC, 0xFC, 0xFC, 0xFC, 0xF3, 0xF3,
            0x3C, 0x3C, 0x3C, 0x3C, 0x3C, 0x3C, 0x3C, 0x3C,
            0xF0, 0xF0, 0xF0, 0xF0, 0x00, 0x00, 0xF3, 0xF3,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xCF, 0xCF,
            0x00, 0x00, 0x0F, 0x0F, 0x3F, 0x3F, 0x0F, 0x0F,
            0x00, 0x00, 0x00, 0x00, 0xC0, 0xC0, 0x0F, 0x0F,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xF0, 0xF0,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xF3, 0xF3,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xC0, 0xC0,
            0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0xFF, 0xFF,
            0xC0, 0xC0, 0xC0, 0xC0, 0xC0, 0xC0, 0xC3, 0xC3,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFC, 0xFC,
            0xF3, 0xF3, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0,
            0x3C, 0x3C, 0xFC, 0xFC, 0xFC, 0xFC, 0x3C, 0x3C,
            0xF3, 0xF3, 0xF3, 0xF3, 0xF3, 0xF3, 0xF3, 0xF3,
            0xF3, 0xF3, 0xC3, 0xC3, 0xC3, 0xC3, 0xC3, 0xC3,
            0xCF, 0xCF, 0xCF, 0xCF, 0xCF, 0xCF, 0xCF, 0xCF,
            0x3C, 0x3C, 0x3F, 0x3F, 0x3C, 0x3C, 0x0F, 0x0F,
            0x3C, 0x3C, 0xFC, 0xFC, 0x00, 0x00, 0xFC, 0xFC,
            0xFC, 0xFC, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0,
            0xF3, 0xF3, 0xF3, 0xF3, 0xF3, 0xF3, 0xF0, 0xF0,
            0xC3, 0xC3, 0xC3, 0xC3, 0xC3, 0xC3, 0xFF, 0xFF,
            0xCF, 0xCF, 0xCF, 0xCF, 0xCF, 0xCF, 0xC3, 0xC3,
            0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0xFC, 0xFC,
            0x3C, 0x42, 0xB9, 0xA5, 0xB9, 0xA5, 0x42, 0x3C
        }};

        auto tiledataiter = inittiledata.begin();
        for (int addr = 0x0010; addr < 0x01A0; addr += 2) 
	{
            vram[addr] = *tiledataiter++;
	}
    }

    void MMU::initnot()
    {
	cout << "MMU::Initialized" << endl;
	if (!biosload)
	{
	    initio();
	}
	gbmode = Mode::Default; // DO NOT DISABLE THIS! Doing so breaks the emulator on reset!
    }

    void MMU::shutdown()
    {
	rom.clear();
	vram.clear();
	sram.clear();
	wram.clear();
	oam.clear();
	hram.clear();
	cartmem.clear();
	rambanks.clear();
	
	cout << "MMU::Shutting down..." << endl;
    }

    void MMU::dosavestate(mbGBSavestate &file)
    {
	file.section("MMU ");

	file.vararray(rambanks.data(), rambanks.size());
	file.vararray(vram.data(), 0x4000);
	file.vararray(wram.data(), 0x8000);
	file.vararray(oam.data(), 0xA0);
	file.vararray(hram.data(), 0x7F);
	file.vararray(gbcbgpalette.data(), 0x40);
	file.vararray(gbcobjpalette.data(), 0x40);
	file.varint(&gbcbgpaletteindex);
	file.varint(&gbcobjpaletteindex);
	file.bool32(&doublespeed);
	file.varint(&currentrombank);
	file.varint(&currentrambank);
	file.varint(&wisdomrombank);
	file.varint(&mbc6rombanka);
	file.varint(&mbc6rombankb);
	file.varint(&mbc6rambanka);
	file.varint(&mbc6rambankb);
	file.varint(&higherrombankbits);
	file.bool32(&rommode);
	file.bool32(&ramenabled);
	file.bool32(&externalrampres);
	file.bool32(&mbc6flashenable);
	file.bool32(&mbc6flashwriteenable);
    }

    bool MMU::loadbackup(vector<uint8_t> data)
    {
	bool success = true;	

	if (batteryenabled)
	{
	    if (data.empty())
	    {
		success = true;
	    }
	    else
	    {
		if ((gbmbc != MBCType::None) && (gbmbc != MBCType::MBC7))
		{
		    int rambanksize = (mbcramsize << 10);
		    rambanks = vector<uint8_t>(data.begin(), (data.begin() + rambanksize));
		    if (isrtcpres)
		    {
			loadbackuprtc(data, rambanksize);
		    }
		}
		else if (gbmbc == MBCType::MBC7)
		{
		    rambanks = vector<uint8_t>(data.begin(), (data.begin() + 256));
		}
	    }
	}

	return success;
    }

    vector<uint8_t> MMU::savebackup()
    {
	vector<uint8_t> save;
	if (batteryenabled)
	{
	    if ((gbmbc != MBCType::None) && (gbmbc != MBCType::MBC7))
	    {
		int rambanksize = (mbcramsize << 10);
		cout << "RAM size: " << dec << (int)(rambanksize >> 10) << " KB" << endl;
		save = vector<uint8_t>(rambanks.begin(), (rambanks.begin() + rambanksize));

		if (isrtcpres)
		{
		    savebackuprtc(save);
		}
	    }
	    else if (gbmbc == MBCType::MBC7)
	    {
		save = vector<uint8_t>(rambanks.begin(), (rambanks.begin() + 256));
	    }
	}

	return save;
    }

    uint8_t MMU::readDirectly(uint16_t addr)
    {
	if (addr < 0x8000)
	{
	    uint8_t temp = 0;

	    bool isromread = false;

	    if (biosload == true)
	    {
		if ((biossize == 0x900) && (addr > 0x100) && (addr < 0x200))
		{
		    isromread = true;
		}
		else if (addr == 0x100)
		{
		    exitbios();
		    isromread = true;
		}
		else if (addr < biossize)
		{
		    isromread = false;
		}
		else
		{
		    isromread = true;
		}
	    }
	    else
	    {
		isromread = true;
	    }

	    if (isromread == true)
	    {
		switch (gbmbc)
		{
		    case MBCType::None: temp = rom[addr]; break;
		    case MBCType::MBC1: temp = mbc1read(addr); break;
		    case MBCType::MBC2: temp = mbc2read(addr); break;
		    case MBCType::MBC3: temp = mbc3read(addr); break;
		    case MBCType::MBC5: temp = mbc5read(addr); break;
		    case MBCType::MBC6: temp = mbc6read(addr); break;
		    case MBCType::MBC7: temp = mbc7read(addr); break;
		    case MBCType::Camera: temp = gbcameraread(addr); break;
		    case MBCType::WisdomTree: temp = wisdomtreeread(addr); break;
		}
	    }
	    else
	    {
		return bios[addr];
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
		case MBCType::MBC6: temp = mbc6read(addr); break;
		case MBCType::MBC7: temp = mbc7read(addr); break;
		case MBCType::Camera: temp = gbcameraread(addr); break;
		case MBCType::WisdomTree: temp = wisdomtreeread(addr); break;
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

    uint8_t MMU::readByte(uint16_t addr)
    {
    	int temp = 0;
    
	if (addr < 0x4000)
	{
	    temp = readDirectly(addr); // Pocket Music and Disney's Aladdin (GBC ver.) rely on this in order to perform OAM DMA transfers properly
	}
	else if (addr < 0x8000)
	{
	    if (dmabusblock != Bus::External)
	    {
		temp = readDirectly(addr);
	    }
	    else
	    {
		temp = oamtransferbyte;
	    }
	}
	else if (addr < 0xA000)
	{
	    if (dmabusblock != Bus::Vram)
	    {
		temp = readDirectly(addr);
	    }
	    else
	    {
		temp = oamtransferbyte;
	    }
	}
	else if (addr < 0xFE00)
	{
	    if (dmabusblock != Bus::External)
	    {
		temp = readDirectly(addr);
	    }
	}
	else if (addr < 0xFEA0)
	{
	    if (dmabusblock == Bus::None)
	    {
		temp = readDirectly(addr);
	    }
	    else
	    {
		temp = 0xFF;
	    }
	}
	else if (addr < 0xFF00)
	{
	    temp = 0x00;
	}
	else
	{
	    temp = readDirectly(addr);
	}
	
	return temp;
    }

    void MMU::writeDirectly(uint16_t addr, uint8_t value)
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
		case MBCType::MBC6: mbc6write(addr, value); break;
		case MBCType::MBC7: mbc7write(addr, value); break;
		case MBCType::Camera: gbcamerawrite(addr, value); break;
		case MBCType::WisdomTree: wisdomtreewrite(addr, value); break;
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
		case MBCType::MBC6: mbc6write(addr, value); break;
		case MBCType::MBC7: mbc7write(addr, value); break;
		case MBCType::Camera: gbcamerawrite(addr, value); break;
		case MBCType::WisdomTree: wisdomtreewrite(addr, value); break;
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

    void MMU::writeByte(uint16_t addr, uint8_t value)
    {
	if (addr < 0x8000)
	{
	    if (dmabusblock != Bus::External)
	    {
		writeDirectly(addr, value);
	    }
	}
	else if (addr < 0xA000)
	{
	    if (dmabusblock != Bus::Vram)
	    {
		writeDirectly(addr, value);
	    }
	}
	else if (addr < 0xFE00)
	{
	    if (dmabusblock != Bus::External)
	    {
		writeDirectly(addr, value);
	    }
	}
	else if (addr < 0xFEA0)
	{
	    if (dmabusblock == Bus::None)
	    {
		writeDirectly(addr, value);
	    }
	}
	else if (addr < 0xFF00)
	{
	    return;
	}
	else
	{
	    writeDirectly(addr, value);
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
	
	if (addr >= 0xFF10 && (addr <= 0xFF26))
	{
	    temp = memoryreadhandlers.at((addr - 0xFF00))(addr);
	}
	else if ((addr >= 0xFF30) && (addr < 0xFF40))
	{
	    temp = memoryreadhandlers.at((addr - 0xFF00))(addr);
	}
	else if ((addr >= 0xFF40) && (addr < 0xFF46))
	{
	    temp = memoryreadhandlers.at((addr - 0xFF00))(addr);
	}
	else if ((addr >= 0xFF47) && (addr <= 0xFF4B))
	{
	    temp = memoryreadhandlers.at((addr - 0xFF00))(addr);
	}
	else if (addr == 0xFF56)
	{
	    temp = memoryreadhandlers.at((addr - 0xFF00))(addr);
	}
	else
	{
	    switch ((addr & 0xFF))
	    {
	    	case 0x00: temp = memoryreadhandlers.at((addr - 0xFF00))(addr); break;
	    	case 0x01: temp = memoryreadhandlers.at((addr - 0xFF00))(addr); break;
	    	case 0x02: temp = memoryreadhandlers.at((addr - 0xFF00))(addr); break;
	    	case 0x04: temp = memoryreadhandlers.at((addr - 0xFF00))(addr); break;
	    	case 0x05: temp = memoryreadhandlers.at((addr - 0xFF00))(addr); break;
	    	case 0x06: temp = memoryreadhandlers.at((addr - 0xFF00))(addr); break;
	    	case 0x07: temp = memoryreadhandlers.at((addr - 0xFF00))(addr); break;
	    	case 0x0F: temp = (interruptflags | 0xE0); break;
	    	case 0x46: temp = oamdmastart; break;
	    	case 0x4D: temp = (key1 | ((isgbcmode()) ? 0x7E : 0xFF)); break;
	    	case 0x4F:
		{
		    if (isgbcconsole())
		    {
			temp = ((isgbcmode()) ? (vrambank | 0xFE) : 0xFE);
		    }
		    else
		    {
			temp = 0xFF;
		    }
		}
		break;
	    	case 0x55: temp = ((!ishdmaactive) | 0x7F); break;
	    	case 0x68: temp = (isgbcconsole()) ? gbcbgpaletteindex : 0xFF; break;
	    	case 0x69: temp = (isgbcconsole()) ? gbcbgpalette[gbcbgpaletteindex] : 0xFF; break;
	    	case 0x6A: temp = (isgbcconsole()) ? gbcobjpaletteindex : 0xFF; break;
	    	case 0x6B: temp = (isgbcconsole()) ? gbcobjpalette[gbcobjpaletteindex] : 0xFF; break;
	    	case 0x70: temp = (wrambank | 0xF8); break;
	    	default: temp = 0xFF; break;
	    }
	}
	
	return temp;
    }

    void MMU::writeIO(uint16_t addr, uint8_t value)
    {
	if (addr >= 0xFF10 && (addr <= 0xFF26))
	{
	    memorywritehandlers.at((addr - 0xFF00))(addr, value);
	}
	else if (addr >= 0xFF30 && (addr < 0xFF40))
	{
	    memorywritehandlers.at((addr - 0xFF00))(addr, value);
	}
	else if (addr >= 0xFF40 && (addr < 0xFF46))
	{
	    memorywritehandlers.at((addr - 0xFF00))(addr, value);
	}
	else if (addr >= 0xFF47 && (addr <= 0xFF4B))
	{
	    memorywritehandlers.at((addr - 0xFF00))(addr, value);
	}
	else if (addr == 0xFF56)
	{
	    memorywritehandlers.at((addr - 0xFF00))(addr, value);
	}
	else
	{
	    switch ((addr & 0xFF))
	    {
		case 0x00: memorywritehandlers.at((addr - 0xFF00))(addr, value); break;
		case 0x01: memorywritehandlers.at((addr - 0xFF00))(addr, value); break;
		case 0x02: memorywritehandlers.at((addr - 0xFF00))(addr, value); break;
		case 0x04: memorywritehandlers.at((addr - 0xFF00))(addr, value); break;
		case 0x05: memorywritehandlers.at((addr - 0xFF00))(addr, value); break;
	 	case 0x06: memorywritehandlers.at((addr - 0xFF00))(addr, value); break;
		case 0x07: memorywritehandlers.at((addr - 0xFF00))(addr, value); break;
		case 0x0F: writeif(value); break;
		case 0x46: writedma(value); break;
		case 0x4D: 
		{
		    key1 = value;
		}
		break;
		case 0x4F: 
		{
		    vrambank = (isgbcmode()) ? BitGetVal(value, 0) : 0;
		}
		break;
		case 0x51:
		{
		    hdmasource = ((value << 8) | (hdmasource & 0xFF));
	 	}
		break;
		case 0x52:
		{
		    hdmasource = ((hdmasource & 0xFF00) | (value & 0xF0));
		}
		break;
		case 0x53:
		{
		    hdmadest = ((((value & 0x1F) | 0x80) << 8) | (hdmadest & 0xFF));
		}
	 	break;
		case 0x54:
		{
		    hdmadest = ((hdmadest & 0xFF00) | (value & 0xF0));
		}
		break;
		case 0x55:
		{
		    if (hdmastate == DmaState::Inactive)
		    {
			initgbcdma(value);
		    }
		    else
		    {
		    	if (TestBit(value, 7))
		   	{
			    initgbcdma(value);
		    	}
		    	else
		   	{
			    ishdmaactive = false;
			    hdmabytestocopy = 0;
			    hdmabytes = 0;
			    hdmastate = DmaState::Inactive;
		        }
		    }
		}
		break;
		case 0x68:
		{
		    if (isdmgconsole())
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
		    gameboy = (agbmode) ? Console::AGB : Console::CGB;
		}
		else
		{
		    gameboy = Console::DMG;
		}
	    }
	    

	    if (isgbcconsole() && cgbflag && gbmode == Mode::Default)
	    {
		gbmode = Mode::CGB;
	    }
	    else
	    {
		gbmode = Mode::DMG;
	    }

	    // initvram();

	    cout << "Title: " << determinegametitle(cartmem) << endl;
	    determinembctype(cartmem);
	    numrombanks = getrombanks(cartmem);
	    numrambanks = getrambanks(cartmem);
	    isromonly(cartmem);
	    cout << "MBC type: " << mbctype << endl;
	    cout << "ROM size: " << romsize << endl;
	    cout << "RAM size: " << ramsize << endl;

	    if (gbmbc != MBCType::None && numrombanks != 0 && size != (numrombanks * 0x4000))
	    {
		cout << "MMU::Warning - Size of ROM does not match size in cartridge header." << endl;
	    }

	    if (gbmbc == MBCType::MBC7)
	    {
		rambanks.clear();
		rambanks.resize(0x200, 0);
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
		    gameboy = (agbmode) ? Console::AGB : Console::CGB;
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

	    if (gbmbc == MBCType::MBC7)
	    {
		rambanks.clear();
		rambanks.resize(256, 0);
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

    bool MMU::loadROM(vector<uint8_t> data)
    {
	if (!data.empty())
	{
	    cartmem = data;

	    bool cgbflag = ((cartmem[0x0143] == 0xC0) || (cartmem[0x0143] == 0x80));

	    if (gameboy == Console::Default)
	    {
		if (cgbflag)
		{
		    gameboy = (agbmode) ? Console::AGB : Console::CGB;
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
	    
	    if (mbcramsize != 0)
	    {
		rambanks.resize((mbcramsize << 10), 0);
	    }

	    if (gbmbc != MBCType::None && static_cast<int>(data.size()) != (numrombanks * 0x4000))
	    {
		cout << "MMU::Warning - Size of ROM does not match size in cartridge header." << endl;
	    }

	    if (gbmbc == MBCType::MBC7)
	    {
		rambanks.clear();
		rambanks.resize(256, 0);
	    }

	    rom = vector<uint8_t>(cartmem.begin(), (cartmem.begin() + 0x8000));
	    cout << "MMU::ROM succesfully loaded." << endl;
	    return true;
	}
	else
	{
	    cout << "MMU::Error - ROM could not be opened." << endl;
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

    bool MMU::loadBIOS(vector<uint8_t> data)
    {
	if (!data.empty())
	{
	    size_t size = data.size();

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
	    bios = data;

	    cout << "MMU::BIOS succesfully loaded." << endl;
	    return true;
	}
	else
	{
	    cout << "MMU::Error - BIOS could not be opened." << endl;
	    return false;
	}
    }
};
