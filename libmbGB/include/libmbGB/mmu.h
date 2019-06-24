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
	    vector<uint8_t> bios;

	    bool externalrampres = false;
	    bool biosload = false;
	    int ramsize = 0;

	    void determinembctype(vector<uint8_t>& rom);
	    void determineramsize(vector<uint8_t>& rom);
	    string determinegametitle(vector<uint8_t>& rom);
	    string mbctype;

	    bool loadBIOS(string filename);
	    bool loadROM(string filename);

	    uint8_t readByte(uint16_t addr);
	    void writeByte(uint16_t addr, uint8_t value);
	    uint16_t readWord(uint16_t addr);
	    void writeWord(uint16_t addr, uint16_t value);

	    uint8_t readIO(uint16_t addr);
	    void writeIO(uint16_t addr, uint8_t value);

	    poweronfunc poweron;

	    inline bool islcdenabled()
	    {
		return TestBit(lcdc, 7);
	    }

	    inline bool isbgenabled()
	    {
		return TestBit(lcdc, 0);
	    }

	    inline void setstatmode(int mode)
	    {
		stat = ((stat & 0xFC) | mode);
	    }

	    inline void writelcdc(uint8_t value)
	    {
		bool lcdwasenabled = islcdenabled();

		lcdc = value;

		poweron(lcdwasenabled);
	    }

	    void setpoweroncallback(poweronfunc cb)
	    {
		poweron = cb;
	    }

	    uint8_t lcdc = 0x91;
	    uint8_t stat = 0x01;
	    uint8_t scrolly = 0x00;
	    uint8_t scrollx = 0x00;
	    uint8_t ly = 0x00;
	    uint8_t lyc = 0x00;
	    uint8_t bgpalette = 0xFC;
    };
};

#endif // LIBMBGB_MMU