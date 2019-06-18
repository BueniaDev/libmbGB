// This file is part of libmbGB.
// Copyright (C) 2019 Buenia.ui
// libmbGB is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// libmbGB is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with libmbGB.  If not, see <https://www.gnu.org/licenses/>.

#ifndef LIBMBGB_CPU
#define LIBMBGB_CPU

#include "mmu.h"
#include "libmbgb_api.h"
#include <cstdint>
#include <iostream>
using namespace gb;
using namespace std;

namespace gb
{
    struct Register
    {
	uint8_t hi;
	uint8_t lo;
	uint16_t reg;

	uint8_t gethi()
	{
	    return hi;
	}

	void sethi(uint8_t val)
	{
	    hi = val;
	    reg = ((val << 8) | (reg & 0xFF));
	}

	uint8_t getlo()
	{
	    return lo;
	}

	void setlo(uint8_t val)
	{
	    lo = val;
	    reg = ((reg & 0xFF00) | (val));
	}

	uint16_t getreg()
	{
	    return reg;
	}

	void setreg(uint16_t val)
	{
	    reg = val;
	    hi = (val >> 8);
	    lo = (val & 0xFF);
	}
    };

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

    class LIBMBGB_API CPU
    {
	public:
	    CPU(MMU& memory);
	    ~CPU();

	    CPUState state = CPUState::Running;

	    void init();
	    void initnobios();
	    void initbios();
	    void shutdown();

	    MMU& mem;

	    Register af;
	    Register bc;
	    Register de;
	    Register hl;
	    uint16_t pc;
	    uint16_t sp;

	    int runfor(int cycles);
	    void hardwaretick(int cycles);
	    int executenextopcode(uint8_t opcode);
	    int executenextcbopcode(uint8_t opcode);

	    uint16_t getimmword();
    };
};


#endif // LIBMBGB_CPU