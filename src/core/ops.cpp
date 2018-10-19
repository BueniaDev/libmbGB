#include "../../include/libmbGB/cpu.h"
#include "../../include/libmbGB/utils.h"
#include <iostream>
using namespace gb;
using namespace std;

namespace gb
{
	void CPU::executeopcode(uint8_t opcode)
	{
	    switch (opcode)
	    {
		// 8-bit loads

		// LD nn, n	
		case 0x06: load8bit(bc.hi, mem->readByte(pc++), 8); break;
		case 0x0E: load8bit(bc.lo, mem->readByte(pc++), 8); break;
		case 0x16: load8bit(de.hi, mem->readByte(pc++), 8); break;
		case 0x1E: load8bit(de.lo, mem->readByte(pc++), 8); break;
		case 0x26: load8bit(hl.hi, mem->readByte(pc++), 8); break;
		case 0x2E: load8bit(hl.lo, mem->readByte(pc++), 8); break;

		// LD r1, r2
		case 0x40: load8bit(bc.hi, bc.hi, 4); break;
		case 0x41: load8bit(bc.hi, bc.lo, 4); break;
		case 0x42: load8bit(bc.hi, de.hi, 4); break;
		case 0x43: load8bit(bc.hi, de.lo, 4); break;
		case 0x44: load8bit(bc.hi, hl.hi, 4); break;
		case 0x45: load8bit(bc.hi, hl.lo, 4); break;
		case 0x46: load8bit(bc.hi, mem->readByte(hl.reg), 8); break;
		case 0x48: load8bit(bc.lo, bc.hi, 4); break;
		case 0x49: load8bit(bc.lo, bc.lo, 4); break;
		case 0x4A: load8bit(bc.lo, de.hi, 4); break;
		case 0x4B: load8bit(bc.lo, de.lo, 4); break;
		case 0x4C: load8bit(bc.lo, hl.hi, 4); break;
		case 0x4D: load8bit(bc.lo, hl.lo, 4); break;
		case 0x4E: load8bit(bc.lo, mem->readByte(hl.reg), 8); break;
		case 0x50: load8bit(de.hi, bc.hi, 4); break;
		case 0x51: load8bit(de.hi, bc.lo, 4); break;
		case 0x52: load8bit(de.hi, de.hi, 4); break;
		case 0x53: load8bit(de.hi, de.lo, 4); break;
		case 0x54: load8bit(de.hi, hl.hi, 4); break;
		case 0x55: load8bit(de.hi, hl.lo, 4); break;
		case 0x56: load8bit(de.hi, mem->readByte(hl.reg), 8); break;
		case 0x58: load8bit(de.lo, bc.hi, 4); break;
		case 0x59: load8bit(de.lo, bc.lo, 4); break;
		case 0x5A: load8bit(de.lo, de.hi, 4); break;
		case 0x5B: load8bit(de.lo, de.lo, 4); break;
		case 0x5C: load8bit(de.lo, hl.hi, 4); break;
		case 0x5D: load8bit(de.lo, hl.lo, 4); break;
		case 0x5E: load8bit(de.lo, mem->readByte(hl.reg), 8); break;
		case 0x60: load8bit(hl.hi, bc.hi, 4); break;
		case 0x61: load8bit(hl.hi, bc.lo, 4); break;
		case 0x62: load8bit(hl.hi, de.hi, 4); break;
		case 0x63: load8bit(hl.hi, de.lo, 4); break;
		case 0x64: load8bit(hl.hi, hl.hi, 4); break;
		case 0x65: load8bit(hl.hi, hl.lo, 4); break;
		case 0x66: load8bit(hl.hi, mem->readByte(hl.reg), 8); break;
		case 0x68: load8bit(hl.lo, bc.hi, 4); break;
		case 0x69: load8bit(hl.lo, bc.lo, 4); break;
		case 0x6A: load8bit(hl.lo, de.hi, 4); break;
		case 0x6B: load8bit(hl.lo, de.lo, 4); break;
		case 0x6C: load8bit(hl.lo, hl.hi, 4); break;
		case 0x6D: load8bit(hl.lo, hl.lo, 4); break;
		case 0x6E: load8bit(hl.lo, mem->readByte(hl.reg), 8); break;
		case 0x70: load8bit(mem->readByte(hl.reg), bc.hi, 8); break;
		case 0x71: load8bit(mem->readByte(hl.reg), bc.lo, 8); break;
		case 0x72: load8bit(mem->readByte(hl.reg), de.hi, 8); break;
		case 0x73: load8bit(mem->readByte(hl.reg), de.lo, 8); break;
		case 0x74: load8bit(mem->readByte(hl.reg), hl.hi, 8); break;
		case 0x75: load8bit(mem->readByte(hl.reg), hl.lo, 8); break;
		case 0x36: load8bit(mem->readByte(hl.reg), mem->readByte(pc++), 12); break;

		// LD A, n
		case 0x7F: load8bit(af.hi, af.hi, 4); break;
		case 0x78: load8bit(af.hi, bc.hi, 4); break;
		case 0x79: load8bit(af.hi, bc.lo, 4); break;
		case 0x7A: load8bit(af.hi, de.hi, 4); break;
		case 0x7B: load8bit(af.hi, de.lo, 4); break;
		case 0x7C: load8bit(af.hi, hl.hi, 4); break;
		case 0x7D: load8bit(af.hi, hl.lo, 4); break;
		case 0x0A: load8bit(af.hi, mem->readByte(bc.reg), 8); break;
		case 0x1A: load8bit(af.hi, mem->readByte(de.reg), 8); break;
		case 0x7E: load8bit(af.hi, mem->readByte(hl.reg), 8); break;
		case 0xFA: load8bit(af.hi, mem->readByte(mem->readWord(pc++)), 16); break;
		case 0x3E: load8bit(af.hi, mem->readByte(pc++), 8); break;

		// LD n, A
		case 0x47: load8bit(bc.hi, af.hi, 4); break;
		case 0x4F: load8bit(bc.lo, af.hi, 4); break;
		case 0x57: load8bit(de.hi, af.hi, 4); break;
		case 0x5F: load8bit(de.lo, af.hi, 4); break;
		case 0x67: load8bit(hl.hi, af.hi, 4); break;
		case 0x6F: load8bit(hl.lo, af.hi, 4); break;
		case 0x02: load8bit(mem->readByte(bc.reg), af.hi, 8); break;
		case 0x12: load8bit(mem->readByte(de.reg), af.hi, 8); break;
		case 0x77: load8bit(mem->readByte(hl.reg), af.hi, 8); break;
		case 0xEA: load8bit(mem->readByte(mem->readWord(pc++)), af.hi, 16); break;

		// LD A, (C)
		case 0xF2: load8bit(af.hi, (0xFF00 + bc.lo), 8); break;
		
		// LD (C), A
		case 0xE2: load8bit((0xFF00 + bc.lo), af.hi, 8); break;

		// LDD A, (HL)
		case 0x3A: load8bit(af.hi, mem->readByte(hl.reg--), 8); break;
		
		// LDD (HL), A
		case 0x32: load8bit(mem->readByte(hl.reg--), af.hi, 8); break;

		// LDI A, (HL)
		case 0x2A: load8bit(af.hi, mem->readByte(hl.reg++), 8); break;

		// LDI (HL), A
		case 0x22: load8bit(mem->readByte(hl.reg++), af.hi, 8); break;

		// LDH (n), A
		case 0xE0: load8bit((0xFF00 + mem->readByte(pc++)), af.hi, 12); break;

		// LDH A, (n)
		case 0xF0: load8bit(af.hi, (0xFF00 + mem->readByte(pc++)), 12); break;	

		// 16-bit loads
		
		// LD n, nn
		case 0x01: load16bit(bc.reg, mem->readWord(pc++), 12); break;
		case 0x11: load16bit(de.reg, mem->readWord(pc++), 12); break;
		case 0x21: load16bit(hl.reg, mem->readWord(pc++), 12); break;
		case 0x31: load16bit(sp, mem->readWord(pc++), 12); break;

		// LD SP, HL
		case 0xF9: load16bit(sp, hl.reg, 8); break;

		// LDHL SP, n
		case 0xF8:
		{
		    int8_t n = (int8_t)mem->readByte(pc++);
		    uint16_t value = (sp + n) & 0xFFFF;
		    load16bit(hl.reg, value, 0);

		    af.lo = 0;

		    Reset(af.lo, Bit(zero));
		    BitReset(af.lo, subtract);

		    if ((sp & 0xF) + (n & 0xF) > 0xF)
		    {
			BitSet(af.lo, half);
		    }
		    else
		    {
			BitReset(af.lo, half);
		    }

		    if ((sp + n) > 0xFFFF)
		    {
			BitSet(af.lo, carry);
		    }
		    else
		    {
			BitReset(af.lo, carry);
		    }

		    m_cycles += 12;
		}
		break;

		// LD (nn), SP
		case 0x08: load16bit(mem->readWord(pc++), sp, 20); break;

		// PUSH nn
		case 0xF5: pushontostack(af.reg, 16); break;
		case 0xC5: pushontostack(bc.reg, 16); break;
		case 0xD5: pushontostack(de.reg, 16); break;
		case 0xE5: pushontostack(hl.reg, 16); break;

		// POP nn
		case 0xF1: popontostack(af.reg, 12); break;
		case 0xC1: popontostack(bc.reg, 12); break;
		case 0xD1: popontostack(de.reg, 12); break;
		case 0xE1: popontostack(hl.reg, 12); break;

		// 8-bit ALU

		// ADD A, n
		case 0x87: add8bit(af.lo, af.lo, 4, false); break;
		case 0x80: add8bit(af.lo, bc.hi, 4, false); break;
		case 0x81: add8bit(af.lo, bc.lo, 4, false); break;
		case 0x82: add8bit(af.lo, de.hi, 4, false); break;
		case 0x83: add8bit(af.lo, de.lo, 4, false); break;
		case 0x84: add8bit(af.lo, hl.hi, 4, false); break;
		case 0x85: add8bit(af.lo, hl.lo, 4, false); break;
		case 0x86: add8bit(af.lo, mem->readByte(hl.reg), 8, false); break;
		case 0xC6: add8bit(af.lo, mem->readByte(pc++), 8, false); break;

		// ADC A, n
		case 0x8F: add8bit(af.lo, af.lo, 4, true); break;
		case 0x88: add8bit(af.lo, bc.hi, 4, true); break;
		case 0x89: add8bit(af.lo, bc.lo, 4, true); break;
		case 0x8A: add8bit(af.lo, de.hi, 4, true); break;
		case 0x8B: add8bit(af.lo, de.lo, 4, true); break;
		case 0x8C: add8bit(af.lo, hl.hi, 4, true); break;
		case 0x8D: add8bit(af.lo, hl.lo, 4, true); break;
		case 0x8E: add8bit(af.lo, mem->readByte(hl.reg), 8, true); break;
		case 0xCE: add8bit(af.lo, mem->readByte(pc++), 8, true); break;

		// TODO: More opcodes

		default: cout << "Unrecognized opcode at 0x" << hex << (int) opcode << endl;
	    }
	}

	void CPU::executecbopcode(uint8_t opcode)
	{
	    switch (opcode)
	    {
		default: cout << "Unrecognized extended opcode at 0xCB" << hex << (int) opcode << endl;
	    }
	}
}
