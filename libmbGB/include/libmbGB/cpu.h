/*
    This file is part of libmbGB.
    Copyright (C) 2022 BueniaDev.

    libmbGB is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    libmbGB is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with libmbGB.  If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef LIBMBGB_CPU_H
#define LIBMBGB_CPU_H

#include "mmu.h"
using namespace gb;

namespace gb
{
    struct GBStatus
    {
	uint16_t af = 0;
	uint16_t bc = 0;
	uint16_t de = 0;
	uint16_t hl = 0;
	uint16_t pc = 0;
	uint16_t sp = 0;
    };

    enum GBConflict : int
    {
	ReadOld = 0,
	ReadNew = 1,
	WriteCPU = 2,
	PaletteDMG = 3,
	PaletteCGB = 4,
    };

    class LIBMBGB_API GBRegister
    {
	public:
	    GBRegister()
	    {

	    }

	    ~GBRegister()
	    {

	    }

	    uint8_t& hi()
	    {
		return high;
	    }

	    uint8_t& lo()
	    {
		return low;
	    }

	    operator uint16_t() const
	    {
		return ((high << 8) | low);
	    }

	    GBRegister& operator=(uint16_t data)
	    {
		low = (data & 0xFF);
		high = (data >> 8);
		return *this;
	    }

	    GBRegister operator++(int)
	    {
		auto old = *this;
		*this += 1;
		return old;
	    }

	    GBRegister operator--(int)
	    {
		auto old = *this;
		*this -= 1;
		return old;
	    }

	    GBRegister& operator+=(int data)
	    {
		*this = (*this + data);
		return *this;
	    }

	    GBRegister& operator-=(int data)
	    {
		*this = (*this - data);
		return *this;
	    }

	private:
	    uint8_t low = 0;
	    uint8_t high = 0;
    };

    class LIBMBGB_API GBCPU
    {
	public:
	    GBCPU(GBMMU &mem);
	    ~GBCPU();

	    void init();
	    void shutdown();

	    void runInstruction();
	    void debugOutput(bool print_dasm = true);

	    void doSavestate(mbGBSavestate &file);

	    GBStatus getStatus()
	    {
		return internal_status;
	    }

	    size_t disassembleInstr(ostream &stream, uint16_t addr);

	private:
	    GBMMU &memory;

	    GBStatus internal_status;

	    unordered_map<uint8_t, GBConflict> cgbConflicts = 
	    {
		{0x0F, WriteCPU},
		{0x45, WriteCPU},
		{0x47, PaletteCGB},
		{0x48, PaletteCGB},
		{0x49, PaletteCGB},
	    };

	    unordered_map<uint8_t, GBConflict> dmgConflicts = 
	    {
		{0x0F, WriteCPU},
		{0x45, ReadOld},
		{0x40, ReadNew},
		{0x42, ReadNew},
		{0x47, PaletteDMG},
		{0x48, PaletteDMG},
		{0x49, PaletteDMG},

		{0x4A, ReadNew},
		{0x4B, ReadNew},
		{0x43, ReadNew},
	    };

	    void internalCycle()
	    {
		tick(4);
	    }

	    void setStatus()
	    {
		internal_status.af = reg_af;
		internal_status.bc = reg_bc;
		internal_status.de = reg_de;
		internal_status.hl = reg_hl;
		internal_status.pc = pc;
		internal_status.sp = sp;
	    }

	    uint16_t getRP(int reg)
	    {
		uint16_t data = 0;
		switch (reg)
		{
		    case 0: data = reg_bc; break;
		    case 1: data = reg_de; break;
		    case 2: data = reg_hl; break;
		    case 3: data = sp; break;
		}

		return data;
	    }

	    void setRP(int reg, uint16_t data)
	    {
		switch (reg)
		{
		    case 0: reg_bc = data; break;
		    case 1: reg_de = data; break;
		    case 2: reg_hl = data; break;
		    case 3: sp = data; break;
		}
	    }

	    uint16_t getRP2(int reg)
	    {
		uint16_t data = 0;
		switch (reg)
		{
		    case 0: data = reg_bc; break;
		    case 1: data = reg_de; break;
		    case 2: data = reg_hl; break;
		    case 3: data = reg_af; break;
		}

		return data;
	    }

	    void setRP2(int reg, uint16_t data)
	    {
		switch (reg)
		{
		    case 0: reg_bc = data; break;
		    case 1: reg_de = data; break;
		    case 2: reg_hl = data; break;
		    case 3: reg_af = (data & 0xFFF0); break;
		}
	    }

	    uint8_t getReg(int reg)
	    {
		uint8_t data = 0;
		switch (reg)
		{
		    case 0: data = reg_bc.hi(); break;
		    case 1: data = reg_bc.lo(); break;
		    case 2: data = reg_de.hi(); break;
		    case 3: data = reg_de.lo(); break;
		    case 4: data = reg_hl.hi(); break;
		    case 5: data = reg_hl.lo(); break;
		    case 6: data = readHL(); break;
		    case 7: data = reg_af.hi(); break;
		}

		return data;
	    }

	    void setReg(int reg, uint8_t data)
	    {
		switch (reg)
		{
		    case 0: reg_bc.hi() = data; break;
		    case 1: reg_bc.lo() = data; break;
		    case 2: reg_de.hi() = data; break;
		    case 3: reg_de.lo() = data; break;
		    case 4: reg_hl.hi() = data; break;
		    case 5: reg_hl.lo() = data; break;
		    case 6: writeHL(data); break;
		    case 7: reg_af.hi() = data; break;
		}
	    }

	    void setALU(int reg, uint8_t operand)
	    {
		switch (reg)
		{
		    case 0: addByte(operand); break;
		    case 1: adcByte(operand); break;
		    case 2: subByte(operand); break;
		    case 3: sbcByte(operand); break;
		    case 4: andByte(operand); break;
		    case 5: xorByte(operand); break;
		    case 6: orByte(operand); break;
		    case 7: cmpByte(operand); break;
		}
	    }

	    bool getCond(int reg)
	    {
		bool cond = false;
		reg &= 0x3;
		switch (reg)
		{
		    case 0: cond = !isZero(); break;
		    case 1: cond = isZero(); break;
		    case 2: cond = !isCarry(); break;
		    case 3: cond = isCarry(); break;
		}

		return cond;
	    }

	    uint16_t inc16(uint16_t data)
	    {
		internalCycle();
		return (data + 1);
	    }

	    uint16_t dec16(uint16_t data)
	    {
		internalCycle();
		return (data - 1);
	    }

	    void jumpRel8()
	    {
		int8_t offs = getImmByte();
		internalCycle();
		pc += offs;
	    }

	    void jumpRel8Cond(bool cond)
	    {
		int8_t offs = getImmByte();

		if (cond)
		{
		    internalCycle();
		    pc += offs;
		}
	    }

	    void push(uint16_t data)
	    {
		internalCycle();
		writeByte(--sp, (data >> 8));
		writeByte(--sp, (data & 0xFF));
	    }

	    uint16_t pop()
	    {
		uint8_t low = readByte(sp++);
		uint8_t high = readByte(sp++);
		return ((high << 8) | low);
	    }

	    void jump()
	    {
		uint16_t addr = getImmWord();
		internalCycle();
		pc = addr;
	    }

	    void jumpCond(bool cond)
	    {
		uint16_t addr = getImmWord();

		if (cond)
		{
		    internalCycle();
		    pc = addr;
		}
	    }

	    void call()
	    {
		uint16_t addr = getImmWord();
		push(pc);
		pc = addr;
	    }

	    void callCond(bool cond)
	    {
		uint16_t addr = getImmWord();

		if (cond)
		{
		    push(pc);
		    pc = addr;
		}
	    }

	    void ret()
	    {
		uint16_t addr = pop();
		internalCycle();
		pc = addr;
	    }

	    void reti()
	    {
		ret();
		is_ime = true;
	    }

	    void retCond(bool cond)
	    {
		internalCycle();

		if (cond)
		{
		    uint16_t addr = pop();
		    internalCycle();
		    pc = addr;
		}
	    }

	    void rst(int reg)
	    {
		uint16_t addr = (reg * 8);
		push(pc);
		pc = addr;
	    }

	    void stop()
	    {
		pc += 1;

		uint8_t data = memory.readByte(0xFF40);

		memory.writeByte(0xFF40, (data & 0x7F));

		if (memory.isCGB())
		{
		    if (memory.prepareSpeedSwitch())
		    {
			memory.commitSpeedSwitch();
			memory.writeByte(0xFF40, data);
		    }
		}
		else
		{
		    is_stopped = true;
		}
	    }

	    void halt()
	    {
		is_halted = true;

		bool is_irq = ((memory.getIE() & memory.getIF() & 0x1F) != 0);

		if (!is_ime && is_irq)
		{
		    is_halted = false;
		    is_halt_bug = true;
		}
	    }

	    uint8_t addInternal(uint8_t source, uint8_t operand, bool is_carry = false)
	    {
		uint16_t result = (source + operand + is_carry);
		uint16_t half_res = ((source & 0xF) + (operand & 0xF) + is_carry);
		
		setZero(uint8_t(result) == 0);
		setSubtract(false);
		setHalf(testbit(half_res, 4));
		setCarry(testbit(result, 8));
		return uint8_t(result);
	    }

	    uint16_t addInternal16(uint16_t source, uint16_t operand)
	    {
		uint32_t result = (source + operand);
		uint32_t half_res = ((source & 0xFFF) + (operand & 0xFFF));

		setSubtract(false);
		setHalf(testbit(half_res, 12));
		setCarry(testbit(result, 16));
		return uint16_t(result);
	    }

	    uint8_t subInternal(uint8_t source, uint8_t operand, bool is_carry = false)
	    {
		uint16_t result = (source - operand - is_carry);
		uint16_t half_res = ((source & 0xF) - (operand & 0xF) - is_carry);
		
		setZero(uint8_t(result) == 0);
		setSubtract(true);
		setHalf(testbit(half_res, 4));
		setCarry(testbit(result, 8));
		return uint8_t(result);
	    }

	    uint8_t andInternal(uint8_t source, uint8_t operand)
	    {
		uint8_t result = (source & operand);
		setZero(result == 0);
		setSubtract(false);
		setHalf(true);
		setCarry(false);
		return result;
	    }

	    uint8_t orInternal(uint8_t source, uint8_t operand)
	    {
		uint8_t result = (source | operand);
		setZero(result == 0);
		setSubtract(false);
		setHalf(false);
		setCarry(false);
		return result;
	    }

	    uint8_t xorInternal(uint8_t source, uint8_t operand)
	    {
		uint8_t result = (source ^ operand);
		setZero(result == 0);
		setSubtract(false);
		setHalf(false);
		setCarry(false);
		return result;
	    }

	    void addByte(uint8_t source)
	    {
		reg_af.hi() = addInternal(reg_af.hi(), source);
	    }

	    void adcByte(uint8_t source)
	    {
		reg_af.hi() = addInternal(reg_af.hi(), source, isCarry());
	    }

	    void addWord(uint16_t source)
	    {
		internalCycle();
		reg_hl = addInternal16(reg_hl, source);
	    }

	    void subByte(uint8_t source)
	    {
		reg_af.hi() = subInternal(reg_af.hi(), source);
	    }

	    void sbcByte(uint8_t source)
	    {
		reg_af.hi() = subInternal(reg_af.hi(), source, isCarry());
	    }

	    void andByte(uint8_t source)
	    {
		reg_af.hi() = andInternal(reg_af.hi(), source);
	    }

	    void orByte(uint8_t source)
	    {
		reg_af.hi() = orInternal(reg_af.hi(), source);
	    }

	    void xorByte(uint8_t source)
	    {
		reg_af.hi() = xorInternal(reg_af.hi(), source);
	    }

	    void cmpByte(uint8_t source)
	    {
		subInternal(reg_af.hi(), source);
	    }

	    uint8_t incByte(uint8_t source)
	    {
		uint8_t result = (source + 1);
		setZero(result == 0);
		setSubtract(false);
		setHalf((result & 0xF) == 0);
		return result;
	    }

	    uint8_t decByte(uint8_t source)
	    {
		uint8_t result = (source - 1);
		setZero(result == 0);
		setSubtract(true);
		setHalf((result & 0xF) == 0xF);
		return result;
	    }

	    void addSP()
	    {
		uint8_t data = getImmByte();
		internalCycle();
		internalCycle();

		uint16_t carry_res = ((sp & 0xFF) + data);
		uint16_t half_res = ((sp & 0xF) + (data & 0xF));

		setZero(false);
		setSubtract(false);
		setHalf(testbit(half_res, 4));
		setCarry(testbit(carry_res, 8));
		sp += int8_t(data);
	    }

	    void ldHLSP()
	    {
		uint8_t data = getImmByte();
		internalCycle();

		uint16_t carry_res = ((sp & 0xFF) + data);
		uint16_t half_res = ((sp & 0xF) + (data & 0xF));

		setZero(false);
		setSubtract(false);
		setHalf(testbit(half_res, 4));
		setCarry(testbit(carry_res, 8));
		reg_hl = (sp + int8_t(data));
	    }

	    uint8_t readHL()
	    {
		return readByte(reg_hl);
	    }

	    void writeHL(uint8_t data)
	    {
		writeByte(reg_hl, data);
	    }

	    uint8_t rotReg(int reg, uint8_t source)
	    {
		uint8_t data = 0;
		switch (reg)
		{
		    case 0: data = rotateLeftCarry(source); break;
		    case 1: data = rotateRightCarry(source); break;
		    case 2: data = rotateLeft(source); break;
		    case 3: data = rotateRight(source); break;
		    case 4: data = shiftLeftArith(source); break;
		    case 5: data = shiftRightArith(source); break;
		    case 6: data = swapReg(source); break;
		    case 7: data = shiftRightLogical(source); break;
		}

		return data;
	    }

	    uint8_t rotateLeftCarry(uint8_t source)
	    {
		bool is_carry = testbit(source, 7);
		uint8_t result = ((source << 1) | is_carry);
		setZero(result == 0);
		setSubtract(false);
		setHalf(false);
		setCarry(is_carry);
		return result;
	    }

	    uint8_t rotateLeft(uint8_t source)
	    {
		bool is_carry = testbit(source, 7);
		uint8_t result = ((source << 1) | isCarry());
		setZero(result == 0);
		setSubtract(false);
		setHalf(false);
		setCarry(is_carry);
		return result;
	    }

	    uint8_t rotateRightCarry(uint8_t source)
	    {
		bool is_carry = testbit(source, 0);
		uint8_t result = ((source >> 1) | (is_carry << 7));
		setZero(result == 0);
		setSubtract(false);
		setHalf(false);
		setCarry(is_carry);
		return result;
	    }

	    uint8_t rotateRight(uint8_t source)
	    {
		bool is_carry = testbit(source, 0);
		uint8_t result = ((source >> 1) | (isCarry() << 7));
		setZero(result == 0);
		setSubtract(false);
		setHalf(false);
		setCarry(is_carry);
		return result;
	    }

	    uint8_t shiftLeftArith(uint8_t source)
	    {
		bool is_carry = testbit(source, 7);
		uint8_t result = (source << 1);
		setZero(result == 0);
		setSubtract(false);
		setHalf(false);
		setCarry(is_carry);
		return result;
	    }

	    uint8_t shiftRightArith(uint8_t source)
	    {
		bool is_carry = testbit(source, 0);
		bool is_msb = testbit(source, 7);
		uint8_t result = ((source >> 1) | (is_msb << 7));
		setZero(result == 0);
		setSubtract(false);
		setHalf(false);
		setCarry(is_carry);
		return result;
	    }

	    uint8_t shiftRightLogical(uint8_t source)
	    {
		bool is_carry = testbit(source, 0);
		uint8_t result = (source >> 1);
		setZero(result == 0);
		setSubtract(false);
		setHalf(false);
		setCarry(is_carry);
		return result;
	    }

	    uint8_t swapReg(uint8_t source)
	    {
		uint8_t result = ((source << 4) | (source >> 4));
		setZero(result == 0);
		setSubtract(false);
		setHalf(false);
		setCarry(false);
		return result;
	    }

	    void bitReg(uint8_t source, int bit)
	    {
		setHalf(true);
		setSubtract(false);
		setZero(!testbit(source, bit));
	    }

	    uint8_t resReg(uint8_t source, int bit)
	    {
		return resetbit(source, bit);
	    }

	    uint8_t setReg(uint8_t source, int bit)
	    {
		return setbit(source, bit);
	    }

	    void rotateLeftA()
	    {
		reg_af.hi() = rotateLeft(reg_af.hi());
		setZero(false);
	    }

	    void rotateLeftCarryA()
	    {
		reg_af.hi() = rotateLeftCarry(reg_af.hi());
		setZero(false);
	    }

	    void rotateRightA()
	    {
		reg_af.hi() = rotateRight(reg_af.hi());
		setZero(false);
	    }

	    void rotateRightCarryA()
	    {
		reg_af.hi() = rotateRightCarry(reg_af.hi());
		setZero(false);
	    }

	    void daa()
	    {
		uint8_t result = reg_af.hi();

		if (isSubtract())
		{
		    if (isCarry())
		    {
			result -= 0x60;
		    }

		    if (isHalf())
		    {
			result -= 0x06;
		    }
		}
		else
		{
		    if (isCarry() || (reg_af.hi() > 0x99))
		    {
			result += 0x60;
			setCarry(true);
		    }

		    if (isHalf() || ((reg_af.hi() & 0xF) > 0x9))
		    {
			result += 0x06;
		    }
		}

		setZero(result == 0);
		setHalf(false);

		reg_af.hi() = result;
	    }

	    void compA()
	    {
		reg_af.hi() = ~reg_af.hi();
		setSubtract(true);
		setHalf(true);
	    }

	    void compCarry()
	    {
		setCarry(!isCarry());
		setSubtract(false);
		setHalf(false);
	    }

	    void setCarry()
	    {
		setCarry(true);
		setSubtract(false);
		setHalf(false);
	    }

	    void disableInterrupts()
	    {
		is_ime = false;
	    }

	    void enableInterrupts()
	    {
		if (!is_ime && !ime_toggle)
		{
		    ime_toggle = true;
		}
	    }

	    void ldSPHL()
	    {
		sp = reg_hl;
		tick(4);
	    }

	    void writeSP()
	    {
		uint16_t addr = getImmWord();
		writeByte(addr, (sp & 0xFF));
		writeByte((addr + 1), (sp >> 8));
	    }

	    bool dump = false;

	    void tick(int cycles);

	    void executeInstruction(uint8_t instr);
	    void extendedInstruction(uint8_t instr);

	    void disassembleExtended(ostream &stream, uint8_t instr);

	    void unrecognizedInstruction(bool is_prefix, int op_x, int op_y, int op_z);

	    uint8_t readByte(uint16_t addr);
	    void writeByte(uint16_t addr, uint8_t data);

	    uint8_t getImmByte();
	    uint16_t getImmWord();

	    GBRegister reg_af;
	    GBRegister reg_bc;
	    GBRegister reg_de;
	    GBRegister reg_hl;

	    uint16_t pc = 0;
	    uint16_t sp = 0;

	    bool is_ime = false;
	    bool ime_toggle = false;
	    bool is_halted = false;
	    bool is_halt_bug = false;
	    bool is_stopped = false;

	    bool getFlag(int bit)
	    {
		return testbit(reg_af.lo(), bit);
	    }

	    void setFlag(int bit, bool is_set)
	    {
		reg_af.lo() = changebit(reg_af.lo(), bit, is_set);
	    }

	    bool isZero()
	    {
		return getFlag(7);
	    }

	    void setZero(bool is_set)
	    {
		setFlag(7, is_set);
	    }

	    bool isSubtract()
	    {
		return getFlag(6);
	    }

	    void setSubtract(bool is_set)
	    {
		setFlag(6, is_set);
	    }

	    bool isHalf()
	    {
		return getFlag(5);
	    }

	    void setHalf(bool is_set)
	    {
		setFlag(5, is_set);
	    }

	    bool isCarry()
	    {
		return getFlag(4);
	    }

	    void setCarry(bool is_set)
	    {
		setFlag(4, is_set);
	    }
    };
};

#endif // LIBMBGB_CPU_H