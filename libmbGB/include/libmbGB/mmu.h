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
#include <cstring>
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
    using screenfunc = function<void()>;
    using apulengthfunc = function<bool()>;
    using rumblefunc = function<void(bool)>;
    using sensorfunc = function<void(uint16_t &, uint16_t &)>;
	
	using memoryreadfunc = function<uint8_t(uint16_t)>;
	using memorywritefunc = function<void(uint16_t, uint8_t)>;

    class LIBMBGB_API MMU
    {
	public:
	    MMU();
	    ~MMU();

	    bool agbmode = false;
	    Console gameboy = Console::Default;
	    Mode gbmode = Mode::Default;
	    MBCType gbmbc;

	    void init();
	    void initnot();
	    void shutdown();
		
		array<memoryreadfunc, 0x80> memoryreadhandlers;
		array<memorywritefunc, 0x80> memorywritehandlers;
		
		void addmemoryreadhandler(uint16_t addr, memoryreadfunc cb)
		{
			memoryreadhandlers.at((addr - 0xFF00)) = cb;
		}
		
		void addmemorywritehandler(uint16_t addr, memorywritefunc cb)
		{
			memorywritehandlers.at((addr - 0xFF00)) = cb;
		}

	    inline void initio()
	    {
		if (isdmgmode())
		{
		    if (isdmgconsole())
		    {
			divider = 0xABCC;
		    }
		    else
		    {
			divider = 0x267C;
		    }
		}
		else
		{
		    divider = 0x1EA0;
		}

		interruptenabled = 0x00;
		interruptflags = 0xE0;
	    	tima = 0x00;
	    	tma = 0x00;
	        tac = 0xF8;
		vrambank = 0;
		wrambank = 1;
		writeByte(0xFF4D, 0x7E);
		hdmasource = 0xFFFF;
		hdmadest = 0xFFFF;
		hdmalength = 0xFF;
		doublespeed = false;
	    }

	    inline void resetio()
	    {
	        sb = 0x00;
	    	sc = 0x00;
	    	divider = 0x0000;
	    	tima = 0x00;
	    	tma = 0x00;
	        tac = 0x00;
	        interruptflags = 0x00;
	        dma = 0x00;
	        key1 = 0x00;
	        interruptenabled = 0x00;
	    	dmaactive = false;
	    }

	    uint8_t readempty(uint16_t addr)
	    {
		return 0xFF;
	    }

	    void writeempty(uint16_t addr, uint8_t val)
	    {
		return;
	    }

	    bool loadmmu(int offs, string filename);
	    bool savemmu(string filename);

	    bool loadbackup(string filename);
	    bool savebackup(string filename);

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
	    bool isrumblepres = false;
	    bool isrtcpres = false;
	    bool biosload = false;
	    uint8_t currentrombank = 1;
	    uint8_t currentrambank = 0;
	    int higherrombankbits = 0;
	    bool ramenabled = false;
	    bool batteryenabled = false;
	    bool rtcenabled = false;
	    bool ramsecenabled = false;
	    bool rommode = false;
	    int numrombanks;
	    int numrambanks;
	    int mbcramsize;
	    int biossize = 0;
	    bool ismanual = false;
	    bool hybrid = false;
	    bool doublespeed = false;

	    bool rtclatch1 = true;
	    bool rtclatch2 = true;

	    uint8_t readmbc7ram(uint16_t addr);
	    void writembc7ram(uint16_t addr, uint8_t val);
	    void writembc7eeprom(uint8_t val);

	    bool mbc7chipsel = false;
	    bool mbc7chipclk = false;
	    int mbc7chipcmd = 0;
	    int mbc7chipaddr = 0;
	    uint16_t mbc7chipbuf = 0;
	    int mbc7chipsize = 0;
	    int mbc7intstate = 0;
	    int mbc7intvalue = 0;
	    bool mbc7idle = true;
	    bool mbc7wenable = false;

	    bool mbc7sensorlatch = false;
	    uint16_t mbc7sensorx = 0x8000;
	    uint16_t mbc7sensory = 0x8000;

	    sensorfunc setsensor;

	    rumblefunc setrumble;

	    inline void setsensorcallback(sensorfunc cb)
	    {
		setsensor = cb;
	    }

	    inline void setrumblecallback(rumblefunc cb)
	    {
		setrumble = cb;
	    }

	    bool dump = false;

	    int vrambank = 0;
	    int wrambank = 1;

	    int gbcbgpaletteindex = 0;
	    vector<uint8_t> gbcbgpalette;
	    int gbcobjpaletteindex = 0;
	    vector<uint8_t> gbcobjpalette;
	    bool gbcbgpalinc = false;
	    bool gbcobjpalinc = false;

	    bool hdmaactive = false;
	    uint16_t hdmadest = 0;
	    uint16_t hdmasource = 0;
	    uint16_t hdmalength = 0;

	    inline void hdmatransfer()
	    {
		for (int i = 0; i < 0x10; i++)
		{
		    writeByte(hdmadest, readByte(hdmasource));

		    hdmadest += 1;
		    hdmasource += 1;
		}
	    }
	

	    bool isdmgconsole()
	    {
		return (gameboy == Console::DMG);
	    }

	    bool isdmgmode()
	    {
		return (gbmode == Mode::DMG);
	    }

	    bool isgbcmode()
	    {
		return (gbmode == Mode::CGB);
	    }

	    bool isgbcconsole()
	    {
		return (gameboy == Console::CGB || gameboy == Console::AGB);
	    }

	    bool isgbcdmgmode()
	    {
		return (isgbcconsole() && isdmgmode());
	    }

	    bool isgbcgame()
	    {
		return (isgbcconsole() && isgbcmode());
	    }

	    bool ishybridconsole()
	    {
		return hybrid;
	    }

	    bool isdoublespeed()
	    {
		return doublespeed;
	    }

	    inline void determinembctype(vector<uint8_t>& rom)
	    {
		switch (rom[0x0147])
		{
		    case 0x00: gbmbc = MBCType::None; externalrampres = false; mbctype = "ROM ONLY"; batteryenabled = false; break;
		    case 0x01: gbmbc = MBCType::MBC1; externalrampres = false; mbctype = "MBC1"; batteryenabled = false; break;
		    case 0x02: gbmbc = MBCType::MBC1; externalrampres = true; mbctype = "MBC1 + RAM"; batteryenabled = false; break;
		    case 0x03: gbmbc = MBCType::MBC1; externalrampres = true; mbctype = "MBC1 + RAM + BATTERY"; batteryenabled = true; break;
		    case 0x05: gbmbc = MBCType::MBC2; externalrampres = false; mbctype = "MBC2"; batteryenabled = false; break;
		    case 0x06: gbmbc = MBCType::MBC2; externalrampres = false; mbctype = "MBC2 + BATTERY"; batteryenabled = true; break;
		    case 0x08: gbmbc = MBCType::None; externalrampres = true; mbctype = "ROM + RAM"; batteryenabled = false; break;
		    case 0x09: gbmbc = MBCType::None; externalrampres = true; mbctype = "ROM + RAM + BATTERY"; batteryenabled = true; break;
		    case 0x0F: gbmbc = MBCType::MBC3; externalrampres = false; mbctype = "MBC3 + TIMER + BATTERY"; break;
		    case 0x10: gbmbc = MBCType::MBC3; externalrampres = true; mbctype = "MBC3 + TIMER + RAM + BATTERY"; batteryenabled = true; isrtcpres = true; break;
		    case 0x11: gbmbc = MBCType::MBC3; externalrampres = false; mbctype = "MBC3"; batteryenabled = false; break;
		    case 0x12: gbmbc = MBCType::MBC3; externalrampres = true; mbctype = "MBC3 + RAM"; batteryenabled = false; break;
		    case 0x13: gbmbc = MBCType::MBC3; externalrampres = true; mbctype = "MBC3 + RAM + BATTERY"; batteryenabled = true; break;
		    case 0x19: gbmbc = MBCType::MBC5; externalrampres = false; mbctype = "MBC5"; batteryenabled = false; break;
		    case 0x1A: gbmbc = MBCType::MBC5; externalrampres = true; mbctype = "MBC5 + RAM"; batteryenabled = false; break;
		    case 0x1B: gbmbc = MBCType::MBC5; externalrampres = true; mbctype = "MBC5 + RAM + BATTERY"; batteryenabled = true; break;
		    case 0x1E: gbmbc = MBCType::MBC5; externalrampres = true; isrumblepres = true; mbctype = "MBC5 + RUMBLE + RAM + BATTERY"; batteryenabled = true; break;
		    case 0x22: gbmbc = MBCType::MBC7; externalrampres = true; isrumblepres = true; mbctype = "MBC7 + SENSOR + RUMBLE + RAM + BATTERY"; batteryenabled = true; break;
		    default: cout << "MMU::Error - Unrecognized MBC type of " << hex << (int)(rom[0x0147]) << endl; exit(1); break;
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
		    case 8: banks = 512; romsize = "8 MB"; break;
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
		    case 1: banks = 1; mbcramsize = 2; ramsize = "2 KB"; break;
		    case 2: banks = 1; mbcramsize = 8; ramsize = "8 KB"; break;
		    case 3: banks = 4; mbcramsize = 32; ramsize = "32 KB"; break;
		    case 4: banks = 16; mbcramsize = 128; ramsize = "128 KB"; break;
		    case 5: banks = 8; mbcramsize = 64; ramsize = "64 KB"; break;
	    	    default: cout << "MMU::Error - Unrecognized RAM quantity given in cartridge" << endl; exit(1); break;
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
	    bool loadROM(const char *filename, const uint8_t* buffer, int size);

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
	    uint8_t mbc5read(uint16_t addr);
	    void mbc5write(uint16_t addr, uint8_t value);
	    uint8_t mbc7read(uint16_t addr);
	    void mbc7write(uint16_t addr, uint8_t value);

	    uint8_t readIO(uint16_t addr);
	    void writeIO(uint16_t addr, uint8_t value);

	    uint8_t dmastart = 0;

	    enum DmaState : int
	    {
		Inactive = -1,
		Starting = 0,
		Active = 1,
		Paused = 2
	    };

	    enum Bus : int
	    {
		None = -1,
		External = 0,
		Vram = 1
	    };

	    DmaState oamdmastate = DmaState::Inactive;
	    Bus dmabusblock = Bus::None;

	    uint16_t oamtransferaddr;
	    uint8_t oamtransferbyte;
	    uint8_t oamdmastart;
	    int bytesread = 160;
	    int dmadelay = 0;

	    poweronfunc poweron;
	    joypadfunc updatep1;
	    statirqfunc statirq;
	    screenfunc screen;
	    apulengthfunc apulength;

	    inline void writedma(uint8_t value)
	    {
		oamdmastart = value;
		oamdmastate = DmaState::Starting;
	    }

	    inline void updateoamdma()
	    {
		if (oamdmastate == DmaState::Starting)
		{
		    if (bytesread != 0)
		    {
			oamtransferaddr = (oamdmastart << 8);
			bytesread = 0;
		    }
		    else
		    {
			oamtransferbyte = dmacopy(oamtransferaddr);
			++bytesread;

			oamdmastate = DmaState::Active;

			if ((oamtransferaddr >= 0x8000) && (oamtransferaddr < 0xA000))
			{
			    dmabusblock = Bus::Vram;
			}
			else
			{
			    dmabusblock = Bus::External;
			}
		    }
		}
		else if (oamdmastate == DmaState::Active)
		{
		    oam[(bytesread - 1)] = oamtransferbyte;

		    if (bytesread == 160)
		    {
			oamdmastate = DmaState::Inactive;
			dmabusblock = Bus::None;
			return;
		    }

		    oamtransferbyte = dmacopy(oamtransferaddr + bytesread);
		    ++bytesread;
		}
	    }

	    uint8_t readDirectly(uint16_t addr);
	    void writeDirectly(uint16_t addr, uint8_t value);

	    inline uint8_t dmacopy(uint16_t addr)
	    {
		if (addr < 0xF000)
		{
		    return readDirectly(addr);
		}
		else
		{
		    return readDirectly(addr - 0x2000);
		}
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

	    void setscreencallback(screenfunc cb)
	    {
		screen = cb;
	    }

	    void setapucallbacks(apulengthfunc cb)
	    {
		apulength = cb;
	    }

	    inline void exitbios()
	    {
		biosload = false;
		cout << "MMU::Exiting BIOS..." << endl;
		screen();
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

	    bool ifwrittenthiscycle = false;
	

     	    bool statinterruptsignal = false;
	    bool previnterruptsignal = false;
	    bool isdivinterrupt = true;

	    uint8_t sb = 0x00;
	    uint8_t sc = 0x00;
	    uint16_t divider = 0x0000;
	    uint8_t tima = 0x00;
	    uint8_t tma = 0x00;
	    uint8_t tac = 0x00;
	    uint8_t interruptflags = 0xE1;
	    uint8_t dma = 0x00;
	    uint8_t key1 = 0x00;
	    uint8_t interruptenabled = 0x00;
	    bool dmaactive = false;
	    uint8_t lylastcycle = 0xFF;

    };
};

#endif // LIBMBGB_MMU
