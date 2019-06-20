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

	    uint8_t store8frommem(uint16_t reg)
	    {
		uint8_t temp = mem.readByte(reg);
		hardwaretick(4);
		return temp;
	    }

	    inline void xorreg(uint8_t reg)
	    {
		af.sethi(af.gethi() ^ reg);
		setzero(af.gethi() == 0);
		setsubtract(false);
		sethalf(false);
		setcarry(false);
	    }

	    inline void cmpreg(uint8_t reg)
	    {
		uint8_t temp = (af.gethi() - reg);
		setzero(temp == 0);
		setsubtract(true);
		sethalf(((af.gethi() & 0x0F) - (reg & 0x0F)) < 0);
		setcarry(af.gethi() < reg);
	    }

	    inline uint8_t incregbyte(uint8_t reg)
	    {
		uint8_t temp = reg;
		sethalf((reg & 0x0F) == 0x0F);
		temp += 1;
		setzero((temp == 0));
		return temp;
	    }

	    inline uint8_t decregbyte(uint8_t reg)
	    {
		uint8_t temp = reg;
		sethalf((reg & 0x0F) == 0x00);
		temp -= 1;
		setzero((temp == 0));
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

	    inline void call(uint16_t addr)
	    {
		hardwaretick(4);
		sp -= 2;
		hardwaretick(4);
		mem.writeWord(sp, pc);
		hardwaretick(4);

		pc = addr;
	    }

	    inline int callcond(uint16_t addr, bool cond)
	    {
		int temp = 0;

		if (cond)
		{
		    call(addr);
		    temp = 24;
		}
		else
		{
		    hardwaretick(8);
		    pc += 2;
		    temp = 12;
		}

		return temp;
	    }

	    inline void ret()
	    {
		uint16_t temp = 0;
		hardwaretick(4);
		temp = mem.readWord(sp);
		hardwaretick(4);
		sp += 2;
		hardwaretick(4);
		pc = temp;
	    }

	    inline int retcond(bool cond)
	    {
		int temp = 0;

		hardwaretick(4);
		
		if (cond)
		{
		    ret();
		    temp = 20;
		}
		else
		{
		    temp = 8;
		}

		return temp;
	    }

	    inline void push(uint16_t reg)
	    {
		hardwaretick(4);
		sp -= 2;
		hardwaretick(4);
		mem.writeWord(sp, reg);
		hardwaretick(4);
	    }

	    inline uint16_t pop()
	    {
		uint16_t temp = 0;
		hardwaretick(4);
		temp = mem.readWord(sp);
		hardwaretick(4);
		sp += 2;

		return temp;
	    }

	    inline uint8_t rl(uint8_t reg)
	    {
		uint8_t temp = reg;
		bool carryval = TestBit(reg, 7);

		temp = ((reg << 1) | (iscarry() ? 1 : 0));

		setzero(temp == 0);

		setsubtract(false);
		sethalf(false);
		setcarry(carryval);

		return temp;
	    }

	    inline uint8_t rlc(uint8_t reg)
	    {
		uint8_t temp = reg;

		setcarry(TestBit(reg, 7));

		temp = ((reg << 1) | (reg >> 7));

		setzero(temp == 0);

		setsubtract(false);
		sethalf(false);

		return temp;
	    }

	    inline uint8_t rr(uint8_t reg)
	    {
		uint8_t temp = reg;
		bool carryval = TestBit(reg, 0);

		temp = ((reg >> 1) | ((iscarry() ? 1 : 0) << 7));

		setzero(temp == 0);

		setsubtract(false);
		sethalf(false);
		setcarry(carryval);

		return temp;
	    }

	    inline uint8_t rrc(uint8_t reg)
	    {
		uint8_t temp = reg;

		setcarry(TestBit(reg, 0));

		temp = ((reg >> 1) | (reg << 7));

		setzero(temp == 0);

		setsubtract(false);
		sethalf(false);

		return temp;
	    }

	    inline void bit(uint8_t reg, int bit)
	    {
		setzero(!TestBit(reg, bit));
		setsubtract(false);
		sethalf(true);
	    }

	    void setzero(bool val)
	    {
		uint8_t temp = af.getlo();
		temp = (val) ? BitSet(temp, 7) : BitReset(temp, 7);
		af.setlo(temp);
	    }

	    bool iszero()
	    {
		return TestBit(af.getlo(), 7);
	    }

	    void setsubtract(bool val)
	    {
		uint8_t temp = af.getlo();
		temp = (val) ? BitSet(temp, 6) : BitReset(temp, 6);
		af.setlo(temp);
	    }

	    bool issubtract()
	    {
		return TestBit(af.getlo(), 6);
	    }

	    void sethalf(bool val)
	    {
		uint8_t temp = af.getlo();
		temp = (val) ? BitSet(temp, 5) : BitReset(temp, 5);
		af.setlo(temp);
	    }

	    bool ishalf()
	    {
		return TestBit(af.getlo(), 5);
	    }

	    void setcarry(bool val)
	    {
		uint8_t temp = af.getlo();
		temp = (val) ? BitSet(temp, 4) : BitReset(temp, 4);
		af.setlo(temp);
	    }

	    bool iscarry()
	    {
		return TestBit(af.getlo(), 4);
	    }
	
    };
};


#endif // LIBMBGB_CPU