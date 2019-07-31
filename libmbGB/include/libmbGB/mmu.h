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

    class LIBMBGB_API MMU
    {
	public:
	    MMU();
	    ~MMU();

	    Console gameboy = Console::Default;
	    Mode gbmode = Mode::Default;
	    MBCType gbmbc;

	    void init();
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
		ly = 0x00;
	    	tima = 0x00;
	    	tma = 0x00;
	        tac = 0xF8;
		lcdc = 0x91;
		writeByte(0xFF10, 0x80);
		writeByte(0xFF11, 0xBF);
		writeByte(0xFF12, 0xF3);
		writeByte(0xFF14, 0xBF);
		writeByte(0xFF16, 0x3F);
		writeByte(0xFF19, 0xBF);
		writeByte(0xFF1A, 0x7F);
		writeByte(0xFF1B, 0xFF);
		writeByte(0xFF1C, 0x9F);
		writeByte(0xFF1E, 0xBF);
		writeByte(0xFF20, 0xFF);
		writeByte(0xFF23, 0xBF);
		writeByte(0xFF24, 0x77);
		writeByte(0xFF25, 0xF3);
		writeByte(0xFF26, 0xF1);
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
		return (gameboy == Console::CGB);
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

	    poweronfunc poweron;
	    joypadfunc updatep1;
	    statirqfunc statirq;
	    screenfunc screen;

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

	    int s1sweepshift = 0;
	    bool s1sweepnegate = false;
	    int s1sweepperiodload = 0;
	    int s1sweepperiod = 0;
	    bool s1sweepenable = false;
	    uint16_t s1sweepshadow = 0;
	    int s1duty = 0;
	    int s1lengthload = 0;
	    bool s1envaddmode = false;
	    bool s1envrunning = false;
	    int s1envperiodload = 0;
	    int s1envperiod = 0;
	    int s1volumeload = 0;
	    int s1volume = 0;
	    bool s1dacenabled = false;
	    uint16_t s1freq = 0x0000;
	    int s1timer = 0;
	    bool s1lengthenabled = false;
	    bool s1triggerbit = false;
	    int s1lengthcounter = 0;
	    bool s1enabled = false;
	    int s1outputvol = 0;

	    int s2duty = 0;
	    int s2lengthload = 0;
	    bool s2envaddmode = false;
	    bool s2envrunning = false;
	    int s2envperiodload = 0;
	    int s2envperiod = 0;
	    int s2volumeload = 0;
	    int s2volume = 0;
	    bool s2dacenabled = false;
	    uint16_t s2freq = 0x0000;
	    int s2timer = 0;
	    bool s2lengthenabled = false;
	    bool s2triggerbit = false;
	    int s2lengthcounter = 0;
	    bool s2enabled = false;
	    int s2outputvol = 0;

	    bool wavedacenabled = false;
	    uint8_t wavelengthload = 0x00;
	    int wavevolumecode = 0;
	    uint16_t wavefreq = 0x0000;
	    int wavetimer = 0;
	    bool wavelengthenabled = false;
	    bool wavetriggerbit = false;
	    uint8_t outputbyte = 0x00;
	    uint8_t lastoutputbyte = 0x00;
	    bool waveenabled = false;
	    int wavelengthcounter = 0;
	    int wavepositioncounter = 0;
	    uint8_t waveram[0x10] = {0};
	    int waveoutputvol = 0;

	    int noiselengthload = 0;
	    bool noiseenvaddmode = false;
	    bool noiseenvrunning = false;
	    int noiseenvperiodload = 0;
	    int noiseenvperiod = 0;
	    int noisevolumeload = 0;
	    int noisevolume = 0;
	    bool noisedacenabled = false;
	    int noisedivisor = 0;
	    int noisetimer = 0;
	    bool noisewidthmode = false;
	    int noiseclockshift = 0;
	    bool noiselengthenabled = false;
	    bool noisetriggerbit = false;
	    int noiselengthcounter = 0;
	    bool noiseenabled = false;
	    int noiseoutputvol = 0;
	    int lfsr = 0;
	    int divisors[8] =
	    {
		8, 16, 32, 48,
		64, 80, 96, 112
	    };


	
	    int rightvol = 0;
	    bool vinrightenable = false;
	    int leftvol = 0;
	    bool vinleftenable = false;

	    bool rightenables[4] = {false};
	    bool leftenables[4] = {false};

	    bool soundenabled = true;

	    uint8_t lylastcycle = 0xFF;

	    inline uint16_t sweepcalc()
	    {
		uint16_t newfreq = 0;
		newfreq = (s1sweepshadow >> s1sweepshift);

		if (s1sweepnegate)
		{
		    newfreq = (s1sweepshadow - newfreq);
		}
		else
		{
		    newfreq = (s1sweepshadow + newfreq);
		}

		if (newfreq > 2047)
		{
		    s1enabled = false;
		}

		return newfreq;

	    }

	    inline void s1trigger()
	    {
		s1enabled = true;
		if (s1lengthcounter == 0)
		{
		    s1lengthcounter = (64 - (s1lengthload & 0x3F));
		    s1lengthload &= 0xC0;
		}

		s1timer = ((2048 - s1freq) * 4);
		s1envrunning = true;
		s1envperiod = s1envperiodload;
		s1volume = s1volumeload;

		s1sweepshadow = s1freq;
		s1sweepperiod = s1sweepperiodload;

		if (s1sweepperiod == 0)
		{
		    s1sweepperiod = 8;
		}

		s1sweepenable = ((s1sweepperiod > 0) || (s1sweepshift > 0));

		if (s1sweepshift > 0)
		{
		    sweepcalc();
		}
	    }

	    inline void s2trigger()
	    {
		s2enabled = true;
		if (s2lengthcounter == 0)
		{
		    s2lengthcounter = (64 - (s2lengthload & 0x3F));
		    s2lengthload &= 0xC0;
		}

		s2timer = ((2048 - s2freq) * 4);
		s2envrunning = true;
		s2envperiod = s2envperiodload;
		s2volume = s2volumeload;
	    }

	    inline void wavetrigger()
	    {
		waveenabled = true;
		if (wavelengthcounter == 0)
		{
		    wavelengthcounter = 256;
		}

		wavetimer = ((2048 - wavefreq) * 2);
		wavepositioncounter = 0;
	    }

	    inline void noisetrigger()
	    {
		noiseenabled = true;
		if (noiselengthcounter == 0)
		{
		    noiselengthcounter = (64 - (noiselengthload & 0x3F));
		    noiselengthload &= 0xC0;
		}

		noisetimer = divisors[noisedivisor];
		noiseenvrunning = true;
		noiseenvperiod = noiseenvperiodload;
		noisevolume = noisevolumeload;

		lfsr = 0x7FFF;
	    }

	    inline uint8_t getsoundenabled()
	    {
		uint8_t temp = 0;

		if (soundenabled)
		{
		    temp |= (soundenabled << 7);
		}

		temp |= (waveenabled << 2);
		temp |= (noiseenabled << 3);
		temp |= (s2enabled << 1);
		temp |= (s1enabled);
		return temp;
	    }
    };
};

#endif // LIBMBGB_MMU
