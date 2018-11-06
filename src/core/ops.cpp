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

		// STOP
		case 0x10: stop(); break;
		
		// HALT
		case 0x76: halted = true;		

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
		case 0x01: bc.reg = mem->readWord(pc + 1); m_cycles += 12; break;
		case 0x11: de.reg = mem->readWord(pc + 1); m_cycles += 12; break;
		case 0x21: hl.reg = mem->readWord(pc + 1); m_cycles += 12; break;
		case 0x31: sp = mem->readWord(pc + 1); m_cycles += 12; break;

		// LD SP, HL
		case 0xF9: sp = hl.reg; m_cycles += 8; break;

		// LDHL SP, n
		case 0xF8:
		{
		    int8_t n = (int8_t)mem->readByte(pc + 1);
		    uint16_t value = (sp + n) & 0xFFFF;
		    hl.reg = value;

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
		case 0x08: mem->writeWord(mem->readWord(pc), sp); pc += 2; m_cycles += 20; break;

		// PUSH nn
		case 0xF5: mem->writeByte(sp--, af.hi); mem->writeByte(sp--, af.lo); m_cycles += 16; break;
		case 0xC5: mem->writeByte(sp--, bc.hi); mem->writeByte(sp--, bc.lo); m_cycles += 16;; break;
		case 0xD5: mem->writeByte(sp--, de.hi); mem->writeByte(sp--, de.lo); m_cycles += 16; break;
		case 0xE5: mem->writeByte(sp--, hl.hi); mem->writeByte(sp--, hl.lo); m_cycles += 16; break;

		// POP nn
		case 0xF1: af.lo = mem->readByte(sp++) & 0xF0; af.hi = mem->readByte(sp++); m_cycles += 12; break;
		case 0xC1: bc.lo = mem->readByte(sp++); bc.hi = mem->readByte(sp++); m_cycles += 12; break;
		case 0xD1: de.lo = mem->readByte(sp++); de.hi = mem->readByte(sp++); m_cycles += 12; break;
		case 0xE1: hl.lo = mem->readByte(sp++); hl.hi = mem->readByte(sp++); m_cycles += 12; break;

		// 8-bit ALU

		// ADD A, n
		case 0x87: af.hi = add8bit(af.hi, af.hi, false); m_cycles += 4; break;
		case 0x80: af.hi = add8bit(af.hi, bc.hi, false); m_cycles += 4; break;
		case 0x81: af.hi = add8bit(af.hi, bc.lo, false); m_cycles += 4; break;
		case 0x82: af.hi = add8bit(af.hi, de.hi, false); m_cycles += 4; break;
		case 0x83: af.hi = add8bit(af.hi, de.lo, false); m_cycles += 4; break;
		case 0x84: af.hi = add8bit(af.hi, hl.hi, false); m_cycles += 4; break;
		case 0x85: af.hi = add8bit(af.hi, hl.lo, false); m_cycles += 4; break;
		case 0x86: af.hi = add8bit(af.hi, mem->readByte(hl.reg), false); m_cycles += 8; break;
		case 0xC6: af.hi = add8bit(af.hi, mem->readByte(pc + 1), false); m_cycles += 8; break;

		// ADC A, n
		case 0x8F: af.hi = add8bit(af.hi, af.hi, true); m_cycles += 4; break;
		case 0x88: af.hi = add8bit(af.hi, bc.hi, true); m_cycles += 4; break;
		case 0x89: af.hi = add8bit(af.hi, bc.lo, true); m_cycles += 4; break;
		case 0x8A: af.hi = add8bit(af.hi, de.hi, true); m_cycles += 4; break;
		case 0x8B: af.hi = add8bit(af.hi, de.lo, true); m_cycles += 4; break;
		case 0x8C: af.hi = add8bit(af.hi, hl.hi, true); m_cycles += 4; break;
		case 0x8D: af.hi = add8bit(af.hi, hl.lo, true); m_cycles += 4; break;
		case 0x8E: af.hi = add8bit(af.hi, mem->readByte(hl.reg), true); m_cycles += 8; break;
		case 0xCE: af.hi = add8bit(af.hi, mem->readByte(pc + 1), true); m_cycles += 8; break;

		// SUB n
		case 0x97: af.hi = sub8bit(af.hi, af.hi, false); m_cycles += 4; break;
		case 0x90: af.hi = sub8bit(af.hi, bc.hi, false); m_cycles += 4; break;
		case 0x91: af.hi = sub8bit(af.hi, bc.lo, false); m_cycles += 4; break;
		case 0x92: af.hi = sub8bit(af.hi, de.hi, false); m_cycles += 4; break;
		case 0x93: af.hi = sub8bit(af.hi, de.lo, false); m_cycles += 4; break;
		case 0x94: af.hi = sub8bit(af.hi, hl.hi, false); m_cycles += 4; break;
		case 0x95: af.hi = sub8bit(af.hi, hl.lo, false); m_cycles += 4; break;
		case 0x96: af.hi = sub8bit(af.hi, mem->readByte(hl.reg), false); m_cycles += 8; break;
		case 0xD6: af.hi = sub8bit(af.hi, mem->readByte(pc + 1), false); m_cycles += 8; break;

		// SBC A, n
		case 0x9F: af.hi = sub8bit(af.hi, af.hi, true); m_cycles += 4; break;
		case 0x98: af.hi = sub8bit(af.hi, bc.hi, true); m_cycles += 4; break;
		case 0x99: af.hi = sub8bit(af.hi, bc.lo, true); m_cycles += 4; break;
		case 0x9A: af.hi = sub8bit(af.hi, de.hi, true); m_cycles += 4; break;
		case 0x9B: af.hi = sub8bit(af.hi, de.lo, true); m_cycles += 4; break;
		case 0x9C: af.hi = sub8bit(af.hi, hl.hi, true); m_cycles += 4; break;
		case 0x9D: af.hi = sub8bit(af.hi, hl.lo, true); m_cycles += 4; break;
		case 0x9E: af.hi = sub8bit(af.hi, mem->readByte(hl.reg), true); m_cycles += 8; break;
		case 0xDE: af.hi = sub8bit(af.hi, mem->readByte(pc + 1), true); m_cycles += 8; break;		
		
		// AND n
		case 0xA7: af.hi = and8bit(af.hi, af.hi); m_cycles += 4; break;
		case 0xA0: af.hi = and8bit(af.hi, bc.hi); m_cycles += 4; break;
		case 0xA1: af.hi = and8bit(af.hi, bc.lo); m_cycles += 4; break;
		case 0xA2: af.hi = and8bit(af.hi, de.hi); m_cycles += 4; break;
		case 0xA3: af.hi = and8bit(af.hi, de.lo); m_cycles += 4; break;
		case 0xA4: af.hi = and8bit(af.hi, hl.hi); m_cycles += 4; break;
		case 0xA5: af.hi = and8bit(af.hi, hl.lo); m_cycles += 4; break;
		case 0xA6: af.hi = and8bit(af.hi, mem->readByte(hl.reg)); m_cycles += 8; break;
		case 0xE6: af.hi = and8bit(af.hi, mem->readByte(pc + 1)); m_cycles += 8; break;

		// OR n
		case 0xB7: af.hi = or8bit(af.hi, af.hi); m_cycles += 4; break;
		case 0xB0: af.hi = or8bit(af.hi, bc.hi); m_cycles += 4; break;
		case 0xB1: af.hi = or8bit(af.hi, bc.lo); m_cycles += 4; break;
		case 0xB2: af.hi = or8bit(af.hi, de.hi); m_cycles += 4; break;
		case 0xB3: af.hi = or8bit(af.hi, de.lo); m_cycles += 4; break;
		case 0xB4: af.hi = or8bit(af.hi, hl.hi); m_cycles += 4; break;
		case 0xB5: af.hi = or8bit(af.hi, hl.lo); m_cycles += 4; break;
		case 0xB6: af.hi = or8bit(af.hi, mem->readByte(hl.reg)); m_cycles += 8; break;
		case 0xF6: af.hi = or8bit(af.hi, mem->readByte(pc + 1)); m_cycles += 8; break;

		// XOR n
		case 0xAF: af.hi = xor8bit(af.hi, af.hi); m_cycles += 4; break;
		case 0xA8: af.hi = xor8bit(af.hi, bc.hi); m_cycles += 4; break;
		case 0xA9: af.hi = xor8bit(af.hi, bc.lo); m_cycles += 4; break;
		case 0xAA: af.hi = xor8bit(af.hi, de.hi); m_cycles += 4; break;
		case 0xAB: af.hi = xor8bit(af.hi, de.lo); m_cycles += 4; break;
		case 0xAC: af.hi = xor8bit(af.hi, hl.hi); m_cycles += 4; break;
		case 0xAD: af.hi = xor8bit(af.hi, hl.lo); m_cycles += 4; break;
		case 0xAE: af.hi = xor8bit(af.hi, mem->readByte(hl.reg)); m_cycles += 8; break;
		case 0xEE: af.hi = xor8bit(af.hi, mem->readByte(pc + 1)); m_cycles += 8; break;

		// CP n
		case 0xBF: af.hi = sub8bit(af.hi, af.hi, false); m_cycles += 4; break;
		case 0xB8: af.hi = sub8bit(af.hi, bc.hi, false); m_cycles += 4; break;
		case 0xB9: af.hi = sub8bit(af.hi, bc.lo, false); m_cycles += 4; break;
		case 0xBA: af.hi = sub8bit(af.hi, de.hi, false); m_cycles += 4; break;
		case 0xBB: af.hi = sub8bit(af.hi, de.lo, false); m_cycles += 4; break;
		case 0xBC: af.hi = sub8bit(af.hi, hl.hi, false); m_cycles += 4; break;
		case 0xBD: af.hi = sub8bit(af.hi, hl.lo, false); m_cycles += 4; break;
		case 0xBE: af.hi = sub8bit(af.hi, mem->readByte(hl.reg), false); m_cycles += 8; break;
		case 0xFE: af.hi = sub8bit(af.hi, mem->readByte(pc + 1), false); m_cycles += 8; break;

		// INC n
		case 0x3C: af.hi = inc8bit(af.hi); m_cycles += 4; break;
		case 0x04: bc.hi = inc8bit(bc.hi); m_cycles += 4; break;
		case 0x0C: bc.lo = inc8bit(bc.lo); m_cycles += 4; break;
		case 0x14: de.hi = inc8bit(de.hi); m_cycles += 4; break;
		case 0x1C: de.lo = inc8bit(de.lo); m_cycles += 4; break;
		case 0x24: hl.hi = inc8bit(hl.hi); m_cycles += 4; break;
		case 0x2C: hl.lo = inc8bit(hl.lo); m_cycles += 4; break;
		case 0x34: mem->writeByte(hl.reg, inc8bit(mem->readByte(hl.reg))); m_cycles += 12; break;

		// DEC n
		case 0x3D: af.hi = dec8bit(af.hi); m_cycles += 4; break;
		case 0x05: bc.hi = dec8bit(bc.hi); m_cycles += 4; break;
		case 0x0D: bc.lo = dec8bit(bc.lo); m_cycles += 4; break;
		case 0x15: de.hi = dec8bit(de.hi); m_cycles += 4; break;
		case 0x1D: de.lo = dec8bit(de.lo); m_cycles += 4; break;
		case 0x25: hl.hi = dec8bit(hl.hi); m_cycles += 4; break;
		case 0x2D: hl.lo = dec8bit(hl.lo); m_cycles += 4; break;
		case 0x35: mem->writeByte(hl.reg, dec8bit(mem->readByte(hl.reg))); m_cycles += 12; break;

		// 16-bit ALU

		// ADD HL, n
		case 0x09: hl.reg = add16bit(hl.reg, bc.reg); m_cycles += 8; break;
		case 0x19: hl.reg = add16bit(hl.reg, de.reg); m_cycles += 8; break;
		case 0x29: hl.reg = add16bit(hl.reg, hl.reg); m_cycles += 8; break;
		case 0x39: hl.reg = add16bit(hl.reg, sp); m_cycles += 8; break;

		// ADD SP, n
		case 0xE8: sp = adds16bit(sp, mem->readsByte(pc + 1)); m_cycles += 16; break;

		// INC nn
		case 0x03: bc.reg++; m_cycles += 8; break;
		case 0x13: de.reg++; m_cycles += 8; break;
		case 0x23: hl.reg++; m_cycles += 8;break;
		case 0x33: sp++; m_cycles += 8; break;

		// DEC nn
		case 0x0B: bc.reg--; m_cycles += 8; break;
		case 0x1B: de.reg--; m_cycles += 8; break;
		case 0x2B: hl.reg--; m_cycles += 8; break;
		case 0x3B: sp--; m_cycles += 8; break;

		// DAA
		case 0x27: daa(); break;
		
		// SCF
		case 0x37:
		{
		    af.lo = 0;
		    BitReset(af.lo, subtract);
		    BitReset(af.lo, half);
		    BitSet(af.lo, carry);

		    m_cycles += 4;
		}
		break;

		// CCF
		case 0x3F:
		{
		    af.lo = 0;
		    if (TestBit(af.lo, carry))
		    {
			BitReset(af.lo, carry);
		    }
		    else
		    {
			BitSet(af.lo, carry);
		    }

		    BitReset(af.lo, subtract);
		    BitReset(af.lo, half);

		    m_cycles += 4;
		}
		break;

		// DI
		case 0xF3: interruptdis = true; m_cycles += 4; break;

		// EI
		case 0xFB: interrupten = true; m_cycles += 4; break;

		// Rotates & shifts

		// RLCA
		case 0x07: af.hi = rlc(af.hi); m_cycles += 4; break;

		// RLA
		case 0x17: af.hi = rl(af.hi); m_cycles += 4; break;

		// RRCA
		case 0x0F: af.hi = rrc(af.hi); m_cycles += 4; break;

		// RRA
		case 0x1F: af.hi = rr(af.hi); m_cycles += 4; break;

		// TODO: More opcodes

		default: cout << "Unrecognized opcode at 0x" << hex << (int) opcode << endl;
	    }
	}

	void CPU::executecbopcode(uint8_t opcode)
	{
	    switch (opcode)
	    {
		// SWAP	n	

		case 0x37: af.hi = swap(af.hi); m_cycles += 8; break;
		case 0x30: bc.hi = swap(bc.hi); m_cycles += 8; break;
		case 0x31: bc.lo = swap(bc.lo); m_cycles += 8; break;
		case 0x32: de.hi = swap(de.hi); m_cycles += 8; break;
		case 0x33: de.lo = swap(de.lo); m_cycles += 8; break;
		case 0x34: hl.hi = swap(hl.hi); m_cycles += 8; break;
		case 0x35: hl.lo = swap(hl.lo); m_cycles += 8; break;
		case 0x36: mem->writeByte(hl.reg, swap(mem->readByte(hl.reg))); m_cycles += 16; break;

		// Rotates and shifts
		
		// RLC n
		case 0x07: af.hi = rlc(af.hi); m_cycles += 8; break;
		case 0x00: bc.hi = rlc(bc.hi); m_cycles += 8; break;
		case 0x01: bc.lo = rlc(bc.lo); m_cycles += 8; break;
		case 0x02: de.hi = rlc(de.hi); m_cycles += 8; break;
		case 0x03: de.lo = rlc(de.lo); m_cycles += 8; break;
		case 0x04: hl.hi = rlc(hl.hi); m_cycles += 8; break;
		case 0x05: hl.lo = rlc(hl.lo); m_cycles += 8; break;
		case 0x06: mem->writeByte(hl.reg, rlc(mem->readByte(hl.reg))); m_cycles += 16; break;

		// RL n
		case 0x17: af.hi = rl(af.hi); m_cycles += 8; break;
		case 0x10: bc.hi = rl(bc.hi); m_cycles += 8; break;
		case 0x11: bc.lo = rl(bc.lo); m_cycles += 8; break;
		case 0x12: de.hi = rl(de.hi); m_cycles += 8; break;
		case 0x13: de.lo = rl(de.lo); m_cycles += 8; break;
		case 0x14: hl.hi = rl(hl.hi); m_cycles += 8; break;
		case 0x15: hl.lo = rl(hl.lo); m_cycles += 8; break;
		case 0x16: mem->writeByte(hl.reg, rl(mem->readByte(hl.reg))); m_cycles += 16; break;

		// RRC n
		case 0x0F: af.hi = rrc(af.hi); m_cycles += 8; break;
		case 0x08: bc.hi = rrc(bc.hi); m_cycles += 8; break;
		case 0x09: bc.lo = rrc(bc.lo); m_cycles += 8; break;
		case 0x0A: de.hi = rrc(de.hi); m_cycles += 8; break;
		case 0x0B: de.lo = rrc(de.lo); m_cycles += 8; break;
		case 0x0C: hl.hi = rrc(hl.hi); m_cycles += 8; break;
		case 0x0D: hl.lo = rrc(hl.lo); m_cycles += 8; break;
		case 0x0E: mem->writeByte(hl.reg, rrc(mem->readByte(hl.reg))); m_cycles += 16; break;

		// RR n
		case 0x1F: af.hi = rr(af.hi); m_cycles += 8; break;
		case 0x18: bc.hi = rr(bc.hi); m_cycles += 8; break;
		case 0x19: bc.lo = rr(bc.lo); m_cycles += 8; break;
		case 0x1A: de.hi = rr(de.hi); m_cycles += 8; break;
		case 0x1B: de.lo = rr(de.lo); m_cycles += 8; break;
		case 0x1C: hl.hi = rr(hl.hi); m_cycles += 8; break;
		case 0x1D: hl.lo = rr(hl.lo); m_cycles += 8; break;
		case 0x1E: mem->writeByte(hl.reg, rr(mem->readByte(hl.reg))); m_cycles += 16; break;

		// SLA n
		case 0x27: af.hi = sla(af.hi); m_cycles += 8; break;
		case 0x20: bc.hi = sla(bc.hi); m_cycles += 8; break;
		case 0x21: bc.lo = sla(bc.lo); m_cycles += 8; break;
		case 0x22: de.hi = sla(de.hi); m_cycles += 8; break;
		case 0x23: de.lo = sla(de.lo); m_cycles += 8; break;
		case 0x24: hl.hi = sla(hl.hi); m_cycles += 8; break;
		case 0x25: hl.lo = sla(hl.lo); m_cycles += 8; break;
		case 0x26: mem->writeByte(hl.reg, sla(mem->readByte(hl.reg))); m_cycles += 16; break;

		// SRA n
		case 0x2F: af.hi = sra(af.hi); m_cycles += 8; break;
		case 0x28: bc.hi = sra(bc.hi); m_cycles += 8; break;
		case 0x29: bc.lo = sra(bc.lo); m_cycles += 8; break;
		case 0x2A: de.hi = sra(de.hi); m_cycles += 8; break;
		case 0x2B: de.lo = sra(de.lo); m_cycles += 8; break;
		case 0x2C: hl.hi = sra(hl.hi); m_cycles += 8; break;
		case 0x2D: hl.lo = sra(hl.lo); m_cycles += 8; break;
		case 0x2E: mem->writeByte(hl.reg, sra(mem->readByte(hl.reg))); m_cycles += 16; break;

		// SRL n
		case 0x3F: af.hi = srl(af.hi); m_cycles += 8; break;
		case 0x38: bc.hi = srl(bc.hi); m_cycles += 8; break;
		case 0x39: bc.lo = srl(bc.lo); m_cycles += 8; break;
		case 0x3A: de.hi = srl(de.hi); m_cycles += 8; break;
		case 0x3B: de.lo = srl(de.lo); m_cycles += 8; break;
		case 0x3C: hl.hi = srl(hl.hi); m_cycles += 8; break;
		case 0x3D: hl.lo = srl(hl.lo); m_cycles += 8; break;
		case 0x3E: mem->writeByte(hl.reg, srl(mem->readByte(hl.reg))); m_cycles += 16; break;

		default: cout << "Unrecognized extended opcode at 0xCB" << hex << (int) opcode << endl;
	    }
	}
}
