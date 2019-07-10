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

#ifndef LIBMBGB_MMU
#define LIBMBGB_MMU

#include <cstdint>
#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <functional>
#include <array>
#include "enums.h"
#include "libmbgb_api.h"
using namespace std;

namespace gb
{
    inline bool TestBit(uint32_t reg, int bit)
    {
	return (reg & (1 << bit)) ? true : false;
    }

    inline uint8_t BitSet(uint32_t reg, int bit)
    {
	return (reg | (1 << bit));
    }

    inline uint8_t BitReset(uint32_t reg, int bit)
    {
	return (reg & ~(1 << bit));
    }

    inline int BitGetVal(uint32_t reg, int bit)
    {
	return (reg & (1 << bit)) ? 1 : 0;
    }

    using poweronfunc = function<void(bool)>;
    using joypadfunc = function<void()>;
    using statirqfunc = function<bool()>;

    class LIBMBGB_API MMU
    {
	public:
	    MMU();
	    ~MMU();

	    Console gameboy = Console::Default;
	    Mode gbmode;
	    MBCType gbmbc;

	    void init();
	    void shutdown();

	    vector<uint8_t> rom;
	    vector<uint8_t> vram;
	    vector<uint8_t> sram;
	    vector<uint8_t> wram;
	    vector<uint8_t> oam;
	    vector<uint8_t> hram;
	    vector<uint8_t> cartmem;
	    vector<uint8_t> rambanks;
	    vector<uint8_t> bios;
	    array<uint8_t, 4> specialrombanks = {0x00, 0x20, 0x40, 0x60};

	    bool externalrampres = false;
	    bool biosload = false;
	    uint8_t currentrombank = 1;
	    uint8_t currentrambank = 0;
	    int higherrombankbits = 0;
	    bool ramenabled = false;
	    bool rommode = false;
	    int numrombanks;
	    int numrambanks;
	    int mbcramsize;

	    bool isdmgconsole()
	    {
		return (gameboy == Console::DMG);
	    }

	    bool isdmgmode()
	    {
		return (gbmode == Mode::DMG);
	    }

	    bool isgbcconsole()
	    {
		return (gameboy == Console::CGB);
	    }

	    inline void determinembctype(vector<uint8_t>& rom)
	    {
		switch (rom[0x0147])
		{
		    case 0: gbmbc = MBCType::None; externalrampres = false; mbctype = "ROM ONLY"; break;
		    case 1: gbmbc = MBCType::MBC1; externalrampres = false; mbctype = "MBC1"; break;
		    case 2: gbmbc = MBCType::MBC1; externalrampres = true; mbctype = "MBC1 + RAM"; break;
		    case 3: gbmbc = MBCType::MBC1; externalrampres = true; mbctype = "MBC1 + RAM + BATTERY"; break;
		    case 5: gbmbc = MBCType::MBC2; externalrampres = false; mbctype = "MBC2"; break;
		    case 6: gbmbc = MBCType::MBC2; externalrampres = false; mbctype = "MBC2 + BATTERY"; break;
		    case 8: gbmbc = MBCType::None; externalrampres = true; mbctype = "ROM + RAM"; break;
		    case 9: gbmbc = MBCType::None; externalrampres = true; mbctype = "ROM + RAM + BATTERY"; break;
		    case 15: gbmbc = MBCType::MBC3; externalrampres = false; mbctype = "MBC3 + TIMER + BATTERY"; break;
		    case 16: gbmbc = MBCType::MBC3; externalrampres = true; mbctype = "MBC3 + TIMER + RAM + BATTERY"; break;
		    case 17: gbmbc = MBCType::MBC3; externalrampres = false; mbctype = "MBC3"; break;
		    case 18: gbmbc = MBCType::MBC3; externalrampres = true; mbctype = "MBC3 + RAM"; break;
		    case 19: gbmbc = MBCType::MBC3; externalrampres = true; mbctype = "MBC3 + RAM + BATTERY"; break;
		    default: cout << "MMU::Error - Unrecognized MBC type" << endl; exit(1); break;
		}
	    }

	    inline int getrombanks(vector<uint8_t>& rom)
	    {
		bool ismbc1 = ((rom[0x0147] >= 1) && (rom[0x0147] <= 3));

		int banks = 0;

		switch (rom[0x0148])
		{
		    case 0: banks = 0; romsize = "32 KB"; break;
		    case 1: banks = 4; romsize = "64 KB"; break;
		    case 2: banks = 8; romsize = "128 KB"; break;
		    case 3: banks = 16; romsize = "256 KB"; break;
		    case 4: banks = 32; romsize = "512 KB"; break;
		    case 5: banks = (ismbc1 ? 63 : 64); romsize = "1 MB"; break;
		    case 6: banks = (ismbc1 ? 125 : 128); romsize = "2 MB"; break;
		    case 7: banks = 256; romsize = "4 MB"; break;
		    case 82: banks = 72; romsize = "1.1 MB"; break;
		    case 83: banks = 80; romsize = "1.2 MB"; break;
		    case 84: banks = 96; romsize = "1.5 MB"; break;
		    default: cout << "MMU::Error - Unrecognzied ROM quantity given in cartridge" << endl; exit(1); break;
		}

		return banks;
	    }

	    inline int getrambanks(vector<uint8_t>& rom)
	    {
		int banks = 0;

		switch (rom[0x0149])
		{
		    case 0: banks = 0; mbcramsize = 0; ramsize = "None"; break;
		    case 1: banks = 0; mbcramsize = 2; ramsize = "2 KB"; break;
		    case 2: banks = 0; mbcramsize = 8; ramsize = "8 KB"; break;
		    case 3: banks = 4; mbcramsize = 32; ramsize = "32 KB"; break; 
	    	    default: cout << "MMU::Error - Unrecognzied RAM quantity given in cartridge" << endl; exit(1); break;
		}

		return banks;
	    }

	    inline string determinegametitle(vector<uint8_t>& rom)
	    {
		stringstream temp;
	
		for (int i = 0x134; i < 0x0143; i++)
		{
		    temp << ((char)(int)(rom[i]));
		}

		return temp.str();
    	    }

	    string mbctype;
	    string romsize;
	    string ramsize;

	    bool loadBIOS(string filename);
	    bool loadROM(string filename);

	    uint8_t readByte(uint16_t addr);
	    void writeByte(uint16_t addr, uint8_t value);
	    uint16_t readWord(uint16_t addr);
	    void writeWord(uint16_t addr, uint16_t value);

	    uint8_t mbc1read(uint16_t addr);
	    void mbc1write(uint16_t addr, uint8_t value);
	    uint8_t mbc2read(uint16_t addr);
	    void mbc2write(uint16_t addr, uint8_t value);
	    uint8_t mbc3read(uint16_t addr);
	    void mbc3write(uint16_t addr, uint8_t value);

	    uint8_t readIO(uint16_t addr);
	    void writeIO(uint16_t addr, uint8_t value);

	    poweronfunc poweron;
	    joypadfunc updatep1;
	    statirqfunc statirq;

	    inline void lcdchecklyc()
	    {
		if (!TestBit(lcdc, 7))
		{
		    return;
		}

		if ((lyc && ly == lyc) || (!lyc && ly == 153))
		{
		    if (!TestBit(stat, 2))
		    {
			stat = BitSet(stat, 2);

			if (TestBit(stat, 6))
			{
			    checkstatinterrupt();
			}
		    }
		}
		else
		{
		    stat = BitReset(stat, 2);
		}
    	    }

	    inline void dodmatransfer(uint8_t value)
	    {		
		uint16_t addr = (value << 8);
		for (int i = 0; i < 0xA0; i++)
		{
		    writeByte((0xFE00 + i), readByte(addr + i));
		}
	    }

	    inline bool islcdenabled()
	    {
		return TestBit(lcdc, 7);
	    }

	    inline bool iswinenabled()
	    {
		return TestBit(lcdc, 5);
	    }

	    inline bool isobjenabled()
	    {
		return TestBit(lcdc, 1);
	    }

	    inline bool isbgenabled()
	    {
		return TestBit(lcdc, 0);
	    }

	    inline void setstatmode(int mode)
	    {
		stat = ((stat & 0xFC) | mode);
	    }

	    inline void writejoypad(uint8_t value)
	    {
		joypad = ((joypad & 0x0F) | (value & 0x30));
		updatep1();
	    }

	    inline void writelcdc(uint8_t value)
	    {
		lcdc = value;
	    }

	    inline void writediv()
	    {
		divider = 0x0000;
		isdivinterrupt = false;
	    }

	    void setpoweroncallback(poweronfunc cb)
	    {
		poweron = cb;
	    }

	    void setjoypadcallback(joypadfunc cb)
	    {
		updatep1 = cb;
	    }

	    void setstatirqcallback(statirqfunc cb)
	    {
		statirq = cb;
	    }

	    inline void exitbios()
	    {
		biosload = false;
		cout << "MMU::Exiting BIOS..." << endl;
	    }

	    inline void requestinterrupt(int id)
	    {
		if (!ifwrittenthiscycle)
		{
		    interruptflags = BitSet(interruptflags, id);
		}
	    }

	    inline void clearinterrupt(int id)
	    {
		if (!ifwrittenthiscycle)
		{
		    interruptflags = BitReset(interruptflags, id);
		}
	    }

	    bool ispending(int id)
	    {
		uint8_t temp = (interruptflags & interruptenabled);
		return TestBit(temp, id);
	    }

	    bool requestedenabledinterrupts()
	    {
		return (interruptflags & interruptenabled);
	    }

	    inline uint8_t readsc()
	    {
		return (sc & 0x7E);
	    }

	    inline void writesc(uint8_t value)
	    {
		sc = (value & 0x81);
	    }

	    inline void writeif(uint8_t value)
	    {
		interruptflags = (value & 0x1F);
		ifwrittenthiscycle = true;
	    }

	    inline void writestat(uint8_t value)
	    {
		stat = ((value & 0x78) | (stat & 0x07));
		if (TestBit(lcdc, 7) && !TestBit(stat, 1))
		{
		    statinterruptsignal = true;
		    checkstatinterrupt();
		}
	    }

	    inline uint16_t readdiv()
	    {
		return divider;
	    }

	    inline void incdiv(int cycles)
	    {
		divider += cycles;
	    }

	    inline void setlycompare(bool cond)
	    {
		if (cond)
		{
		    stat = BitSet(stat, 2);
		}
		else
		{
		    stat = BitReset(stat, 2);
		}
	    }

	    inline int getstatmode()
	    {
		return (stat & 0x3);
	    }

	    inline void checkstatinterrupt()
	    {
		statinterruptsignal |= (TestBit(stat, 3) && getstatmode() == 0);
		statinterruptsignal |= (TestBit(stat, 4) && getstatmode() == 1);
		statinterruptsignal |= (TestBit(stat, 5) && getstatmode() == 2);
		statinterruptsignal |= (TestBit(stat, 6) && TestBit(stat, 2));

		if (statinterruptsignal && !previnterruptsignal)
		{
		    requestinterrupt(1);
		}

		previnterruptsignal = statinterruptsignal;
		statinterruptsignal = false;
	    }

	    bool ifwrittenthiscycle = false;
	

     	    bool statinterruptsignal = false;
	    bool previnterruptsignal = false;
	    bool isdivinterrupt = true;

	    uint8_t joypad = 0x00;
	    uint8_t sb = 0x00;
	    uint8_t sc = 0x00;
	    uint16_t divider = 0x0000;
	    uint8_t tima = 0x00;
	    uint8_t tma = 0x00;
	    uint8_t tac = 0x00;
	    uint8_t interruptflags = 0xE1;
	    uint8_t lcdc = 0x91;
	    uint8_t stat = 0x01;
	    uint8_t scrolly = 0x00;
	    uint8_t scrollx = 0x00;
	    uint8_t windowy = 0x00;
	    uint8_t windowx = 0x00;
	    uint8_t ly = 0x00;
	    uint8_t lyc = 0x00;
	    uint8_t bgpalette = 0xFC;
	    uint8_t objpalette0 = 0xFF;
	    uint8_t objpalette1 = 0xFF;
	    uint8_t lastbgp = 0x00;
	    uint8_t interruptenabled = 0x00;

	    uint8_t lylastcycle = 0xFF;
    };
};

#endif // LIBMBGB_MMU
