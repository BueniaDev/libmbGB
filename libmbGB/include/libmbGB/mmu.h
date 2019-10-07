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

	    inline void initio()
	    {
		if (isdmgmode())
		{
		    if (isdmgconsole())
		    {
			joypad = 0xCF;
			divider = 0xABCC;
		    }
		    else
		    {
			joypad = 0xFF;
			divider = 0x267C;
		    }
		}
		else
		{
		    joypad = 0xFF;
		    divider = 0x1EA0;
		}

		interruptenabled = 0x00;
		bgpalette = 0xFC;
		interruptflags = 0xE0;
		ly = 0x90;
		stat = 0x01;
	    	tima = 0x00;
	    	tma = 0x00;
	        tac = 0xF8;
		lcdc = 0x91;
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
	        joypad = 0x00;
	        sb = 0x00;
	    	sc = 0x00;
	    	divider = 0x0000;
	    	tima = 0x00;
	    	tma = 0x00;
	        tac = 0x00;
	        interruptflags = 0x00;
	        lcdc = 0x00;
	        stat = 0x00;
	        scrolly = 0x00;
	        scrollx = 0x00;
	        windowy = 0x00;
	        windowx = 0x00;
	        ly = 0x00;
	        lyc = 0x00;
	        dma = 0x00;
	        bgpalette = 0x00;
	        objpalette0 = 0x00;
	        objpalette1 = 0x00;
	        key1 = 0x00;
	        interruptenabled = 0x00;
	    	dmaactive = false;
	    }

	    inline void readio(ifstream& file)
	    {
	        file.read((char*)&joypad, sizeof(joypad));
	        file.read((char*)&sb, sizeof(sb));
	    	file.read((char*)&sc, sizeof(sc));
	    	file.read((char*)&divider, sizeof(divider));
	    	file.read((char*)&tima, sizeof(tima));
	    	file.read((char*)&tma, sizeof(tma));
	        file.read((char*)&tac, sizeof(tac));
	        file.read((char*)&interruptflags, sizeof(interruptflags));
	        file.read((char*)&lcdc, sizeof(lcdc));
	        file.read((char*)&stat, sizeof(stat));
	        file.read((char*)&scrolly, sizeof(scrolly));
	        file.read((char*)&scrollx, sizeof(scrollx));
	        file.read((char*)&windowy, sizeof(windowy));
	        file.read((char*)&windowx, sizeof(windowx));
	        file.read((char*)&ly, sizeof(ly));
	        file.read((char*)&lyc, sizeof(lyc));
	        file.read((char*)&dma, sizeof(dma));
	        file.read((char*)&bgpalette, sizeof(bgpalette));
	        file.read((char*)&objpalette0, sizeof(objpalette0));
	        file.read((char*)&objpalette1, sizeof(objpalette1));
	        file.read((char*)&key1, sizeof(key1));
	        file.read((char*)&interruptenabled, sizeof(interruptenabled));
	    	file.read((char*)&dmaactive, sizeof(dmaactive));
	    }

	    inline void writeio(ofstream& file)
	    {
		file.write((char*)&joypad, sizeof(joypad));
	        file.write((char*)&sb, sizeof(sb));
	    	file.write((char*)&sc, sizeof(sc));
	    	file.write((char*)&divider, sizeof(divider));
	    	file.write((char*)&tima, sizeof(tima));
	    	file.write((char*)&tma, sizeof(tma));
	        file.write((char*)&tac, sizeof(tac));
	        file.write((char*)&interruptflags, sizeof(interruptflags));
	        file.write((char*)&lcdc, sizeof(lcdc));
	        file.write((char*)&stat, sizeof(stat));
	        file.write((char*)&scrolly, sizeof(scrolly));
	        file.write((char*)&scrollx, sizeof(scrollx));
	        file.write((char*)&windowy, sizeof(windowy));
	        file.write((char*)&windowx, sizeof(windowx));
	        file.write((char*)&ly, sizeof(ly));
	        file.write((char*)&lyc, sizeof(lyc));
	        file.write((char*)&dma, sizeof(dma));
	        file.write((char*)&bgpalette, sizeof(bgpalette));
	        file.write((char*)&objpalette0, sizeof(objpalette0));
	        file.write((char*)&objpalette1, sizeof(objpalette1));
	        file.write((char*)&key1, sizeof(key1));
	        file.write((char*)&interruptenabled, sizeof(interruptenabled));
	    	file.write((char*)&dmaactive, sizeof(dmaactive));
	    }

	    bool loadmmu(int offset, string filename);
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
	    bool biosload = false;
	    uint8_t currentrombank = 1;
	    uint8_t currentrambank = 0;
	    int higherrombankbits = 0;
	    bool ramenabled = false;
	    bool batteryenabled = false;
	    bool rommode = false;
	    int numrombanks;
	    int numrambanks;
	    int mbcramsize;
	    int biossize = 0;
	    bool ismanual = false;
	    bool hybrid = false;
	    bool doublespeed = false;

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

	    inline void determinembctype(vector<uint8_t>& rom)
	    {
		switch (rom[0x0147])
		{
		    case 0: gbmbc = MBCType::None; externalrampres = false; mbctype = "ROM ONLY"; batteryenabled = false; break;
		    case 1: gbmbc = MBCType::MBC1; externalrampres = false; mbctype = "MBC1"; batteryenabled = false; break;
		    case 2: gbmbc = MBCType::MBC1; externalrampres = true; mbctype = "MBC1 + RAM"; batteryenabled = false; break;
		    case 3: gbmbc = MBCType::MBC1; externalrampres = true; mbctype = "MBC1 + RAM + BATTERY"; batteryenabled = true; break;
		    case 5: gbmbc = MBCType::MBC2; externalrampres = false; mbctype = "MBC2"; batteryenabled = false; break;
		    case 6: gbmbc = MBCType::MBC2; externalrampres = false; mbctype = "MBC2 + BATTERY"; batteryenabled = true; break;
		    case 8: gbmbc = MBCType::None; externalrampres = true; mbctype = "ROM + RAM"; batteryenabled = false; break;
		    case 9: gbmbc = MBCType::None; externalrampres = true; mbctype = "ROM + RAM + BATTERY"; batteryenabled = true; break;
		    case 15: gbmbc = MBCType::MBC3; externalrampres = false; mbctype = "MBC3 + TIMER + BATTERY"; break;
		    case 16: gbmbc = MBCType::MBC3; externalrampres = true; mbctype = "MBC3 + TIMER + RAM + BATTERY"; batteryenabled = true; break;
		    case 17: gbmbc = MBCType::MBC3; externalrampres = false; mbctype = "MBC3"; batteryenabled = false; break;
		    case 18: gbmbc = MBCType::MBC3; externalrampres = true; mbctype = "MBC3 + RAM"; batteryenabled = false; break;
		    case 19: gbmbc = MBCType::MBC3; externalrampres = true; mbctype = "MBC3 + RAM + BATTERY"; batteryenabled = true; break;
		    case 25: gbmbc = MBCType::MBC5; externalrampres = false; mbctype = "MBC5"; batteryenabled = false; break;
		    case 26: gbmbc = MBCType::MBC5; externalrampres = true; mbctype = "MBC5 + RAM"; batteryenabled = false; break;
		    case 27: gbmbc = MBCType::MBC5; externalrampres = true; mbctype = "MBC5 + RAM + BATTERY"; batteryenabled = true; break;
		    case 30: gbmbc = MBCType::MBC5; externalrampres = true; mbctype = "MBC5 + RUMBLE + RAM + BATTERY"; batteryenabled = true; break;
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
		bool wasenabled = TestBit(value, 7);
		lcdc = value;
		poweron(wasenabled);
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

	    inline void writestat(uint8_t value)
	    {
		stat = ((value & 0x78) | (stat & 0x07));
		if ((isdmgconsole() || ishybridconsole()) && TestBit(lcdc, 7) && !TestBit(stat, 1))
		{
		    statinterruptsignal = true;
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
	    uint8_t dma = 0x00;
	    uint8_t bgpalette = 0xFC;
	    uint8_t objpalette0 = 0xFF;
	    uint8_t objpalette1 = 0xFF;
	    uint8_t key1 = 0x00;
	    uint8_t interruptenabled = 0x00;
	    bool dmaactive = false;
	    uint8_t lylastcycle = 0xFF;

	    int s1sweep = 0;
	    bool s1negative = false;
	    bool s1sweepenabled = false;
	    uint16_t s1shadowfreq = 0;
	    int s1sweepcounter = 0;
	    int s1soundlength = 0;
	    int s1lengthcounter = 0;
	    int s1volumeenvelope = 0;
	    bool s1envelopeenabled = false;
	    int s1envelopecounter = 0;
	    int s1volume = 0;
	    uint8_t s1freqlo = 0;
	    uint8_t s1freqhi = 0;
	    int s1periodtimer = 0;
	    bool s1enabled = false;

	    int s2soundlength = 0;
	    int s2lengthcounter = 0;
	    int s2volumeenvelope = 0;
	    bool s2envelopeenabled = false;
	    int s2envelopecounter = 0;
	    int s2volume = 0;
	    uint8_t s2freqlo = 0;
	    uint8_t s2freqhi = 0;
	    int s2periodtimer = 0;
	    bool s2enabled = false;

	    int wavesweep = 0;
	    int wavesoundlength = 0;
	    int wavelengthcounter = 0;
	    int wavevolumeenvelope = 0;
	    int wavevolume = 0;
	    uint8_t wavefreqlo = 0;
	    uint8_t wavefreqhi = 0;
	    int waveperiodtimer = 0;
	    bool waveenabled = false;
	    int waveramlengthmask = 0;
	    int wavepos = 0;
	    bool wavechannelenabled = false;
	    uint8_t wavecurrentsample = 0;
	    uint8_t wavelastplayedsample = 0;
	    array<uint8_t, 0x10> waveram = {0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF};

	    int noisesoundlength = 0;
	    int noiselengthcounter = 0;
	    int noisevolumeenvelope = 0;
	    bool noiseenvelopeenabled = false;
	    int noiseenvelopecounter = 0;
	    int noiseperiodtimer = 0;
	    uint8_t noisefreqlo = 0;
	    uint8_t noisefreqhi = 0;
	    int noisevolume = 0;
	    uint16_t noiselfsr = 1;
	    bool noiseenabled = false;

	    int mastervolume = 0;
	    int soundselect = 0;
	    int soundon = 0;

	    array<int, 8> s1dutycycle;
	    array<int, 8> s2dutycycle;

	    inline bool s1enabledleft()
	    {
		return (s1enabled && (TestBit(soundselect, 4)));
	    }

	    inline bool s1enabledright()
	    {
		return (s1enabled && (TestBit(soundselect, 0)));
	    }

	    inline void writes1sweep(uint8_t value)
	    {
		s1sweep = (value & 0x7F);

		if ((((s1sweep & 0x70) >> 4) == 0) || ((s1sweep & 0x07) == 0) || (!TestBit(s1sweep, 3) && s1negative))
		{
		    s1sweepenabled = false;
		}
	    }

	    inline void reloads1lengthcounter()
	    {
		s1lengthcounter = (64 - (s1soundlength & 0x3F));
		s1soundlength &= 0xC0;	
	    }

	    inline void sets1dutycycle()
	    {
		switch ((s1soundlength & 0xC0) >> 6)
		{
		    case 0: s1dutycycle = {{false, false, false, false, false, false, false, true}}; break;
		    case 1: s1dutycycle = {{true, false, false, false, false, false, false, true}}; break;
		    case 2: s1dutycycle = {{true, false, false, false, false, true, true, true}}; break;
		    case 3: s1dutycycle = {{false, true, true, true, true, true, true, false}}; break;
		    default: break;
		}
	    }

	    inline void s1writereset(uint8_t value)
	    {
		bool lengthwasenable = TestBit(s1freqhi, 6);
		s1freqhi = (value & 0xC7);

		if (apulength() && !lengthwasenable && TestBit(s1freqhi, 6) && s1lengthcounter > 0)
		{
		    s1lengthcounter -= 1;

		    if (s1lengthcounter == 0)
		    {
			s1enabled = false;
		    }
		}

		if (TestBit(s1freqhi, 7))
		{
		    s1resetchannel();
		}
	    }

	    inline uint16_t s1sweepcalc()
	    {
		uint16_t freqdelta = (s1shadowfreq >> (s1sweep & 0x07));

		if (TestBit(s1sweep, 3))
		{
		    freqdelta *= -1;
		    freqdelta &= 0x7FF;

		    s1negative = true;
		}

		uint16_t newfreq = ((s1shadowfreq + freqdelta) & 0x7FF);

		if (newfreq > 2047)
		{
		    s1sweepenabled = false;
		    s1enabled = false;
		}

		return newfreq;
	    }

	    inline void s1resetchannel()
	    {
		s1enabled = true;
		s1reloadperiod();
		s1freqhi &= 0x7F;

		s1shadowfreq = (s1freqlo | ((s1freqhi & 0x7) << 8));
		s1sweepcounter = ((s1sweep & 0x70) >> 4);
		s1sweepenabled = (s1sweepcounter != 0 && ((s1sweep & 0x07) != 0));
		s1sweepcalc();

		s1negative = false;

		s1volume = ((s1volumeenvelope & 0xF0) >> 4);
		s1envelopecounter = (s1volumeenvelope & 0x07);
		s1envelopeenabled = (s1envelopecounter != 0);


		if ((!TestBit(s1volumeenvelope, 3) && s1volume == 0) || (TestBit(s1volumeenvelope, 3) && s1volume == 0x0F))
		{
		    s1envelopeenabled = false;
		}

		if (s1lengthcounter == 0)
		{
		    s1lengthcounter = 64;

		    if (apulength() && TestBit(s1freqhi, 6))
		    {
			s1lengthcounter -= 1;
		    }
		}
	    }

	    inline void s1reloadperiod()
	    {
		int frequency = (s1freqlo | ((s1freqhi & 0x07) << 8));
		s1periodtimer = ((2048 - frequency) << 1);
	    }

	    inline bool s2enabledleft()
	    {
		return (s2enabled && (TestBit(soundselect, 5)));
	    }

	    inline bool s2enabledright()
	    {
		return (s2enabled && (TestBit(soundselect, 1)));
	    }

	    inline void reloads2lengthcounter()
	    {
		s2lengthcounter = (64 - (s2soundlength & 0x3F));
		s2soundlength &= 0xC0;	
	    }

	    inline void sets2dutycycle()
	    {
		switch ((s2soundlength & 0xC0) >> 6)
		{
		    case 0: s2dutycycle = {{false, false, false, false, false, false, false, true}}; break;
		    case 1: s2dutycycle = {{true, false, false, false, false, false, false, true}}; break;
		    case 2: s2dutycycle = {{true, false, false, false, false, true, true, true}}; break;
		    case 3: s2dutycycle = {{false, true, true, true, true, true, true, false}}; break;
		    default: break;
		}
	    }

	    inline void s2writereset(uint8_t value)
	    {
		bool lengthwasenable = TestBit(s2freqhi, 6);
		s2freqhi = (value & 0xC7);

		if (apulength() && !lengthwasenable && TestBit(s2freqhi, 6) && s2lengthcounter > 0)
		{
		    s2lengthcounter -= 1;

		    if (s2lengthcounter == 0)
		    {
			s2enabled = false;
		    }
		}

		if (TestBit(s2freqhi, 7))
		{
		    s2resetchannel();
		}
	    }

	    inline void s2resetchannel()
	    {
		s2enabled = true;
		s2reloadperiod();
		s2freqhi &= 0x7F;

		s2volume = ((s2volumeenvelope & 0xF0) >> 4);
		s2envelopecounter = (s2volumeenvelope & 0x07);
		s2envelopeenabled = (s2envelopecounter != 0);


		if ((!TestBit(s2volumeenvelope, 3) && s2volume == 0) || (TestBit(s2volumeenvelope, 3) && s2volume == 0x0F))
		{
		    s2envelopeenabled = false;
		}

		if (s2lengthcounter == 0)
		{
		    s2lengthcounter = 64;

		    if (apulength() && TestBit(s2freqhi, 6))
		    {
			s2lengthcounter -= 1;
		    }
		}
	    }

	    inline void s2reloadperiod()
	    {
		int frequency = (s2freqlo | ((s2freqhi & 0x07) << 8));
		s2periodtimer = ((2048 - frequency) << 1);
	    }

	    inline bool waveenabledleft()
	    {
		return (waveenabled && (TestBit(soundselect, 6)));
	    }

	    inline bool waveenabledright()
	    {
		return (waveenabled && (TestBit(soundselect, 2)));
	    }

	    inline void reloadwavelengthcounter()
	    {
		wavelengthcounter = (256 - wavesoundlength);
		wavesoundlength = 0;	
	    }

	    inline void wavereloadperiod()
	    {
		int frequency = (wavefreqlo | ((wavefreqhi & 0x07) << 8));
		waveperiodtimer = (2048 - frequency);
	    }

	    inline void wavewritereset(uint8_t value)
	    {
		bool lengthwasenable = TestBit(wavefreqhi, 6);
		wavefreqhi = (value & 0xC7);

		if (apulength() && !lengthwasenable && TestBit(wavefreqhi, 6) && wavelengthcounter > 0)
		{
		    wavelengthcounter -= 1;

		    if (wavelengthcounter == 0)
		    {
			waveenabled = false;
		    }
		}

		if (TestBit(wavefreqhi, 7))
		{
		    waveresetchannel();
		}
	    }

	    inline void waveresetchannel()
	    {
		waveenabled = true;
		wavereloadperiod();
		wavefreqhi &= 0x7F;

		if (wavelengthcounter == 0)
		{
		    wavelengthcounter = 256;

		    if (apulength() && TestBit(wavefreqhi, 6))
		    {
			wavelengthcounter -= 1;
		    }
		}

		wavepos = 0;
		waveenabled = TestBit(wavesweep, 7);
		wavecurrentsample = wavelastplayedsample;
	    }

	    inline bool noiseenabledleft()
	    {
		return (noiseenabled && (TestBit(soundselect, 7)));
	    }

	    inline bool noiseenabledright()
	    {
		return (noiseenabled && (TestBit(soundselect, 3)));
	    }

	    inline void reloadnoiselengthcounter()
	    {
		noiselengthcounter = (64 - (noisesoundlength & 0x3F));
		noisesoundlength &= 0xC0;
	    }

	    inline void writenoiseenvelope(uint8_t value)
	    {
		noisevolumeenvelope = value;

		if (((noisevolumeenvelope & 0xF0) >> 4) == 0)
		{
		    noiseenabled = false;
		}
	    }

	    inline void noisewritereset(uint8_t value)
	    {
		bool lengthwasenable = TestBit(noisefreqhi, 6);
		noisefreqhi = (value & 0xC0);

		if (apulength() && !lengthwasenable && TestBit(noisefreqhi, 6) && noiselengthcounter > 0)
		{
		    noiselengthcounter -= 1;

		    if (noiselengthcounter == 0)
		    {
			noiseenabled = false;
		    }
		}

		if (TestBit(noisefreqhi, 7))
		{
		    noiseresetchannel();
		}
	    }

	    inline void noisereloadperiod()
	    {
		uint32_t clockdivider = max(((noisefreqlo & 0x07) << 1), 1);
		noiseperiodtimer = (clockdivider << (((noisefreqlo & 0xF0) >> 4) + 2));
	    }

	    inline void noiseresetchannel()
	    {
		noiseenabled = true;
		noisereloadperiod();
		noisefreqhi &= 0x7F;

		noisevolume = ((noisevolumeenvelope & 0xF0) >> 4);
		noiseenvelopecounter = (noisevolumeenvelope & 0x07);
		noiseenvelopeenabled = (noiseenvelopecounter != 0);


		if ((!TestBit(noisevolumeenvelope, 3) && noisevolume == 0) || (TestBit(noisevolumeenvelope, 3) && noisevolume == 0x0F))
		{
		    noiseenvelopeenabled = false;
		}

		if (noiselengthcounter == 0)
		{
		    noiselengthcounter = 64;

		    if (apulength() && TestBit(noisefreqhi, 6))
		    {
			noiselengthcounter -= 1;
		    }
		}
	
		noiselfsr = 0xFFFF;

		if (noisevolume == 0)
		{
		    noiseenabled = false;
		}
	    }

	    inline void writesoundon(uint8_t value)
	    {
		bool wasenabled = TestBit(soundon, 7);
		soundon = (value & 0x80);

		if (wasenabled && !TestBit(soundon, 7))
		{
		    clearregisters();
		}
	    }

	    inline void clearregisters()
	    {
	    	s1sweep = 0;
	    	s1negative = false;
	    	s1sweepenabled = false;
	    	s1shadowfreq = 0;
	    	s1sweepcounter = 0;
	    	s1soundlength = 0;

		if (gameboy != Console::DMG)
		{
	    	    s1lengthcounter = 0;
		}

	    	s1volumeenvelope = 0;
	    	s1envelopeenabled = false;
	    	s1envelopecounter = 0;
	    	s1volume = 0;
	    	s1freqlo = 0;
	    	s1freqhi = 0;
	    	s1periodtimer = 0;
	    	s1enabled = false;

	    	s2soundlength = 0;

		if (gameboy != Console::DMG)
		{
	    	    s2lengthcounter = 0;
		}

	    	s2volumeenvelope = 0;
	    	s2envelopeenabled = false;
	    	s2envelopecounter = 0;
	    	s2volume = 0;
	    	s2freqlo = 0;
	    	s2freqhi = 0;
	    	s2periodtimer = 0;
	    	s2enabled = false;

	    	wavesweep = 0;
	    	wavesoundlength = 0;

		if (gameboy != Console::DMG)
		{
	    	    wavelengthcounter = 0;
		}

	    	wavevolumeenvelope = 0;
	    	wavevolume = 0;
	    	wavefreqlo = 0;
	    	wavefreqhi = 0;
	    	waveperiodtimer = 0;
	    	waveenabled = false;
	    	waveramlengthmask = 0;
	    	wavepos = 0;
	    	wavechannelenabled = false;
	    	wavecurrentsample = 0;
	    	wavelastplayedsample = 0;

	    	noisesoundlength = 0;
		
		if (gameboy != Console::DMG)
		{
	    	    noiselengthcounter = 0;
		}

	    	noisevolumeenvelope = 0;
	    	noiseenvelopeenabled = false;
	    	noiseenvelopecounter = 0;
	    	noiseperiodtimer = 0;
	    	noisefreqlo = 0;
	    	noisefreqhi = 0;
	    	noisevolume = 0;
	    	noiselfsr = 1;
	    	noiseenabled = false;

	    	mastervolume = 0;
	    	soundselect = 0;
	    	soundon = 0;
	    }

	    uint8_t readsoundon()
	    {
		uint8_t temp = soundon;
		temp |= 0x70;
		temp |= ((int)(s1enabled) << 0);
		temp |= ((int)(s2enabled) << 1);
		temp |= ((int)(waveenabled) << 2);
		temp |= ((int)(noiseenabled) << 3);

		return temp;
	    }

    };
};

#endif // LIBMBGB_MMU
