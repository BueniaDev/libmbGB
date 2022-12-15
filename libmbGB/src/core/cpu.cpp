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

#include "cpu.h"
using namespace gb;

namespace gb
{
    GBCPU::GBCPU(GBMMU &mem) : memory(mem)
    {

    }

    GBCPU::~GBCPU()
    {

    }

    void GBCPU::init()
    {
	if (memory.isBIOSLoad())
	{
	    reg_af = 0x0000;
	    reg_bc = 0x0000;
	    reg_de = 0x0000;
	    reg_hl = 0x0000;
	    pc = 0;
	    sp = 0;
	}
	else
	{
	    memory.writeByte(0xFF05, 0x00);
	    memory.writeByte(0xFF06, 0x00);
	    memory.writeByte(0xFF07, 0xF8);
	    memory.writeByte(0xFF40, 0x91);
	    memory.writeByte(0xFF41, 0x05);
	    memory.writeByte(0xFF47, 0xFC);

	    if (memory.isCGB())
	    {
		reg_af = 0x1180;
		reg_bc = 0x0000;
		reg_de = 0xFF56;
		reg_hl = 0x000D;
		pc = 0x0100;
		sp = 0xFFFE;
	    }
	    else
	    {
		reg_af = 0x01B0;
		reg_bc = 0x0013;
		reg_de = 0x00D8;
		reg_hl = 0x014D;
		pc = 0x0100;
		sp = 0xFFFE;
	    }
	}
    }

    void GBCPU::shutdown()
    {
	return;
    }

    void GBCPU::runInstruction()
    {
	if (is_stopped)
	{
	    tick(4);
	    return;
	}

	if (memory.isHDMAInProgress())
	{
	    memory.updateHDMA();
	    return;
	}

	if (is_halted && !memory.isCGB())
	{
	    tick(2);
	}

	uint8_t interrupt_queue = (memory.getIE() & memory.getIF() & 0x1F);

	if (is_halted)
	{
	    int cycles = memory.isCGB() ? 4 : 2;
	    tick(cycles);
	}

	bool effective_ime = is_ime;

	if (ime_toggle)
	{
	    is_ime = !is_ime;
	    ime_toggle = false;
	}

	if (is_halted && !effective_ime && interrupt_queue)
	{
	    is_halted = false;
	}
	else if (effective_ime && interrupt_queue)
	{
	    is_halted = false;
	    tick(4);
	    tick(4);
	    tick(4);
	    writeByte(--sp, (pc >> 8));
	    interrupt_queue = memory.getIE();
	    writeByte(--sp, (pc & 0xFF));
	    interrupt_queue &= (memory.getIF() & 0x1F);

	    if (interrupt_queue)
	    {
		int irq_bit = 0;

		for (int bit = 0; bit < 5; bit++)
		{
		    if (testbit(interrupt_queue, bit))
		    {
			irq_bit = bit;
			break;
		    }
		}

		memory.clearIRQ(irq_bit);
		pc = ((irq_bit * 8) + 0x40);
	    }
	    else
	    {
		pc = 0;
	    }

	    is_ime = false;
	}
	else if (!is_halted)
	{
	    uint8_t instr = getImmByte();

	    if (is_halt_bug)
	    {
		pc -= 1;
		is_halt_bug = false;
	    }

	    executeInstruction(instr);
	}

	setStatus();
    }

    void GBCPU::debugOutput(bool print_dasm)
    {
	cout << "AF: " << hex << int(internal_status.af) << endl;
	cout << "BC: " << hex << int(internal_status.bc) << endl;
	cout << "DE: " << hex << int(internal_status.de) << endl;
	cout << "HL: " << hex << int(internal_status.hl) << endl;
	cout << "PC: " << hex << int(internal_status.pc) << endl;
	cout << "SP: " << hex << int(internal_status.sp) << endl;

	if (print_dasm)
	{
	    stringstream ss;
	    disassembleInstr(ss, internal_status.pc);
	
	    cout << "Disassembly: " << ss.str() << endl;
	}

	cout << endl;
	memory.debugOutput();
    }

    void GBCPU::tick(int cycles)
    {
	memory.tick(cycles);
    }

    uint8_t GBCPU::readByte(uint16_t addr)
    {
	tick(2);
	uint8_t data = memory.readByte(addr);
	tick(2);
	return data;
    }

    void GBCPU::writeByte(uint16_t addr, uint8_t data)
    {
	tick(2);
	memory.writeByte(addr, data);
	tick(2);
    }

    uint8_t GBCPU::getImmByte()
    {
	return readByte(pc++);
    }

    uint16_t GBCPU::getImmWord()
    {
	uint8_t low = getImmByte();
	uint8_t high = getImmByte();

	return ((high << 8) | low);
    }

    void GBCPU::executeInstruction(uint8_t instr)
    {
	int opcode_x = ((instr >> 6) & 0x3);
	int opcode_y = ((instr >> 3) & 0x7);
	int opcode_z = (instr & 0x7);
	int opcode_p = ((opcode_y >> 1) & 0x3);
	int opcode_q = testbit(opcode_y, 0);

	switch (opcode_x)
	{
	    case 0:
	    {
		switch (opcode_z)
		{
		    case 0:
		    {
			switch (opcode_y)
			{
			    case 0: break;
			    case 1: writeSP(); break;
			    case 2: stop(); break;
			    case 3: jumpRel8(); break;
			    case 4:
			    case 5:
			    case 6:
			    case 7: jumpRel8Cond(getCond(opcode_y - 4)); break;
			    default: unrecognizedInstruction(false, opcode_x, opcode_y, opcode_z); break;
			}
		    }
		    break;
		    case 1:
		    {
			if (opcode_q)
			{
			    addWord(getRP(opcode_p));
			}
			else
			{
			    setRP(opcode_p, getImmWord());
			}
		    }
		    break;
		    case 2:
		    {
			if (opcode_q)
			{
			    switch (opcode_p)
			    {
				case 0: reg_af.hi() = readByte(reg_bc); break;
				case 1: reg_af.hi() = readByte(reg_de); break;
				case 2: reg_af.hi() = readByte(reg_hl); reg_hl += 1; break;
				case 3: reg_af.hi() = readByte(reg_hl); reg_hl -= 1; break;
			    }
			}
			else
			{
			    switch (opcode_p)
			    {
				case 0: writeByte(reg_bc, reg_af.hi()); break;
				case 1: writeByte(reg_de, reg_af.hi()); break;
				case 2: writeByte(reg_hl, reg_af.hi()); reg_hl += 1; break;
				case 3: writeByte(reg_hl, reg_af.hi()); reg_hl -= 1; break;
			    }
			}
		    }
		    break;
		    case 3:
		    {
			uint16_t res = getRP(opcode_p);

			if (opcode_q)
			{
			    res = dec16(res);
			}
			else
			{
			    res = inc16(res);
			}

			setRP(opcode_p, res);
		    }
		    break;
		    case 4:
		    {
			uint8_t res = getReg(opcode_y);
			res = incByte(res);
			setReg(opcode_y, res);
		    }
		    break;
		    case 5:
		    {
			uint8_t res = getReg(opcode_y);
			res = decByte(res);
			setReg(opcode_y, res);
		    }
		    break;
		    case 6: setReg(opcode_y, getImmByte()); break;
		    case 7:
		    {
			switch (opcode_y)
			{
			    case 0: rotateLeftCarryA(); break;
			    case 1: rotateRightCarryA(); break;
			    case 2: rotateLeftA(); break;
			    case 3: rotateRightA(); break;
			    case 4: daa(); break;
			    case 5: compA(); break;
			    case 6: setCarry(); break;
			    case 7: compCarry(); break;
			    default: unrecognizedInstruction(false, opcode_x, opcode_y, opcode_z); break;
			}
		    }
		    break;
		    default: unrecognizedInstruction(false, opcode_x, opcode_y, opcode_z); break;
		}
	    }
	    break;
	    case 1:
	    {
		if ((opcode_z == 6) && (opcode_y == 6))
		{
		    halt();
		}
		else
		{
		    setReg(opcode_y, getReg(opcode_z));
		}
	    }
	    break;
	    case 2:
	    {
		uint8_t res = getReg(opcode_z);
		setALU(opcode_y, res);
	    }
	    break;
	    case 3:
	    {
		switch (opcode_z)
		{
		    case 0:
		    {
			switch (opcode_y)
			{
			    case 0:
			    case 1:
			    case 2:
			    case 3: retCond(getCond(opcode_y)); break;
			    case 4:
			    {
				uint16_t io_addr = (0xFF00 + getImmByte());
				writeByte(io_addr, reg_af.hi());
			    }
			    break;
			    case 5: addSP(); break;
			    case 6:
			    {
				uint16_t io_addr = (0xFF00 + getImmByte());
				reg_af.hi() = readByte(io_addr);
			    }
			    break;
			    case 7: ldHLSP(); break;
			    default: unrecognizedInstruction(false, opcode_x, opcode_y, opcode_z); break;
			}
		    }
		    break;
		    case 1:
		    {
			if (opcode_q)
			{
			    switch (opcode_p)
			    {
				case 0: ret(); break;
				case 1: reti(); break;
				case 2: pc = reg_hl; break;
				case 3: ldSPHL(); break;
				default: unrecognizedInstruction(false, opcode_x, opcode_y, opcode_z); break;
			    }
			}
			else
			{
			    setRP2(opcode_p, pop());
			}
		    }
		    break;
		    case 2:
		    {
			switch (opcode_y)
			{
			    case 0:
			    case 1:
			    case 2:
			    case 3: jumpCond(getCond(opcode_y)); break;
			    case 4:
			    {
				uint16_t io_addr = (0xFF00 + reg_bc.lo());
				writeByte(io_addr, reg_af.hi());
			    }
			    break;
			    case 5: writeByte(getImmWord(), reg_af.hi()); break;
			    case 6:
			    {
				uint16_t io_addr = (0xFF00 + reg_bc.lo());
				reg_af.hi() = readByte(io_addr);
			    }
			    break;
			    case 7: reg_af.hi() = readByte(getImmWord()); break;
			    default: unrecognizedInstruction(false, opcode_x, opcode_y, opcode_z); break;
			}
		    }
		    break;
		    case 3:
		    {
			switch (opcode_y)
			{
			    case 0: jump(); break;
			    case 1: extendedInstruction(getImmByte()); break;
			    case 6: disableInterrupts(); break;
			    case 7: enableInterrupts(); break;
			    default: unrecognizedInstruction(false, opcode_x, opcode_y, opcode_z); break;
			}
		    }
		    break;
		    case 4:
		    {
			switch (opcode_y)
			{
			    case 0:
			    case 1:
			    case 2:
			    case 3: callCond(getCond(opcode_y)); break;
			    default: unrecognizedInstruction(false, opcode_x, opcode_y, opcode_z); break;
			}
		    }
		    break;
		    case 5:
		    {
			if (opcode_q)
			{
			    switch (opcode_p)
			    {
				case 0: call(); break;
				default: unrecognizedInstruction(false, opcode_x, opcode_y, opcode_z); break;
			    }
			}
			else
			{
			    push(getRP2(opcode_p));
			}
		    }
		    break;
		    case 6: setALU(opcode_y, getImmByte()); break;
		    case 7: rst(opcode_y); break;
		    default: unrecognizedInstruction(false, opcode_x, opcode_y, opcode_z); break;
		}
	    }
	    break;
	    default: unrecognizedInstruction(false, opcode_x, opcode_y, opcode_z); break;
	}
    }

    void GBCPU::extendedInstruction(uint8_t instr)
    {
	int opcode_x = ((instr >> 6) & 0x3);
	int opcode_y = ((instr >> 3) & 0x7);
	int opcode_z = (instr & 0x7);

	uint8_t res = getReg(opcode_z);

	switch (opcode_x)
	{
	    case 0: res = rotReg(opcode_y, res); break;
	    case 1: bitReg(res, opcode_y); break;
	    case 2: res = resReg(res, opcode_y); break;
	    case 3: res = setReg(res, opcode_y); break;
	    default: unrecognizedInstruction(true, opcode_x, opcode_y, opcode_z); break;
	}

	if (opcode_x != 1)
	{
	    setReg(opcode_z, res);
	}
    }

    size_t GBCPU::disassembleInstr(ostream &stream, uint16_t addr)
    {
	size_t pc = addr;
	size_t prev_pc = pc;

	uint8_t instr = memory.readByte(pc++);
	uint8_t arg = memory.readByte(pc);
	uint16_t arg16 = ((memory.readByte(pc + 1) << 8) | memory.readByte(pc));

	uint16_t rel_offs = (pc + 1 + int8_t(arg));

	int opcode_x = ((instr >> 6) & 0x3);
	int opcode_y = ((instr >> 3) & 0x7);
	int opcode_z = (instr & 0x7);
	int opcode_p = ((opcode_y >> 1) & 0x3);
	int opcode_q = testbit(opcode_y, 0);

	array<string, 4> rp = {"bc", "de", "hl", "sp"};
	array<string, 4> rp2 = {"bc", "de", "hl", "af"};
	array<string, 4> cc = {"nz", "z", "nc", "c"};
	array<string, 8> reg = {"b", "c", "d", "e", "h", "l", "(hl)", "a"};
	array<string, 8> alu = {"add a,", "adc a,", "sub", "sbc", "and", "xor", "or", "cp"};

	switch (opcode_x)
	{
	    case 0:
	    {
		switch (opcode_z)
		{
		    case 0:
		    {
			switch (opcode_y)
			{
			    case 0: stream << "nop"; break;
			    case 1: stream << "ld ($" << hex << int(arg16) << "), sp"; pc += 2; break;
			    case 2: stream << "stop"; break;
			    case 3: stream << "jr $" << hex << int(rel_offs); pc += 1; break;
			    case 4:
			    case 5:
			    case 6:
			    case 7: stream << "jr " << cc.at(opcode_y - 4) << ", $" << hex << int(rel_offs); pc += 1; break;
			    default: stream << "unknown " << dec << opcode_x << "/" << dec << opcode_z << "/" << dec << opcode_y; break;
			}
		    }
		    break;
		    case 1:
		    {
			if (opcode_q)
			{
			    stream << "add hl, " << rp.at(opcode_p);
			}
			else
			{
			    stream << "ld " << rp.at(opcode_p) << ", #$" << hex << int(arg16);
			    pc += 2;
			}
		    }
		    break;
		    case 2:
		    {
			if (opcode_q)
			{
			    switch (opcode_p)
			    {
				case 0: stream << "ld a, (bc)"; break;
				case 1: stream << "ld a, (de)"; break;
				case 2: stream << "ld a, (hl+)"; break;
				case 3: stream << "ld a, (hl-)"; break;
			    }
			}
			else
			{
			    switch (opcode_p)
			    {
				case 0: stream << "ld (bc), a"; break;
				case 1: stream << "ld (de), a"; break;
				case 2: stream << "ld (hl+), a"; break;
				case 3: stream << "ld (hl-), a"; break;
			    }
			}
		    }
		    break;
		    case 3:
		    {
			if (opcode_q)
			{
			    stream << "dec " << rp.at(opcode_p);
			}
			else
			{
			    stream << "inc " << rp.at(opcode_p);
			}
		    }
		    break;
		    case 4: stream << "inc " << reg.at(opcode_y); break;
		    case 5: stream << "dec " << reg.at(opcode_y); break;
		    case 6: stream << "ld " << reg.at(opcode_y) << ", #$" << hex << int(arg); pc += 1; break;
		    case 7:
		    {
			switch (opcode_y)
			{
			    case 0: stream << "rlca"; break;
			    case 1: stream << "rrca"; break;
			    case 2: stream << "rla"; break;
			    case 3: stream << "rra"; break;
			    case 4: stream << "daa"; break;
			    case 5: stream << "cpl"; break;
			    case 6: stream << "scf"; break;
			    case 7: stream << "ccf"; break;
			    default: stream << "unknown " << dec << opcode_x << "/" << dec << opcode_z << "/" << dec << opcode_y; break;
			}
		    }
		    break;
		    default: stream << "unknown " << dec << opcode_x << "/" << dec << opcode_z << "/" << dec << opcode_y; break;
		}
	    }
	    break;
	    case 1:
	    {
		if ((opcode_z == 6) && (opcode_y == 6))
		{
		    stream << "halt";
		}
		else
		{
		    stream << "ld " << reg.at(opcode_y) << ", " << reg.at(opcode_z);
		}
	    }
	    break;
	    case 2:
	    {
		stream << alu.at(opcode_y) << " " << reg.at(opcode_z);
	    }
	    break;
	    case 3:
	    {
		switch (opcode_z)
		{
		    case 0:
		    {
			switch (opcode_y)
			{
			    case 0:
			    case 1:
			    case 2:
			    case 3: stream << "ret " << cc.at(opcode_y); break;
			    case 4: stream << "ld ($FF00 + " << hex << int(arg) << "), a"; pc += 1; break;
			    case 5: stream << "add sp, #$" << hex << int(arg); pc += 1; break;
			    case 6: stream << "ld a, ($FF00 + " << hex << int(arg) << ")"; pc += 1; break;
			    case 7: stream << "ld hl, sp + #$" << hex << int(arg); pc += 1; break;
			    default: stream << "unknown " << dec << opcode_x << "/" << dec << opcode_z << "/" << dec << opcode_y; break;
			}
		    }
		    break;
		    case 1:
		    {
			if (opcode_q)
			{
			    switch (opcode_p)
			    {
				case 0: stream << "ret"; break;
				case 1: stream << "reti"; break;
				case 2: stream << "jp hl"; break;
				case 3: stream << "ld sp, hl"; break;
				default: stream << "unknown " << dec << opcode_x << "/" << dec << opcode_z << "/" << dec << opcode_y; break;
			    }
			}
			else
			{
			    stream << "pop " << rp2.at(opcode_p);
			}
		    }
		    break;
		    case 2:
		    {
			switch (opcode_y)
			{
			    case 0:
			    case 1:
			    case 2:
			    case 3: stream << "jp " << cc.at(opcode_y) << " $" << hex << int(arg16); pc += 2; break;
			    case 4: stream << "ld ($FF00 + c), a"; break;
			    case 5: stream << "ld ($" << hex << int(arg16) << "), a"; pc += 2; break;
			    case 6: stream << "ld a, ($FF00 + c)"; break;
			    case 7: stream << "ld a, ($" << hex << int(arg16) << ")"; pc += 2; break;
			    default: stream << "unknown " << dec << opcode_x << "/" << dec << opcode_z << "/" << dec << opcode_y; break;
			}
		    }
		    break;
		    case 3:
		    {
			switch (opcode_y)
			{
			    case 0: stream << "jp $" << hex << int(arg16); pc += 2; break;
			    case 1: disassembleExtended(stream, arg); pc += 1; break;
			    case 6: stream << "di"; break;
			    case 7: stream << "ei"; break;
			    default: stream << "unknown " << dec << opcode_x << "/" << dec << opcode_z << "/" << dec << opcode_y; break;
			}
		    }
		    break;
		    case 4:
		    {
			switch (opcode_y)
			{
			    case 0:
			    case 1:
			    case 2:
			    case 3: stream << "call " << cc.at(opcode_y) << ", $" << hex << int(arg16); pc += 2; break;
			    default: stream << "unknown " << dec << opcode_x << "/" << dec << opcode_z << "/" << dec << opcode_y; break;
			}
		    }
		    break;
		    case 5:
		    {
			if (opcode_q)
			{
			    switch (opcode_p)
			    {
				case 0: stream << "call $" << hex << int(arg16); pc += 2; break;
				default: stream << "unknown " << dec << opcode_x << "/" << dec << opcode_z << "/" << dec << opcode_y; break;
			    }
			}
			else
			{
			    stream << "push " << rp2.at(opcode_p);
			}
		    }
		    break;
		    case 6: stream << alu.at(opcode_y) << " #$" << hex << int(arg); pc += 1; break;
		    case 7: stream << "rst $" << hex << int(opcode_y * 8); break;
		    default: stream << "unknown " << dec << opcode_x << "/" << dec << opcode_z << "/" << dec << opcode_y; break;
		}
	    }
	    break;
	    default: stream << "unknown " << dec << opcode_x << "/" << dec << opcode_z << "/" << dec << opcode_y; break;
	}

	return (pc - prev_pc);
    }

    void GBCPU::disassembleExtended(ostream &stream, uint8_t instr)
    {
	int opcode_x = ((instr >> 6) & 0x3);
	int opcode_y = ((instr >> 3) & 0x7);
	int opcode_z = (instr & 0x7);

	array<string, 8> reg = {"b", "c", "d", "e", "h", "l", "(hl)", "a"};
	array<string, 8> rot = {"rlc", "rrc", "rl", "rr", "sla", "sra", "swap", "srl"};

	switch (opcode_x)
	{
	    case 0: stream << rot.at(opcode_y) << " " << reg.at(opcode_z); break;
	    case 1: stream << "bit " << dec << opcode_y << ", " << reg.at(opcode_z); break;
	    case 2: stream << "res " << dec << opcode_y << ", " << reg.at(opcode_z); break;
	    case 3: stream << "set " << dec << opcode_y << ", " << reg.at(opcode_z); break;
	}
    }

    void GBCPU::unrecognizedInstruction(bool is_prefix, int op_x, int op_y, int op_z)
    {
	uint8_t opcode = ((op_x << 6) | (op_y << 3) | op_z);
	string opening = (is_prefix) ? "Unrecognized extended opcode" : "Unrecognized opcode";
	cout << opening << " of " << hex << int(opcode) << ", " << dec << op_x << "/" << dec << op_z << "/" << dec << op_y << endl;
	debugOutput(false);
	throw runtime_error("Internal error");
    }
};