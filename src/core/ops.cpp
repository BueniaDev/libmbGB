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
		// NOP
		case 0x00: m_cycles += 4; break;		

		// 8-bit loads

		// LD nn, n	
		case 0x06: bc.hi = mem->readByte(pc + 1); m_cycles += 8; break;
		case 0x0E: bc.lo = mem->readByte(pc + 1); m_cycles += 8; break;
		case 0x16: de.hi = mem->readByte(pc + 1); m_cycles += 8; break;
		case 0x1E: de.lo = mem->readByte(pc + 1); m_cycles += 8; break;
		case 0x26: hl.hi = mem->readByte(pc + 1); m_cycles += 8; break;
		case 0x2E: hl.lo = mem->readByte(pc + 1); m_cycles += 8; break;

		// LD r1, r2
		case 0x40: bc.hi = bc.hi; m_cycles += 4; break;
		case 0x41: bc.hi = bc.lo; m_cycles += 4; break;
		case 0x42: bc.hi = de.hi; m_cycles += 4; break;
		case 0x43: bc.hi = de.lo; m_cycles += 4; break;
		case 0x44: bc.hi = hl.hi; m_cycles += 4; break;
		case 0x45: bc.hi = hl.lo; m_cycles += 4; break;
		case 0x46: bc.hi = mem->readByte(hl.reg); m_cycles += 8; break;
		case 0x48: bc.lo = bc.hi; m_cycles += 4; break;
		case 0x49: bc.lo = bc.lo; m_cycles += 4; break;
		case 0x4A: bc.lo = de.hi; m_cycles += 4; break;
		case 0x4B: bc.lo = de.lo; m_cycles += 4; break;
		case 0x4C: bc.lo = hl.hi; m_cycles += 4; break;
		case 0x4D: bc.lo = hl.lo; m_cycles += 4; break;
		case 0x4E: bc.lo= mem->readByte(hl.reg); m_cycles += 8; break;
		case 0x50: de.hi = bc.hi; m_cycles += 4; break;
		case 0x51: de.hi = bc.lo; m_cycles += 4; break;
		case 0x52: de.hi = de.hi; m_cycles += 4; break;
		case 0x53: de.hi = de.lo; m_cycles += 4; break;
		case 0x54: de.hi = hl.hi; m_cycles += 4; break;
		case 0x55: de.hi = hl.lo; m_cycles += 4; break;
		case 0x56: de.hi = mem->readByte(hl.reg); m_cycles += 8; break;
		case 0x58: de.lo = bc.hi; m_cycles += 4; break;
		case 0x59: de.lo = bc.lo; m_cycles += 4; break;
		case 0x5A: de.lo = de.hi; m_cycles += 4; break;
		case 0x5B: de.lo = de.lo; m_cycles += 4; break;
		case 0x5C: de.lo = hl.hi; m_cycles += 4; break;
		case 0x5D: de.lo = hl.lo; m_cycles += 4; break;
		case 0x5E: de.lo = mem->readByte(hl.reg); m_cycles += 8; break;
		case 0x60: hl.hi = bc.hi; m_cycles += 4; break;
		case 0x61: hl.hi = bc.lo; m_cycles += 4; break;
		case 0x62: hl.hi = de.hi; m_cycles += 4; break;
		case 0x63: hl.hi = de.lo; m_cycles += 4; break;
		case 0x64: hl.hi = hl.hi; m_cycles += 4; break;
		case 0x65: hl.hi = hl.lo; m_cycles += 4; break;
		case 0x66: hl.hi = mem->readByte(hl.reg); m_cycles += 8; break;
		case 0x68: hl.lo = bc.hi; m_cycles += 4; break;
		case 0x69: hl.lo = bc.lo; m_cycles += 4; break;
		case 0x6A: hl.lo = de.hi; m_cycles += 4; break;
		case 0x6B: hl.lo = de.lo; m_cycles += 4; break;
		case 0x6C: hl.lo = hl.hi; m_cycles += 4; break;
		case 0x6D: hl.lo = hl.lo; m_cycles += 4; break;
		case 0x6E: hl.lo = mem->readByte(hl.reg); m_cycles += 8; break;
		case 0x70: mem->writeByte(hl.reg, bc.hi); m_cycles += 8; break;
		case 0x71: mem->writeByte(hl.reg, bc.lo); m_cycles += 8; break;
		case 0x72: mem->writeByte(hl.reg, de.hi); m_cycles += 8; break;
		case 0x73: mem->writeByte(hl.reg, de.lo); m_cycles += 8; break;
		case 0x74: mem->writeByte(hl.reg, hl.hi); m_cycles += 8; break;
		case 0x75: mem->writeByte(hl.reg, hl.lo); m_cycles += 8; break;
		case 0x36: mem->writeByte(hl.reg, mem->readByte(pc + 1)); m_cycles += 12; break;

		// LD A, n
		case 0x7F: af.hi = af.hi; m_cycles += 4; break;
		case 0x78: af.hi = bc.hi; m_cycles += 4; break;
		case 0x79: af.hi = bc.lo; m_cycles += 4; break;
		case 0x7A: af.hi = de.hi; m_cycles += 4; break;
		case 0x7B: af.hi = de.lo; m_cycles += 4; break;
		case 0x7C: af.hi = hl.hi; m_cycles += 4; break;
		case 0x7D: af.hi = hl.lo; m_cycles += 4; break;
		case 0x0A: af.hi = mem->readByte(bc.reg); m_cycles += 8; break;
		case 0x1A: af.hi = mem->readByte(de.reg); m_cycles += 8; break;
		case 0x7E: af.hi = mem->readByte(hl.reg); m_cycles += 8; break;
		case 0xFA: af.hi = mem->readByte(mem->readWord(pc + 1)); m_cycles += 16; break;
		case 0x3E: af.hi = mem->readByte(pc + 1); m_cycles += 8; break;

		// LD n, A
		case 0x47: bc.hi = af.hi; m_cycles += 4; break;
		case 0x4F: bc.lo = af.hi; m_cycles += 4; break;
		case 0x57: de.hi = af.hi; m_cycles += 4; break;
		case 0x5F: de.lo = af.hi; m_cycles += 4; break;
		case 0x67: hl.hi = af.hi; m_cycles += 4; break;
		case 0x6F: hl.lo = af.hi; m_cycles += 4; break;
		case 0x02: mem->writeByte(bc.reg, af.hi); m_cycles += 8; break;
		case 0x12: mem->writeByte(de.reg, af.hi); m_cycles += 8; break;
		case 0x77: mem->writeByte(hl.reg, af.hi); m_cycles += 8; break;
		case 0xEA: mem->writeByte(mem->readByte(mem->readWord(pc + 1)), af.hi); m_cycles += 16; break;

		// LD A, (C)
		case 0xF2: af.hi = (0xFF00 + bc.lo); m_cycles += 8; break;
		
		// LD (C), A
		case 0xE2: mem->writeByte((bc.lo + 0xFF00), af.hi), m_cycles += 8; break;

		// LDD A, (HL)
		case 0x3A: af.hi = mem->readByte(hl.reg - 1); m_cycles += 8; break;
		
		// LDD (HL), A
		case 0x32: mem->writeByte((hl.reg - 1), af.hi); m_cycles += 8; break;

		// LDI A, (HL)
		case 0x2A: af.hi = mem->readByte(hl.reg + 1); m_cycles += 8; break;

		// LDI (HL), A
		case 0x22: mem->writeByte((hl.reg + 1), af.hi); m_cycles += 8; break;

		// LDH (n), A
		case 0xE0: mem->writeByte((0xFF00 + mem->readByte(pc + 1)), af.hi); m_cycles += 12; break;

		// LDH A, (n)
		case 0xF0: af.hi = (0xFF00 + mem->readByte(pc + 1)); m_cycles += 12; break;	

		// 16-bit loads
		
		// LD n, nn
		case 0x01: load16bit(bc.reg, mem->readWord(pc + 1), 12); break;
		case 0x11: load16bit(de.reg, mem->readWord(pc + 1), 12); break;
		case 0x21: load16bit(hl.reg, mem->readWord(pc + 1), 12); break;
		case 0x31: load16bit(sp, mem->readWord(pc + 1), 12); break;

		// LD SP, HL
		case 0xF9: load16bit(sp, hl.reg, 8); break;

		// LDHL SP, n
		case 0xF8:
		{
		    int8_t n = (int8_t)mem->readByte(pc + 1);
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
		case 0x08: load16bit(mem->readWord(pc + 1), sp, 20); break;

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
		case 0xC6: add8bit(af.lo, mem->readByte(pc + 1), 8, false); break;

		// ADC A, n
		case 0x8F: add8bit(af.lo, af.lo, 4, true); break;
		case 0x88: add8bit(af.lo, bc.hi, 4, true); break;
		case 0x89: add8bit(af.lo, bc.lo, 4, true); break;
		case 0x8A: add8bit(af.lo, de.hi, 4, true); break;
		case 0x8B: add8bit(af.lo, de.lo, 4, true); break;
		case 0x8C: add8bit(af.lo, hl.hi, 4, true); break;
		case 0x8D: add8bit(af.lo, hl.lo, 4, true); break;
		case 0x8E: add8bit(af.lo, mem->readByte(hl.reg), 8, true); break;
		case 0xCE: add8bit(af.lo, mem->readByte(pc + 1), 8, true); break;

		// SUB n
		case 0x97: sub8bit(af.lo, af.lo, 4, false); break;
		case 0x90: sub8bit(af.lo, bc.hi, 4, false); break;
		case 0x91: sub8bit(af.lo, bc.lo, 4, false); break;
		case 0x92: sub8bit(af.lo, de.hi, 4, false); break;
		case 0x93: sub8bit(af.lo, de.lo, 4, false); break;
		case 0x94: sub8bit(af.lo, hl.hi, 4, false); break;
		case 0x95: sub8bit(af.lo, hl.lo, 4, false); break;
		case 0x96: sub8bit(af.lo, mem->readByte(hl.reg), 8, false); break;
		case 0xD6: sub8bit(af.lo, mem->readByte(pc + 1), 8, false); break;

		// SBC A, n
		case 0x9F: sub8bit(af.lo, af.lo, 4, true); break;
		case 0x98: sub8bit(af.lo, bc.hi, 4, true); break;
		case 0x99: sub8bit(af.lo, bc.lo, 4, true); break;
		case 0x9A: sub8bit(af.lo, de.hi, 4, true); break;
		case 0x9B: sub8bit(af.lo, de.lo, 4, true); break;
		case 0x9C: sub8bit(af.lo, hl.hi, 4, true); break;
		case 0x9D: sub8bit(af.lo, hl.lo, 4, true); break;
		case 0x9E: sub8bit(af.lo, mem->readByte(hl.reg), 8, true); break;
		case 0xDE: sub8bit(af.lo, mem->readByte(pc + 1), 8, true); break;		
		
		// AND n
		case 0xA7: and8bit(af.hi, af.hi, 4); break;
		case 0xA0: and8bit(af.hi, bc.hi, 4); break;
		case 0xA1: and8bit(af.hi, bc.lo, 4); break;
		case 0xA2: and8bit(af.hi, de.hi, 4); break;
		case 0xA3: and8bit(af.hi, de.lo, 4); break;
		case 0xA4: and8bit(af.hi, hl.hi, 4); break;
		case 0xA5: and8bit(af.hi, hl.lo, 4); break;
		case 0xA6: and8bit(af.hi, mem->readByte(hl.reg), 8); break;
		case 0xE6: and8bit(af.hi, mem->readByte(pc + 1), 8); break;

		// OR n
		case 0xB7: or8bit(af.hi, af.hi, 4); break;
		case 0xB0: or8bit(af.hi, bc.hi, 4); break;
		case 0xB1: or8bit(af.hi, bc.lo, 4); break;
		case 0xB2: or8bit(af.hi, de.hi, 4); break;
		case 0xB3: or8bit(af.hi, de.lo, 4); break;
		case 0xB4: or8bit(af.hi, hl.hi, 4); break;
		case 0xB5: or8bit(af.hi, hl.lo, 4); break;
		case 0xB6: or8bit(af.hi, mem->readByte(hl.reg), 8); break;
		case 0xF6: or8bit(af.hi, mem->readByte(pc + 1), 8); break;

		// XOR n
		case 0xAF: xor8bit(af.hi, af.hi, 4); break;
		case 0xA8: xor8bit(af.hi, bc.hi, 4); break;
		case 0xA9: xor8bit(af.hi, bc.lo, 4); break;
		case 0xAA: xor8bit(af.hi, de.hi, 4); break;
		case 0xAB: xor8bit(af.hi, de.lo, 4); break;
		case 0xAC: xor8bit(af.hi, hl.hi, 4); break;
		case 0xAD: xor8bit(af.hi, hl.lo, 4); break;
		case 0xAE: xor8bit(af.hi, mem->readByte(hl.reg), 8); break;
		case 0xEE: xor8bit(af.hi, mem->readByte(pc + 1), 8); break;

		// CP n
		case 0xBF: sub8bit(af.hi, af.hi, 4, false); break;
		case 0xB8: sub8bit(af.hi, bc.hi, 4, false); break;
		case 0xB9: sub8bit(af.hi, bc.lo, 4, false); break;
		case 0xBA: sub8bit(af.hi, de.hi, 4, false); break;
		case 0xBB: sub8bit(af.hi, de.lo, 4, false); break;
		case 0xBC: sub8bit(af.hi, hl.hi, 4, false); break;
		case 0xBD: sub8bit(af.hi, hl.lo, 4, false); break;
		case 0xBE: sub8bit(af.hi, mem->readByte(hl.reg), 8, false); break;
		case 0xFE: sub8bit(af.hi, mem->readByte(pc + 1), 8, false); break;

		// INC n
		case 0x3C: inc8bit(af.hi, 4); break;
		case 0x04: inc8bit(bc.hi, 4); break;
		case 0x0C: inc8bit(bc.lo, 4); break;
		case 0x14: inc8bit(de.hi, 4); break;
		case 0x1C: inc8bit(de.lo, 4); break;
		case 0x24: inc8bit(hl.hi, 4); break;
		case 0x2C: inc8bit(hl.lo, 4); break;
		case 0x34: inc8bit(mem->readByte(hl.reg), 12); break;

		// DEC n
		case 0x3D: dec8bit(af.hi, 4); break;
		case 0x05: dec8bit(bc.hi, 4); break;
		case 0x0D: dec8bit(bc.lo, 4); break;
		case 0x15: dec8bit(de.hi, 4); break;
		case 0x1D: dec8bit(de.lo, 4); break;
		case 0x25: dec8bit(hl.hi, 4); break;
		case 0x2D: dec8bit(hl.lo, 4); break;
		case 0x35: dec8bit(mem->readByte(hl.reg), 12); break;

		// 16-bit ALU

		// ADD HL, n
		case 0x09: add16bit(hl.reg, bc.reg, 8); break;
		case 0x19: add16bit(hl.reg, de.reg, 8); break;
		case 0x29: add16bit(hl.reg, hl.reg, 8); break;
		case 0x39: add16bit(hl.reg, sp, 8); break;

		// ADD SP, n
		case 0xE8: adds16bit(sp, mem->readsByte(pc + 1), 16); break;

		// INC nn
		case 0x03: inc16bit(bc.reg, 8); break;
		case 0x13: inc16bit(de.reg, 8); break;
		case 0x23: inc16bit(hl.reg, 8); break;
		case 0x33: inc16bit(sp, 8); break;

		// DEC nn
		case 0x0B: dec16bit(bc.reg, 8); break;
		case 0x1B: dec16bit(de.reg, 8); break;
		case 0x2B: dec16bit(hl.reg, 8); break;
		case 0x3B: dec16bit(sp, 8); break;

		// DAA
		case 0x27: daa(); break;

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
