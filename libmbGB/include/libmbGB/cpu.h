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

	    void printregs();

	    uint8_t getimmbyte()
	    {
		uint8_t temp = mem.readByte(pc++);
		hardwaretick(4);
		return temp;
	    }

	    int8_t getimmsignedbyte()
	    {
		int8_t temp = (int8_t)(mem.readByte(pc++));
		hardwaretick(4);
		return temp;
	    }

	    uint16_t getimmword()
	    {
		hardwaretick(4);
		uint16_t temp = mem.readWord(pc);
		hardwaretick(4);

		pc += 2;

		return temp;
	    }

	    void load8intomem(uint16_t reg, uint8_t imm)
	    {
		mem.writeByte(reg, imm);
		hardwaretick(4);
	    }

	    inline void xorreg(uint8_t reg)
	    {
		af.sethi(af.gethi() ^ reg);
		af.setlo(setzero(af.getlo(), af.gethi() == 0));
		af.setlo(setsubtract(af.getlo(), false));
		af.setlo(sethalf(af.getlo(), false));
		af.setlo(setcarry(af.getlo(), false));
	    }

	    inline uint8_t incregbyte(uint8_t reg)
	    {
		uint8_t temp = reg;
		af.setlo(sethalf(af.getlo(), ((reg & 0x0F) == 0x0F)));
		temp += 1;
		af.setlo(setzero(af.getlo(), (temp == 0)));
		return temp;
	    }

	    inline uint8_t decregbyte(uint8_t reg)
	    {
		uint8_t temp = reg;
		af.setlo(sethalf(af.getlo(), ((reg & 0x0F) == 0x00)));
		temp -= 1;
		af.setlo(setzero(af.getlo(), (temp == 0)));
		return temp;
	    }

	    inline uint16_t increg(uint16_t reg)
	    {
		return (reg + 1);
	    }

	    inline uint16_t decreg(uint16_t reg)
	    {
		return (reg - 1);
	    }

	    inline void reljump(int8_t imm)
	    {
		hardwaretick(4);
		pc += imm;
	    }

	    inline int reljumpcond(int8_t imm, bool cond)
	    {
		int temp = 0;		

		if (cond)
		{
		    reljump(imm);
		    temp = 12;
		}
		else
		{
		    hardwaretick(4);
		    temp = 8;
		}

		return temp;
	    }

	    inline void bit(uint8_t reg, int bit)
	    {
		af.setlo(setzero(af.getlo(), !TestBit(reg, bit)));
		af.setlo(setsubtract(af.getlo(), false));
		af.setlo(sethalf(af.getlo(), true));
	    }

	    uint8_t setzero(uint8_t reg, bool val)
	    {
		uint8_t temp = reg;
		temp = (val) ? BitSet(temp, 7) : BitReset(temp, 7);
		return temp;
	    }

	    uint8_t setsubtract(uint8_t reg, bool val)
	    {
		uint8_t temp = reg;
		temp = (val) ? BitSet(temp, 6) : BitReset(temp, 6);
		return temp;
	    }

	    uint8_t sethalf(uint8_t reg, bool val)
	    {
		uint8_t temp = reg;
		temp = (val) ? BitSet(temp, 5) : BitReset(temp, 5);
		return temp;
	    }

	    uint8_t setcarry(uint8_t reg, bool val)
	    {
		uint8_t temp = reg;
		temp = (val) ? BitSet(temp, 4) : BitReset(temp, 4);
		return temp;
	    }
	
    };
};


#endif // LIBMBGB_CPU