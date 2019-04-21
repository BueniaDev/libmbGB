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
		case 0x06: bc.hi = mem->readByte(pc++); m_bytes += 2; m_cycles += 8; break;
		case 0x0E: bc.lo = mem->readByte(pc++); m_bytes += 2; m_cycles += 8; break;
		case 0x16: de.hi = mem->readByte(pc++); m_bytes += 2; m_cycles += 8; break;
		case 0x1E: de.lo = mem->readByte(pc++); m_bytes += 2; m_cycles += 8; break;
		case 0x26: hl.hi = mem->readByte(pc++); m_bytes += 2; m_cycles += 8; break;
		case 0x2E: hl.lo = mem->readByte(pc++); m_bytes += 2; m_cycles += 8; break;

		// LD r1, r2
		case 0x40: bc.hi = bc.hi; m_bytes += 1; m_cycles += 4; break;
		case 0x41: bc.hi = bc.lo; m_bytes += 1; m_cycles += 4; break;
		case 0x42: bc.hi = de.hi; m_bytes += 1; m_cycles += 4; break;
		case 0x43: bc.hi = de.lo; m_bytes += 1; m_cycles += 4; break;
		case 0x44: bc.hi = hl.hi; m_bytes += 1; m_cycles += 4; break;
		case 0x45: bc.hi = hl.lo; m_bytes += 1; m_cycles += 4; break;
		case 0x46: bc.hi = mem->readByte(hl.reg); m_bytes += 1; m_cycles += 8; break;
		case 0x48: bc.lo = bc.hi; m_bytes += 1; m_cycles += 4; break;
		case 0x49: bc.lo = bc.lo; m_bytes += 1; m_cycles += 4; break;
		case 0x4A: bc.lo = de.hi; m_bytes += 1; m_cycles += 4; break;
		case 0x4B: bc.lo = de.lo; m_bytes += 1; m_cycles += 4; break;
		case 0x4C: bc.lo = hl.hi; m_bytes += 1; m_cycles += 4; break;
		case 0x4D: bc.lo = hl.lo; m_bytes += 1; m_cycles += 4; break;
		case 0x4E: bc.lo= mem->readByte(hl.reg); m_bytes += 1; m_cycles += 8; break;
		case 0x50: de.hi = bc.hi; m_bytes += 1; m_cycles += 4; break;
		case 0x51: de.hi = bc.lo; m_bytes += 1; m_cycles += 4; break;
		case 0x52: de.hi = de.hi; m_bytes += 1; m_cycles += 4; break;
		case 0x53: de.hi = de.lo; m_bytes += 1; m_cycles += 4; break;
		case 0x54: de.hi = hl.hi; m_bytes += 1; m_cycles += 4; break;
		case 0x55: de.hi = hl.lo; m_bytes += 1; m_cycles += 4; break;
		case 0x56: de.hi = mem->readByte(hl.reg); m_bytes += 1; m_cycles += 8; break;
		case 0x58: de.lo = bc.hi; m_bytes += 1; m_cycles += 4; break;
		case 0x59: de.lo = bc.lo; m_bytes += 1; m_cycles += 4; break;
		case 0x5A: de.lo = de.hi; m_bytes += 1; m_cycles += 4; break;
		case 0x5B: de.lo = de.lo; m_bytes += 1; m_cycles += 4; break;
		case 0x5C: de.lo = hl.hi; m_bytes += 1; m_cycles += 4; break;
		case 0x5D: de.lo = hl.lo; m_bytes += 1; m_cycles += 4; break;
		case 0x5E: de.lo = mem->readByte(hl.reg); m_bytes += 1; m_cycles += 8; break;
		case 0x60: hl.hi = bc.hi; m_bytes += 1; m_cycles += 4; break;
		case 0x61: hl.hi = bc.lo; m_bytes += 1; m_cycles += 4; break;
		case 0x62: hl.hi = de.hi; m_bytes += 1; m_cycles += 4; break;
		case 0x63: hl.hi = de.lo; m_bytes += 1; m_cycles += 4; break;
		case 0x64: hl.hi = hl.hi; m_bytes += 1; m_cycles += 4; break;
		case 0x65: hl.hi = hl.lo; m_bytes += 1; m_cycles += 4; break;
		case 0x66: hl.hi = mem->readByte(hl.reg); m_bytes += 1; m_cycles += 8; break;
		case 0x68: hl.lo = bc.hi; m_bytes += 1; m_cycles += 4; break;
		case 0x69: hl.lo = bc.lo; m_bytes += 1; m_cycles += 4; break;
		case 0x6A: hl.lo = de.hi; m_bytes += 1; m_cycles += 4; break;
		case 0x6B: hl.lo = de.lo; m_bytes += 1; m_cycles += 4; break;
		case 0x6C: hl.lo = hl.hi; m_bytes += 1; m_cycles += 4; break;
		case 0x6D: hl.lo = hl.lo; m_bytes += 1; m_cycles += 4; break;
		case 0x6E: hl.lo = mem->readByte(hl.reg); m_bytes += 1; m_cycles += 8; break;
		case 0x70: mem->writeByte(hl.reg, bc.hi); m_bytes += 1; m_cycles += 8; break;
		case 0x71: mem->writeByte(hl.reg, bc.lo); m_bytes += 1; m_cycles += 8; break;
		case 0x72: mem->writeByte(hl.reg, de.hi); m_bytes += 1; m_cycles += 8; break;
		case 0x73: mem->writeByte(hl.reg, de.lo); m_bytes += 1; m_cycles += 8; break;
		case 0x74: mem->writeByte(hl.reg, hl.hi); m_bytes += 1; m_cycles += 8; break;
		case 0x75: mem->writeByte(hl.reg, hl.lo); m_bytes += 1; m_cycles += 8; break;
		case 0x36: advancecycles(6); mem->writeByte(hl.reg, mem->readByte(pc++)); m_bytes += 2; m_cycles += 6; break;

		// LD A, n
		case 0x7F: af.hi = af.hi; m_bytes += 1; m_cycles += 4; break;
		case 0x78: af.hi = bc.hi; m_bytes += 1; m_cycles += 4; break;
		case 0x79: af.hi = bc.lo; m_bytes += 1; m_cycles += 4; break;
		case 0x7A: af.hi = de.hi; m_bytes += 1; m_cycles += 4; break;
		case 0x7B: af.hi = de.lo; m_bytes += 1; m_cycles += 4; break;
		case 0x7C: af.hi = hl.hi; m_bytes += 1; m_cycles += 4; break;
		case 0x7D: af.hi = hl.lo; m_bytes += 1; m_cycles += 4; break;
		case 0x0A: af.hi = mem->readByte(bc.reg); m_bytes += 1; m_cycles += 8; break;
		case 0x1A: af.hi = mem->readByte(de.reg); m_bytes += 1; m_cycles += 8; break;
		case 0x7E: af.hi = mem->readByte(hl.reg); m_bytes += 1; m_cycles += 8; break;
		case 0xFA: advancecycles(8); af.hi = mem->readByte(mem->readWord(pc)); pc += 2; m_bytes += 3; m_cycles += 8; break;
		case 0x3E: af.hi = mem->readByte(pc++); m_bytes += 2; m_cycles += 8; break;

		// LD n, A
		case 0x47: bc.hi = af.hi; m_bytes += 1; m_cycles += 4; break;
		case 0x4F: bc.lo = af.hi; m_bytes += 1; m_cycles += 4; break;
		case 0x57: de.hi = af.hi; m_bytes += 1; m_cycles += 4; break;
		case 0x5F: de.lo = af.hi; m_bytes += 1; m_cycles += 4; break;
		case 0x67: hl.hi = af.hi; m_bytes += 1; m_cycles += 4; break;
		case 0x6F: hl.lo = af.hi; m_bytes += 1; m_cycles += 4; break;
		case 0x02: mem->writeByte(bc.reg, af.hi); m_bytes += 1; m_cycles += 8; break;
		case 0x12: mem->writeByte(de.reg, af.hi); m_bytes += 1; m_cycles += 8; break;
		case 0x77: mem->writeByte(hl.reg, af.hi); m_bytes += 1; m_cycles += 8; break;
		case 0xEA: advancecycles(8); mem->writeByte(mem->readWord(pc), af.hi); pc += 2; m_bytes += 3; m_cycles += 8; break;

		// LD A, (C)
		case 0xF2: af.hi = mem->readByte(0xFF00 | bc.lo); m_bytes += 1; m_cycles += 8; break;

		// LD (C), A
		case 0xE2: mem->writeByte((bc.lo | 0xFF00), af.hi), m_bytes += 1; m_cycles += 8; break;

		// LDD A, (HL)
		case 0x3A: af.hi = mem->readByte(hl.reg); hl.reg--; m_bytes += 1; m_cycles += 8; break;

		// LDD (HL), A
		case 0x32: mem->writeByte(hl.reg--, af.hi); m_bytes += 1; m_cycles += 8; break;

		// LDI A, (HL)
		case 0x2A: af.hi = mem->readByte(hl.reg); hl.reg++; m_bytes += 1; m_cycles += 8; break;

		// LDI (HL), A
		case 0x22: mem->writeByte(hl.reg, af.hi); hl.reg++; m_bytes += 1; m_cycles += 8; break;

		// LDH (n), A
		case 0xE0:
		{
		    advancecycles(6);
		    uint8_t temp = mem->readByte(pc++);
		    mem->writeByte(0xFF00 | temp, af.hi);
		    m_cycles += 6;
		}
		break;

		// LDH A, (n)
		case 0xF0:
		{
		    advancecycles(6);
		    uint8_t temp = mem->readByte(pc++);
		    af.hi = mem->readByte(0xFF00 | temp);
		    m_cycles += 6;
		}
		break;

		// 16-bit loads

		// LD n, nn
		case 0x01: bc.reg = mem->readWord(pc); pc += 2; m_bytes += 3; m_cycles += 12; break;
		case 0x11: de.reg = mem->readWord(pc); pc += 2; m_bytes += 3; m_cycles += 12; break;
		case 0x21: hl.reg = mem->readWord(pc); pc += 2; m_bytes += 3; m_cycles += 12; break;
		case 0x31: sp = mem->readWord(pc); pc += 2; m_bytes += 3; m_cycles += 12; break;

		// LD SP, HL
		case 0xF9: sp = hl.reg; m_bytes += 1; m_cycles += 8; break;

		// LDHL SP, n
		case 0xF8: hl.reg = adds8bit(sp, mem->readsByte(pc++)); m_bytes += 2; m_cycles += 12; break;

		// LD (nn), SP
		case 0x08: mem->writeWord(mem->readWord(pc), sp); pc += 2; m_bytes += 3; m_cycles += 20; break;

		// PUSH nn
		case 0xF5: mem->writeByte(--sp, af.hi); mem->writeByte(--sp, af.lo); m_bytes += 1; m_cycles += 16; break;
		case 0xC5: mem->writeByte(--sp, bc.hi); mem->writeByte(--sp, bc.lo); m_bytes += 1; m_cycles += 16; break;
		case 0xD5: mem->writeByte(--sp, de.hi); mem->writeByte(--sp, de.lo); m_bytes += 1; m_cycles += 16; break;
		case 0xE5: mem->writeByte(--sp, hl.hi); mem->writeByte(--sp, hl.lo); m_bytes += 1; m_cycles += 16; break;

		// POP nn
		case 0xF1: af.lo = mem->readByte(sp++) & 0xF0; af.hi = mem->readByte(sp++); m_bytes += 1; m_cycles += 12; break;
		case 0xC1: bc.lo = mem->readByte(sp++); bc.hi = mem->readByte(sp++); m_bytes += 1; m_cycles += 12; break;
		case 0xD1: de.lo = mem->readByte(sp++); de.hi = mem->readByte(sp++); m_bytes += 1; m_cycles += 12; break;
		case 0xE1: hl.lo = mem->readByte(sp++); hl.hi = mem->readByte(sp++); m_bytes += 1; m_cycles += 12; break;


		// 8-bit ALU

		// ADD A, n
		case 0x87: af.hi = add8bit(af.hi, af.hi); m_bytes += 1; m_cycles += 4; break;
		case 0x80: af.hi = add8bit(af.hi, bc.hi); m_bytes += 1; m_cycles += 4; break;
		case 0x81: af.hi = add8bit(af.hi, bc.lo); m_bytes += 1; m_cycles += 4; break;
		case 0x82: af.hi = add8bit(af.hi, de.hi); m_bytes += 1; m_cycles += 4; break;
		case 0x83: af.hi = add8bit(af.hi, de.lo); m_bytes += 1; m_cycles += 4; break;
		case 0x84: af.hi = add8bit(af.hi, hl.hi); m_bytes += 1; m_cycles += 4; break;
		case 0x85: af.hi = add8bit(af.hi, hl.lo); m_bytes += 1; m_cycles += 4; break;
		case 0x86: af.hi = add8bit(af.hi, mem->readByte(hl.reg)); m_bytes += 1; m_cycles += 8; break;
		case 0xC6: af.hi = add8bit(af.hi, mem->readByte(pc++)); m_bytes += 2; m_cycles += 8; break;

		// ADC A, n
		case 0x8F: af.hi = add8bitc(af.hi, af.hi); m_bytes += 1; m_cycles += 4; break;
		case 0x88: af.hi = add8bitc(af.hi, bc.hi); m_bytes += 1; m_cycles += 4; break;
		case 0x89: af.hi = add8bitc(af.hi, bc.lo); m_bytes += 1; m_cycles += 4; break;
		case 0x8A: af.hi = add8bitc(af.hi, de.hi); m_bytes += 1; m_cycles += 4; break;
		case 0x8B: af.hi = add8bitc(af.hi, de.lo); m_bytes += 1; m_cycles += 4; break;
		case 0x8C: af.hi = add8bitc(af.hi, hl.hi); m_bytes += 1; m_cycles += 4; break;
		case 0x8D: af.hi = add8bitc(af.hi, hl.lo); m_bytes += 1; m_cycles += 4; break;
		case 0x8E: af.hi = add8bitc(af.hi, mem->readByte(hl.reg)); m_bytes += 1; m_cycles += 8; break;
		case 0xCE: af.hi = add8bitc(af.hi, mem->readByte(pc++)); m_bytes += 2; m_cycles += 8; break;

		// SUB n
		case 0x97: af.hi = sub8bit(af.hi, af.hi); m_bytes += 1; m_cycles += 4; break;
		case 0x90: af.hi = sub8bit(af.hi, bc.hi); m_bytes += 1; m_cycles += 4; break;
		case 0x91: af.hi = sub8bit(af.hi, bc.lo); m_bytes += 1; m_cycles += 4; break;
		case 0x92: af.hi = sub8bit(af.hi, de.hi); m_bytes += 1; m_cycles += 4; break;
		case 0x93: af.hi = sub8bit(af.hi, de.lo); m_bytes += 1; m_cycles += 4; break;
		case 0x94: af.hi = sub8bit(af.hi, hl.hi); m_bytes += 1; m_cycles += 4; break;
		case 0x95: af.hi = sub8bit(af.hi, hl.lo); m_bytes += 1; m_cycles += 4; break;
		case 0x96: af.hi = sub8bit(af.hi, mem->readByte(hl.reg)); m_bytes += 1; m_cycles += 8; break;
		case 0xD6: af.hi = sub8bit(af.hi, mem->readByte(pc++)); m_bytes += 2; m_cycles += 8; break;

		// SBC A, n
		case 0x9F: af.hi = sub8bitc(af.hi, af.hi); m_bytes += 1;  m_cycles += 4; break;
		case 0x98: af.hi = sub8bitc(af.hi, bc.hi); m_bytes += 1; m_cycles += 4; break;
		case 0x99: af.hi = sub8bitc(af.hi, bc.lo); m_bytes += 1; m_cycles += 4; break;
		case 0x9A: af.hi = sub8bitc(af.hi, de.hi); m_bytes += 1; m_cycles += 4; break;
		case 0x9B: af.hi = sub8bitc(af.hi, de.lo); m_bytes += 1; m_cycles += 4; break;
		case 0x9C: af.hi = sub8bitc(af.hi, hl.hi); m_bytes += 1; m_cycles += 4; break;
		case 0x9D: af.hi = sub8bitc(af.hi, hl.lo); m_bytes += 1; m_cycles += 4; break;
		case 0x9E: af.hi = sub8bitc(af.hi, mem->readByte(hl.reg)); m_bytes += 1; m_cycles += 8; break;
		case 0xDE: af.hi = sub8bitc(af.hi, mem->readByte(pc++)); m_bytes += 2; m_cycles += 8; break;

		// AND n
		case 0xA7: af.hi = and8bit(af.hi, af.hi); m_bytes += 1; m_cycles += 4; break;
		case 0xA0: af.hi = and8bit(af.hi, bc.hi); m_bytes += 1; m_cycles += 4; break;
		case 0xA1: af.hi = and8bit(af.hi, bc.lo); m_bytes += 1; m_cycles += 4; break;
		case 0xA2: af.hi = and8bit(af.hi, de.hi); m_bytes += 1; m_cycles += 4; break;
		case 0xA3: af.hi = and8bit(af.hi, de.lo); m_bytes += 1; m_cycles += 4; break;
		case 0xA4: af.hi = and8bit(af.hi, hl.hi); m_bytes += 1; m_cycles += 4; break;
		case 0xA5: af.hi = and8bit(af.hi, hl.lo); m_bytes += 1; m_cycles += 4; break;
		case 0xA6: af.hi = and8bit(af.hi, mem->readByte(hl.reg)); m_bytes += 1; m_cycles += 8; break;
		case 0xE6: af.hi = and8bit(af.hi, mem->readByte(pc++)); m_bytes += 2; m_cycles += 8; break;

		// OR n
		case 0xB7: af.hi = or8bit(af.hi, af.hi); m_bytes += 1; m_cycles += 4; break;
		case 0xB0: af.hi = or8bit(af.hi, bc.hi); m_bytes += 1; m_cycles += 4; break;
		case 0xB1: af.hi = or8bit(af.hi, bc.lo); m_bytes += 1; m_cycles += 4; break;
		case 0xB2: af.hi = or8bit(af.hi, de.hi); m_bytes += 1; m_cycles += 4; break;
		case 0xB3: af.hi = or8bit(af.hi, de.lo); m_bytes += 1; m_cycles += 4; break;
		case 0xB4: af.hi = or8bit(af.hi, hl.hi); m_bytes += 1; m_cycles += 4; break;
		case 0xB5: af.hi = or8bit(af.hi, hl.lo); m_bytes += 1; m_cycles += 4; break;
		case 0xB6: af.hi = or8bit(af.hi, mem->readByte(hl.reg)); m_bytes += 1; m_cycles += 8; break;
		case 0xF6: af.hi = or8bit(af.hi, mem->readByte(pc++)); m_bytes += 2; m_cycles += 8; break;

		// XOR n
		case 0xAF: af.hi = xor8bit(af.hi, af.hi); m_bytes += 1; m_cycles += 4; break;
		case 0xA8: af.hi = xor8bit(af.hi, bc.hi); m_bytes += 1; m_cycles += 4; break;
		case 0xA9: af.hi = xor8bit(af.hi, bc.lo); m_bytes += 1; m_cycles += 4; break;
		case 0xAA: af.hi = xor8bit(af.hi, de.hi); m_bytes += 1; m_cycles += 4; break;
		case 0xAB: af.hi = xor8bit(af.hi, de.lo); m_bytes += 1; m_cycles += 4; break;
		case 0xAC: af.hi = xor8bit(af.hi, hl.hi); m_bytes += 1; m_cycles += 4; break;
		case 0xAD: af.hi = xor8bit(af.hi, hl.lo); m_bytes += 1; m_cycles += 4; break;
		case 0xAE: af.hi = xor8bit(af.hi, mem->readByte(hl.reg)); m_bytes += 1; m_cycles += 8; break;
		case 0xEE: af.hi = xor8bit(af.hi, mem->readByte(pc++)); m_bytes += 2; m_cycles += 8; break;

		// CP n
		case 0xBF: sub8bit(af.hi, af.hi); m_bytes += 1; m_cycles += 4; break;
		case 0xB8: sub8bit(af.hi, bc.hi); m_bytes += 1; m_cycles += 4; break;
		case 0xB9: sub8bit(af.hi, bc.lo); m_bytes += 1; m_cycles += 4; break;
		case 0xBA: sub8bit(af.hi, de.hi); m_bytes += 1; m_cycles += 4; break;
		case 0xBB: sub8bit(af.hi, de.lo); m_bytes += 1; m_cycles += 4; break;
		case 0xBC: sub8bit(af.hi, hl.hi); m_bytes += 1; m_cycles += 4; break;
		case 0xBD: sub8bit(af.hi, hl.lo); m_bytes += 1; m_cycles += 4; break;
		case 0xBE: sub8bit(af.hi, mem->readByte(hl.reg)); m_bytes += 1; m_cycles += 8; break;
		case 0xFE: sub8bit(af.hi, mem->readByte(pc++)); m_bytes += 2; m_cycles += 8; break;

		// INC n
		case 0x3C: af.hi = inc8bit(af.hi); m_bytes += 1; m_cycles += 4; break;
		case 0x04: bc.hi = inc8bit(bc.hi); m_bytes += 1; m_cycles += 4; break;
		case 0x0C: bc.lo = inc8bit(bc.lo); m_bytes += 1; m_cycles += 4; break;
		case 0x14: de.hi = inc8bit(de.hi); m_bytes += 1; m_cycles += 4; break;
		case 0x1C: de.lo = inc8bit(de.lo); m_bytes += 1; m_cycles += 4; break;
		case 0x24: hl.hi = inc8bit(hl.hi); m_bytes += 1; m_cycles += 4; break;
		case 0x2C: hl.lo = inc8bit(hl.lo); m_bytes += 1; m_cycles += 4; break;
		case 0x34: 
		{
		    uint8_t temp = mem->readByte(hl.reg);
		    advancecycles(6);
		    mem->writeByte(hl.reg, inc8bit(temp)); 
		    m_bytes += 1;
		    m_cycles += 6;
		}
		break;

		// DEC n
		case 0x3D: af.hi = dec8bit(af.hi); m_bytes += 1; m_cycles += 4; break;
		case 0x05: bc.hi = dec8bit(bc.hi); m_bytes += 1; m_cycles += 4; break;
		case 0x0D: bc.lo = dec8bit(bc.lo); m_bytes += 1; m_cycles += 4; break;
		case 0x15: de.hi = dec8bit(de.hi); m_bytes += 1; m_cycles += 4; break;
		case 0x1D: de.lo = dec8bit(de.lo); m_bytes += 1; m_cycles += 4; break;
		case 0x25: hl.hi = dec8bit(hl.hi); m_bytes += 1; m_cycles += 4; break;
		case 0x2D: hl.lo = dec8bit(hl.lo); m_bytes += 1; m_cycles += 4; break;
		case 0x35: 
		{
		    uint8_t temp = mem->readByte(hl.reg);
		    advancecycles(6);
		    mem->writeByte(hl.reg, dec8bit(temp)); 
		    m_bytes += 1;
		    m_cycles += 6;
		}
		break;


		// 16-bit ALU

		// ADD HL, n
		case 0x09: hl.reg = add16bit(hl.reg, bc.reg); m_bytes += 1; m_cycles += 8; break;
		case 0x19: hl.reg = add16bit(hl.reg, de.reg); m_bytes += 1; m_cycles += 8; break;
		case 0x29: hl.reg = add16bit(hl.reg, hl.reg); m_bytes += 1; m_cycles += 8; break;
		case 0x39: hl.reg = add16bit(hl.reg, sp); m_bytes += 1; m_cycles += 8; break;

		// ADD SP, n
		case 0xE8: sp = adds8bit(sp, mem->readsByte(pc++)); m_bytes += 2; m_cycles += 16; break;

		// INC nn
		case 0x03: bc.reg++; m_bytes += 1; m_cycles += 8; break;
		case 0x13: de.reg++; m_bytes += 1; m_cycles += 8; break;
		case 0x23: hl.reg++; m_bytes += 1; m_cycles += 8;break;
		case 0x33: sp++; m_bytes += 1; m_cycles += 8; break;

		// DEC nn
		case 0x0B: bc.reg--; m_bytes += 1; m_cycles += 8; break;
		case 0x1B: de.reg--; m_bytes += 1; m_cycles += 8; break;
		case 0x2B: hl.reg--; m_bytes += 1; m_cycles += 8; break;
		case 0x3B: sp--; m_bytes += 1; m_cycles += 8; break;


		// Misc

		// NOP
		case 0x00: m_bytes += 1; m_cycles += 4; break;

		// STOP
		case 0x10: stop(); break;

		// HALT
		case 0x76:
		{
		    halted = true;
		    skipinstruction = ((mem->memorymap[0xFF0F] & mem->memorymap[0xFFFF] & 0x1F) && (!interruptmaster)) ? true : false;
		    m_bytes += 1;
		    m_cycles += 4;
		}
		break;

		// DAA
		case 0x27: daa(); break;

		// CPL
		case 0x2F:
		{
		    af.hi = ~af.hi;
		    af.lo = BitSet(af.lo, subtract);
		    af.lo = BitSet(af.lo, half);
		    m_bytes += 1;
		    m_cycles += 4;
		}
		break;

		// SCF
		case 0x37:
		{
		    uint8_t zeroflag = BitGetVal(af.lo, zero);
		    af.lo = 0;

		    if (zeroflag == 1)
		    {
			af.lo = BitSet(af.lo, zero);
		    }

		    af.lo = BitSet(af.lo, carry);

		    m_bytes += 1;
		    m_cycles += 4;
		}
		break;

		// CCF
		case 0x3F:
		{
		    uint8_t zeroflag = BitGetVal(af.lo, zero);
		    uint8_t carryflag = BitGetVal(af.lo, carry);
		    af.lo = 0;

		    if (zeroflag == 1)
		    {
			af.lo = BitSet(af.lo, zero);
		    }

		    if (carryflag == 0)
		    {
			af.lo = BitSet(af.lo, carry);
		    }
		    else
		    {
			af.lo = BitReset(af.lo, carry);
		    }

		    m_bytes += 1;
		    m_cycles += 4;
		}
		break;

		// DI
		case 0xF3: interruptmaster = false; m_bytes += 1; m_cycles += 4; break;

		// EI
		case 0xFB: interruptdelay = true; m_bytes += 1; m_cycles += 4; break;


		// Rotates & shifts

		// RLCA
		case 0x07: af.hi = rlc(af.hi); af.lo = BitReset(af.lo, zero); m_bytes += 1; m_cycles += 4; break;

		// RLA
		case 0x17: af.hi = rl(af.hi); af.lo = BitReset(af.lo, zero); m_bytes += 1; m_cycles += 4; break;

		// RRCA
		case 0x0F: af.hi = rrc(af.hi); af.lo = BitReset(af.lo, zero); m_bytes += 1; m_cycles += 4; break;

		// RRA
		case 0x1F: af.hi = rr(af.hi); af.lo = BitReset(af.lo, zero); m_bytes += 1; m_cycles += 4; break;


		// Jumps

		// JP nn
		case 0xC3: pc = mem->readWord(pc); m_bytes += 1; m_cycles += 16; break;

		// JP cc, nn
		case 0xC2:
		{
		    if (!TestBit(af.lo, zero))
		    {
			pc = mem->readWord(pc);
			m_cycles += 16;
		    }
		    else
		    {
			pc += 2;
			m_cycles += 12;
		    }

		    m_bytes += 3;
		}
		break;

		case 0xCA:
		{
		    if (TestBit(af.lo, zero))
		    {
			pc = mem->readWord(pc);
			m_cycles += 16;
		    }
		    else
		    {
			pc += 2;
			m_cycles += 12;
		    }
		    m_bytes += 3;
		}
		break;

		case 0xD2:
		{
		    if (!TestBit(af.lo, carry))
		    {
			pc = mem->readWord(pc);
			m_cycles += 16;
		    }
		    else
		    {
			pc += 2;
			m_cycles += 12;
		    }
		    m_bytes += 3;
		}
		break;

		case 0xDA:
		{
		    if (TestBit(af.lo, carry))
		    {
			pc = mem->readWord(pc);
			m_cycles += 16;
		    }
		    else
		    {
			pc += 2;
			m_cycles += 12;
		    }
		    m_bytes += 3;
		}
		break;

		// JP (HL)
		case 0xE9: pc = hl.reg; m_bytes += 1; m_cycles += 4; break;

		// JR n
		case 0x18: jr(mem->readByte(pc++)); m_bytes += 2; m_cycles += 12; break;

		// JR cc, n
		case 0x20:
		{
		    if (!TestBit(af.lo, zero))
		    {
			jr(mem->readByte(pc));
			m_cycles += 12;
		    }
		    else
		    {
			m_cycles += 8;
		    }

		    pc++;
		    m_bytes += 2;
		}
		break;

		case 0x28:
		{
		    if (TestBit(af.lo, zero))
		    {
			jr(mem->readByte(pc));
			m_cycles += 12;
		    }
		    else
		    {
			m_cycles += 8;
		    }

		    pc++;
		    m_bytes += 2;
		}
		break;

		case 0x30:
		{
		    if (!TestBit(af.lo, carry))
		    {
			jr(mem->readByte(pc));
			m_cycles += 12;
		    }
		    else
		    {
			m_cycles += 8;
		    }

		    pc++;
		    m_bytes += 2;
		}
		break;

		case 0x38:
		{
		    if (TestBit(af.lo, carry))
		    {
			jr(mem->readByte(pc));
			m_cycles += 12;
		    }
		    else
		    {
			m_cycles += 8;
		    }

		    pc++;
		    m_bytes += 2;
		}
		break;

		// Calls

		// CALL nn
		case 0xCD:
		{
		    sp -= 2;
		    mem->writeWord(sp, pc + 2);
		    pc = mem->readWord(pc);
		    m_cycles += 24;
		    m_bytes += 3;
		}
		break;

		case 0xC4:
		{
		    if (!TestBit(af.lo, zero))
		    {
			sp -= 2;
		    	mem->writeWord(sp, pc + 2);
		    	pc = mem->readWord(pc);
		    	m_cycles += 24;
		    }
		    else
		    {
			pc += 2;
			m_cycles += 12;
		    }

		    m_bytes += 3;
		}
		break;

		case 0xCC:
		{
		    if (TestBit(af.lo, zero))
		    {
			sp -= 2;
		    	mem->writeWord(sp, pc + 2);
		    	pc = mem->readWord(pc);
		    	m_cycles += 24;
		    }
		    else
		    {
			pc += 2;
			m_cycles += 12;
		    }

		    m_bytes += 3;
		}
		break;

		case 0xD4:
		{
		    if (!TestBit(af.lo, carry))
		    {
			sp -= 2;
		    	mem->writeWord(sp, pc + 2);
		    	pc = mem->readWord(pc);
		    	m_cycles += 24;
		    }
		    else
		    {
			pc += 2;
			m_cycles += 12;
		    }

		    m_bytes += 3;
		}
		break;

		case 0xDC:
		{
		    if (TestBit(af.lo, carry))
		    {
			sp -= 2;
		    	mem->writeWord(sp, pc + 2);
		    	pc = mem->readWord(pc);
		    	m_cycles += 24;
		    }
		    else
		    {
			pc += 2;
			m_cycles += 12;
		    }

		    m_bytes += 3;
		}
		break;


		// Restarts

		// RST n
		case 0xC7: sp -= 2; mem->writeWord(sp, pc); pc = 0x00; m_bytes += 1; m_cycles += 16; break;
		case 0xCF: sp -= 2; mem->writeWord(sp, pc); pc = 0x08; m_bytes += 1; m_cycles += 16; break;
		case 0xD7: sp -= 2; mem->writeWord(sp, pc); pc = 0x10; m_bytes += 1; m_cycles += 16; break;
		case 0xDF: sp -= 2; mem->writeWord(sp, pc); pc = 0x18; m_bytes += 1; m_cycles += 16; break;
		case 0xE7: sp -= 2; mem->writeWord(sp, pc); pc = 0x20; m_bytes += 1; m_cycles += 16; break;
		case 0xEF: sp -= 2; mem->writeWord(sp, pc); pc = 0x28; m_bytes += 1; m_cycles += 16; break;
		case 0xF7: sp -= 2; mem->writeWord(sp, pc); pc = 0x30; m_bytes += 1; m_cycles += 16; break;
		case 0xFF: sp -= 2; mem->writeWord(sp, pc); pc = 0x38; m_bytes += 1; m_cycles += 16; break;

		// Returns

		// RET
		case 0xC9: pc = mem->readWord(sp); sp += 2; m_bytes += 1; m_cycles += 16; break;

		// RET cc
		case 0xC0:
		{
		    if (!TestBit(af.lo, zero))
		    {
			pc = mem->readWord(sp);
			sp += 2;
			m_cycles += 20;
		    }
		    else
		    {
			m_cycles += 8;
		    }

		    m_bytes += 1;
		}
		break;

		case 0xC8:
		{
		    if (TestBit(af.lo, zero))
		    {
			pc = mem->readWord(sp);
			sp += 2;
			m_cycles += 20;
		    }
		    else
		    {
			m_cycles += 8;
		    }

		    m_bytes += 1;
		}
		break;

		case 0xD0:
		{
		    if (!TestBit(af.lo, carry))
		    {
			pc = mem->readWord(sp);
			sp += 2;
			m_cycles += 20;
		    }
		    else
		    {
			m_cycles += 8;
		    }

		    m_bytes += 1;
		}
		break;

		case 0xD8:
		{
		    if (TestBit(af.lo, carry))
		    {
			pc = mem->readWord(sp);
			sp += 2;
			m_cycles += 20;
		    }
		    else
		    {
			m_cycles += 8;
		    }

		    m_bytes += 1;
		}
		break;

		// RETI
		case 0xD9:
		{
		    pc = mem->readWord(sp); 
		    sp += 2; 
		    interruptmaster = true;
		    m_bytes += 1;
		    m_cycles += 16;
		}
		break;

		// Extended ops
		case 0xCB: executecbopcode(); break;

		default: cout << "Unrecognized opcode at 0x" << hex << (int) opcode << endl; printvalues(); exit(1); break;
	    }
	}

	void CPU::executecbopcode()
	{
	    uint8_t opcode = mem->readByte(pc++);

	    switch (opcode)
	    {
		// Misc

		// SWAP	n

		case 0x37: af.hi = swap(af.hi); m_bytes += 2; m_cycles += 8; break;
		case 0x30: bc.hi = swap(bc.hi); m_bytes += 2; m_cycles += 8; break;
		case 0x31: bc.lo = swap(bc.lo); m_bytes += 2; m_cycles += 8; break;
		case 0x32: de.hi = swap(de.hi); m_bytes += 2; m_cycles += 8; break;
		case 0x33: de.lo = swap(de.lo); m_bytes += 2; m_cycles += 8; break;
		case 0x34: hl.hi = swap(hl.hi); m_bytes += 2; m_cycles += 8; break;
		case 0x35: hl.lo = swap(hl.lo); m_bytes += 2; m_cycles += 8; break;
		case 0x36:
		{
		    advancecycles(6);
		    uint8_t temp = mem->readByte(hl.reg);
		    advancecycles(5);
		    mem->writeByte(hl.reg, swap(temp));
		    m_bytes += 2; 
		    m_cycles += 5;
		}
		break;


		// Rotates and shifts

		// RLC n
		case 0x07: af.hi = rlc(af.hi); m_bytes += 2; m_cycles += 8; break;
		case 0x00: bc.hi = rlc(bc.hi); m_bytes += 2; m_cycles += 8; break;
		case 0x01: bc.lo = rlc(bc.lo); m_bytes += 2; m_cycles += 8; break;
		case 0x02: de.hi = rlc(de.hi); m_bytes += 2; m_cycles += 8; break;
		case 0x03: de.lo = rlc(de.lo); m_bytes += 2; m_cycles += 8; break;
		case 0x04: hl.hi = rlc(hl.hi); m_bytes += 2; m_cycles += 8; break;
		case 0x05: hl.lo = rlc(hl.lo); m_bytes += 2; m_cycles += 8; break;
		case 0x06:
		{
		    advancecycles(6);
		    uint8_t temp = mem->readByte(hl.reg);
		    advancecycles(5);
		    mem->writeByte(hl.reg, rlc(temp));
		    m_bytes += 2; 
		    m_cycles += 5;
		}
		break;

		// RL n
		case 0x17: af.hi = rl(af.hi); m_bytes += 2; m_cycles += 8; break;
		case 0x10: bc.hi = rl(bc.hi); m_bytes += 2; m_cycles += 8; break;
		case 0x11: bc.lo = rl(bc.lo); m_bytes += 2; m_cycles += 8; break;
		case 0x12: de.hi = rl(de.hi); m_bytes += 2; m_cycles += 8; break;
		case 0x13: de.lo = rl(de.lo); m_bytes += 2; m_cycles += 8; break;
		case 0x14: hl.hi = rl(hl.hi); m_bytes += 2; m_cycles += 8; break;
		case 0x15: hl.lo = rl(hl.lo); m_bytes += 2; m_cycles += 8; break;
		case 0x16:
		{
		    advancecycles(6);
		    uint8_t temp = mem->readByte(hl.reg);
		    advancecycles(5);
		    mem->writeByte(hl.reg, rl(temp));
		    m_bytes += 2; 
		    m_cycles += 5;
		}
		break;

		// RRC n
		case 0x0F: af.hi = rrc(af.hi); m_bytes += 2; m_cycles += 8; break;
		case 0x08: bc.hi = rrc(bc.hi); m_bytes += 2; m_cycles += 8; break;
		case 0x09: bc.lo = rrc(bc.lo); m_bytes += 2; m_cycles += 8; break;
		case 0x0A: de.hi = rrc(de.hi); m_bytes += 2; m_cycles += 8; break;
		case 0x0B: de.lo = rrc(de.lo); m_bytes += 2; m_cycles += 8; break;
		case 0x0C: hl.hi = rrc(hl.hi); m_bytes += 2; m_cycles += 8; break;
		case 0x0D: hl.lo = rrc(hl.lo); m_bytes += 2; m_cycles += 8; break;
		case 0x0E:
		{
		    advancecycles(6);
		    uint8_t temp = mem->readByte(hl.reg);
		    advancecycles(5);
		    mem->writeByte(hl.reg, rrc(temp));
		    m_bytes += 2; 
		    m_cycles += 5;
		}
		break;

		// RR n
		case 0x1F: af.hi = rr(af.hi); m_bytes += 2; m_cycles += 8; break;
		case 0x18: bc.hi = rr(bc.hi); m_bytes += 2; m_cycles += 8; break;
		case 0x19: bc.lo = rr(bc.lo); m_bytes += 2; m_cycles += 8; break;
		case 0x1A: de.hi = rr(de.hi); m_bytes += 2; m_cycles += 8; break;
		case 0x1B: de.lo = rr(de.lo); m_bytes += 2; m_cycles += 8; break;
		case 0x1C: hl.hi = rr(hl.hi); m_bytes += 2; m_cycles += 8; break;
		case 0x1D: hl.lo = rr(hl.lo); m_bytes += 2; m_cycles += 8; break;
		case 0x1E:
		{
		    advancecycles(6);
		    uint8_t temp = mem->readByte(hl.reg);
		    advancecycles(5);
		    mem->writeByte(hl.reg, rr(temp));
		    m_bytes += 2; 
		    m_cycles += 5;
		}
		break;

		// SLA n
		case 0x27: af.hi = sla(af.hi); m_bytes += 2; m_cycles += 8; break;
		case 0x20: bc.hi = sla(bc.hi); m_bytes += 2; m_cycles += 8; break;
		case 0x21: bc.lo = sla(bc.lo); m_bytes += 2; m_cycles += 8; break;
		case 0x22: de.hi = sla(de.hi); m_bytes += 2; m_cycles += 8; break;
		case 0x23: de.lo = sla(de.lo); m_bytes += 2; m_cycles += 8; break;
		case 0x24: hl.hi = sla(hl.hi); m_bytes += 2; m_cycles += 8; break;
		case 0x25: hl.lo = sla(hl.lo); m_bytes += 2; m_cycles += 8; break;
		case 0x26:
		{
		    advancecycles(6);
		    uint8_t temp = mem->readByte(hl.reg);
		    advancecycles(5);
		    mem->writeByte(hl.reg, sla(temp));
		    m_bytes += 2; 
		    m_cycles += 5;
		}
		break;

		// SRA n
		case 0x2F: af.hi = sra(af.hi); m_bytes += 2; m_cycles += 8; break;
		case 0x28: bc.hi = sra(bc.hi); m_bytes += 2; m_cycles += 8; break;
		case 0x29: bc.lo = sra(bc.lo); m_bytes += 2; m_cycles += 8; break;
		case 0x2A: de.hi = sra(de.hi); m_bytes += 2; m_cycles += 8; break;
		case 0x2B: de.lo = sra(de.lo); m_bytes += 2; m_cycles += 8; break;
		case 0x2C: hl.hi = sra(hl.hi); m_bytes += 2; m_cycles += 8; break;
		case 0x2D: hl.lo = sra(hl.lo); m_bytes += 2; m_cycles += 8; break;
		case 0x2E:
		{
		    advancecycles(6);
		    uint8_t temp = mem->readByte(hl.reg);
		    advancecycles(5);
		    mem->writeByte(hl.reg, sra(temp));
		    m_bytes += 2; 
		    m_cycles += 5;
		}
		break;

		// SRL n
		case 0x3F: af.hi = srl(af.hi); m_bytes += 2; m_cycles += 8; break;
		case 0x38: bc.hi = srl(bc.hi); m_bytes += 2; m_cycles += 8; break;
		case 0x39: bc.lo = srl(bc.lo); m_bytes += 2; m_cycles += 8; break;
		case 0x3A: de.hi = srl(de.hi); m_bytes += 2; m_cycles += 8; break;
		case 0x3B: de.lo = srl(de.lo); m_bytes += 2; m_cycles += 8; break;
		case 0x3C: hl.hi = srl(hl.hi); m_bytes += 2; m_cycles += 8; break;
		case 0x3D: hl.lo = srl(hl.lo); m_bytes += 2; m_cycles += 8; break;
		case 0x3E:
		{
		    advancecycles(6);
		    uint8_t temp = mem->readByte(hl.reg);
		    advancecycles(5);
		    mem->writeByte(hl.reg, srl(temp));
		    m_bytes += 2; 
		    m_cycles += 5;
		}
		break;


		// Bit opcodes

		// BIT b, r
		case 0x47: bit(af.hi, 0); m_bytes += 2; m_cycles += 8; break;
		case 0x40: bit(bc.hi, 0); m_bytes += 2; m_cycles += 8; break;
		case 0x41: bit(bc.lo, 0); m_bytes += 2; m_cycles += 8; break;
		case 0x42: bit(de.hi, 0); m_bytes += 2; m_cycles += 8; break;
		case 0x43: bit(de.lo, 0); m_bytes += 2; m_cycles += 8; break;
		case 0x44: bit(hl.hi, 0); m_bytes += 2; m_cycles += 8; break;
		case 0x45: bit(hl.lo, 0); m_bytes += 2; m_cycles += 8; break;
		case 0x46: advancecycles(6); bit(mem->readByte(hl.reg), 0); m_bytes += 2; m_cycles += 6; break;
		case 0x4F: bit(af.hi, 1); m_bytes += 2; m_cycles += 8; break;
		case 0x48: bit(bc.hi, 1); m_bytes += 2; m_cycles += 8; break;
		case 0x49: bit(bc.lo, 1); m_bytes += 2; m_cycles += 8; break;
		case 0x4A: bit(de.hi, 1); m_bytes += 2; m_cycles += 8; break;
		case 0x4B: bit(de.lo, 1); m_bytes += 2; m_cycles += 8; break;
		case 0x4C: bit(hl.hi, 1); m_bytes += 2; m_cycles += 8; break;
		case 0x4D: bit(hl.lo, 1); m_bytes += 2; m_cycles += 8; break;
		case 0x4E: advancecycles(6); bit(mem->readByte(hl.reg), 1); m_bytes += 2; m_cycles += 6; break;
		case 0x57: bit(af.hi, 2); m_bytes += 2; m_cycles += 8; break;
		case 0x50: bit(bc.hi, 2); m_bytes += 2; m_cycles += 8; break;
		case 0x51: bit(bc.lo, 2); m_bytes += 2; m_cycles += 8; break;
		case 0x52: bit(de.hi, 2); m_bytes += 2; m_cycles += 8; break;
		case 0x53: bit(de.lo, 2); m_bytes += 2; m_cycles += 8; break;
		case 0x54: bit(hl.hi, 2); m_bytes += 2; m_cycles += 8; break;
		case 0x55: bit(hl.lo, 2); m_bytes += 2; m_cycles += 8; break;
		case 0x56: advancecycles(6); bit(mem->readByte(hl.reg), 2); m_bytes += 2; m_cycles += 6; break;
		case 0x5F: bit(af.hi, 3); m_bytes += 2; m_cycles += 8; break;
		case 0x58: bit(bc.hi, 3); m_bytes += 2; m_cycles += 8; break;
		case 0x59: bit(bc.lo, 3); m_bytes += 2; m_cycles += 8; break;
		case 0x5A: bit(de.hi, 3); m_bytes += 2; m_cycles += 8; break;
		case 0x5B: bit(de.lo, 3); m_bytes += 2; m_cycles += 8; break;
		case 0x5C: bit(hl.hi, 3); m_bytes += 2; m_cycles += 8; break;
		case 0x5D: bit(hl.lo, 3); m_bytes += 2; m_cycles += 8; break;
		case 0x5E: advancecycles(6); bit(mem->readByte(hl.reg), 3); m_bytes += 2; m_cycles += 6; break;
		case 0x67: bit(af.hi, 4); m_bytes += 2; m_cycles += 8; break;
		case 0x60: bit(bc.hi, 4); m_bytes += 2; m_cycles += 8; break;
		case 0x61: bit(bc.lo, 4); m_bytes += 2; m_cycles += 8; break;
		case 0x62: bit(de.hi, 4); m_bytes += 2; m_cycles += 8; break;
		case 0x63: bit(de.lo, 4); m_bytes += 2; m_cycles += 8; break;
		case 0x64: bit(hl.hi, 4); m_bytes += 2; m_cycles += 8; break;
		case 0x65: bit(hl.lo, 4); m_bytes += 2; m_cycles += 8; break;
		case 0x66: advancecycles(6); bit(mem->readByte(hl.reg), 4); m_bytes += 2; m_cycles += 6; break;
		case 0x6F: bit(af.hi, 5); m_bytes += 2; m_cycles += 8; break;
		case 0x68: bit(bc.hi, 5); m_bytes += 2; m_cycles += 8; break;
		case 0x69: bit(bc.lo, 5); m_bytes += 2; m_cycles += 8; break;
		case 0x6A: bit(de.hi, 5); m_bytes += 2; m_cycles += 8; break;
		case 0x6B: bit(de.lo, 5); m_bytes += 2; m_cycles += 8; break;
		case 0x6C: bit(hl.hi, 5); m_bytes += 2; m_cycles += 8; break;
		case 0x6D: bit(hl.lo, 5); m_bytes += 2; m_cycles += 8; break;
		case 0x6E: advancecycles(6); bit(mem->readByte(hl.reg), 5); m_bytes += 2; m_cycles += 6; break;
		case 0x77: bit(af.hi, 6); m_bytes += 2; m_cycles += 8; break;
		case 0x70: bit(bc.hi, 6); m_bytes += 2; m_cycles += 8; break;
		case 0x71: bit(bc.lo, 6); m_bytes += 2; m_cycles += 8; break;
		case 0x72: bit(de.hi, 6); m_bytes += 2; m_cycles += 8; break;
		case 0x73: bit(de.lo, 6); m_bytes += 2; m_cycles += 8; break;
		case 0x74: bit(hl.hi, 6); m_bytes += 2; m_cycles += 8; break;
		case 0x75: bit(hl.lo, 6); m_bytes += 2; m_cycles += 8; break;
		case 0x76: advancecycles(6); bit(mem->readByte(hl.reg), 6); m_bytes += 2; m_cycles += 6; break;
		case 0x7F: bit(af.hi, 7); m_bytes += 2; m_cycles += 8; break;
		case 0x78: bit(bc.hi, 7); m_bytes += 2; m_cycles += 8; break;
		case 0x79: bit(bc.lo, 7); m_bytes += 2; m_cycles += 8; break;
		case 0x7A: bit(de.hi, 7); m_bytes += 2; m_cycles += 8; break;
		case 0x7B: bit(de.lo, 7); m_bytes += 2; m_cycles += 8; break;
		case 0x7C: bit(hl.hi, 7); m_bytes += 2; m_cycles += 8; break;
		case 0x7D: bit(hl.lo, 7); m_bytes += 2; m_cycles += 8; break;
		case 0x7E: advancecycles(6); bit(mem->readByte(hl.reg), 7); m_bytes += 2; m_cycles += 6; break;

		// SET b, r
		case 0xC7: af.hi = set(af.hi, 0); m_bytes += 2; m_cycles += 8; break;
		case 0xC0: bc.hi = set(bc.hi, 0); m_bytes += 2; m_cycles += 8; break;
		case 0xC1: bc.lo = set(bc.lo, 0); m_bytes += 2; m_cycles += 8; break;
		case 0xC2: de.hi = set(de.hi, 0); m_bytes += 2; m_cycles += 8; break;
		case 0xC3: de.lo = set(de.lo, 0); m_bytes += 2; m_cycles += 8; break;
		case 0xC4: hl.hi = set(hl.hi, 0); m_bytes += 2; m_cycles += 8; break;
		case 0xC5: hl.lo = set(hl.lo, 0); m_bytes += 2; m_cycles += 8; break;
		case 0xC6:
		{
		    advancecycles(6);
		    uint8_t temp = mem->readByte(hl.reg);
		    advancecycles(5);
		    mem->writeByte(hl.reg, set(temp, 0));
		    m_bytes += 2; 
		    m_cycles += 5;
		}
		break;
		case 0xCF: af.hi = set(af.hi, 1); m_bytes += 2; m_cycles += 8; break;
		case 0xC8: bc.hi = set(bc.hi, 1); m_bytes += 2; m_cycles += 8; break;
		case 0xC9: bc.lo = set(bc.lo, 1); m_bytes += 2; m_cycles += 8; break;
		case 0xCA: de.hi = set(de.hi, 1); m_bytes += 2; m_cycles += 8; break;
		case 0xCB: de.lo = set(de.lo, 1); m_bytes += 2; m_cycles += 8; break;
		case 0xCC: hl.hi = set(hl.hi, 1); m_bytes += 2; m_cycles += 8; break;
		case 0xCD: hl.lo = set(hl.lo, 1); m_bytes += 2; m_cycles += 8; break;
		case 0xCE:
		{
		    advancecycles(6);
		    uint8_t temp = mem->readByte(hl.reg);
		    advancecycles(5);
		    mem->writeByte(hl.reg, set(temp, 1));
		    m_bytes += 2; 
		    m_cycles += 5;
		}
		break;
		case 0xD7: af.hi = set(af.hi, 2); m_bytes += 2; m_cycles += 8; break;
		case 0xD0: bc.hi = set(bc.hi, 2); m_bytes += 2; m_cycles += 8; break;
		case 0xD1: bc.lo = set(bc.lo, 2); m_bytes += 2; m_cycles += 8; break;
		case 0xD2: de.hi = set(de.hi, 2); m_bytes += 2; m_cycles += 8; break;
		case 0xD3: de.lo = set(de.lo, 2); m_bytes += 2; m_cycles += 8; break;
		case 0xD4: hl.hi = set(hl.hi, 2); m_bytes += 2; m_cycles += 8; break;
		case 0xD5: hl.lo = set(hl.lo, 2); m_bytes += 2; m_cycles += 8; break;
		case 0xD6:
		{
		    advancecycles(6);
		    uint8_t temp = mem->readByte(hl.reg);
		    advancecycles(5);
		    mem->writeByte(hl.reg, set(temp, 2));
		    m_bytes += 2; 
		    m_cycles += 5;
		}
		break;
		case 0xDF: af.hi = set(af.hi, 3); m_bytes += 2; m_cycles += 8; break;
		case 0xD8: bc.hi = set(bc.hi, 3); m_bytes += 2; m_cycles += 8; break;
		case 0xD9: bc.lo = set(bc.lo, 3); m_bytes += 2; m_cycles += 8; break;
		case 0xDA: de.hi = set(de.hi, 3); m_bytes += 2; m_cycles += 8; break;
		case 0xDB: de.lo = set(de.lo, 3); m_bytes += 2; m_cycles += 8; break;
		case 0xDC: hl.hi = set(hl.hi, 3); m_bytes += 2; m_cycles += 8; break;
		case 0xDD: hl.lo = set(hl.lo, 3); m_bytes += 2; m_cycles += 8; break;
		case 0xDE:
		{
		    advancecycles(6);
		    uint8_t temp = mem->readByte(hl.reg);
		    advancecycles(5);
		    mem->writeByte(hl.reg, set(temp, 3));
		    m_bytes += 2; 
		    m_cycles += 5;
		}
		break;
		case 0xE7: af.hi = set(af.hi, 4); m_bytes += 2; m_cycles += 8; break;
		case 0xE0: bc.hi = set(bc.hi, 4); m_bytes += 2; m_cycles += 8; break;
		case 0xE1: bc.lo = set(bc.lo, 4); m_bytes += 2; m_cycles += 8; break;
		case 0xE2: de.hi = set(de.hi, 4); m_bytes += 2; m_cycles += 8; break;
		case 0xE3: de.lo = set(de.lo, 4); m_bytes += 2; m_cycles += 8; break;
		case 0xE4: hl.hi = set(hl.hi, 4); m_bytes += 2; m_cycles += 8; break;
		case 0xE5: hl.lo = set(hl.lo, 4); m_bytes += 2; m_cycles += 8; break;
		case 0xE6:
		{
		    advancecycles(6);
		    uint8_t temp = mem->readByte(hl.reg);
		    advancecycles(5);
		    mem->writeByte(hl.reg, set(temp, 4));
		    m_bytes += 2; 
		    m_cycles += 5;
		}
		break;
		case 0xEF: af.hi = set(af.hi, 5); m_bytes += 2; m_cycles += 8; break;
		case 0xE8: bc.hi = set(bc.hi, 5); m_bytes += 2; m_cycles += 8; break;
		case 0xE9: bc.lo = set(bc.lo, 5); m_bytes += 2; m_cycles += 8; break;
		case 0xEA: de.hi = set(de.hi, 5); m_bytes += 2; m_cycles += 8; break;
		case 0xEB: de.lo = set(de.lo, 5); m_bytes += 2; m_cycles += 8; break;
		case 0xEC: hl.hi = set(hl.hi, 5); m_bytes += 2; m_cycles += 8; break;
		case 0xED: hl.lo = set(hl.lo, 5); m_bytes += 2; m_cycles += 8; break;
		case 0xEE:
		{
		    advancecycles(6);
		    uint8_t temp = mem->readByte(hl.reg);
		    advancecycles(5);
		    mem->writeByte(hl.reg, set(temp, 5));
		    m_bytes += 2; 
		    m_cycles += 5;
		}
		break;
		case 0xF7: af.hi = set(af.hi, 6); m_bytes += 2; m_cycles += 8; break;
		case 0xF0: bc.hi = set(bc.hi, 6); m_bytes += 2; m_cycles += 8; break;
		case 0xF1: bc.lo = set(bc.lo, 6); m_bytes += 2; m_cycles += 8; break;
		case 0xF2: de.hi = set(de.hi, 6); m_bytes += 2; m_cycles += 8; break;
		case 0xF3: de.lo = set(de.lo, 6); m_bytes += 2; m_cycles += 8; break;
		case 0xF4: hl.hi = set(hl.hi, 6); m_bytes += 2; m_cycles += 8; break;
		case 0xF5: hl.lo = set(hl.lo, 6); m_bytes += 2; m_cycles += 8; break;
		case 0xF6:
		{
		    advancecycles(6);
		    uint8_t temp = mem->readByte(hl.reg);
		    advancecycles(5);
		    mem->writeByte(hl.reg, set(temp, 6));
		    m_bytes += 2; 
		    m_cycles += 5;
		}
		break;
		case 0xFF: af.hi = set(af.hi, 7); m_bytes += 2; m_cycles += 8; break;
		case 0xF8: bc.hi = set(bc.hi, 7); m_bytes += 2; m_cycles += 8; break;
		case 0xF9: bc.lo = set(bc.lo, 7); m_bytes += 2; m_cycles += 8; break;
		case 0xFA: de.hi = set(de.hi, 7); m_bytes += 2; m_cycles += 8; break;
		case 0xFB: de.lo = set(de.lo, 7); m_bytes += 2; m_cycles += 8; break;
		case 0xFC: hl.hi = set(hl.hi, 7); m_bytes += 2; m_cycles += 8; break;
		case 0xFD: hl.lo = set(hl.lo, 7); m_bytes += 2; m_cycles += 8; break;
		case 0xFE: 
		{
		    advancecycles(6);
		    uint8_t temp = mem->readByte(hl.reg);
		    advancecycles(5);
		    mem->writeByte(hl.reg, set(temp, 7));
		    m_bytes += 2; 
		    m_cycles += 5;
		}
		break;

		// RES b, r
		case 0x87: af.hi = res(af.hi, 0); m_bytes += 2; m_cycles += 8; break;
		case 0x80: bc.hi = res(bc.hi, 0); m_bytes += 2; m_cycles += 8; break;
		case 0x81: bc.lo = res(bc.lo, 0); m_bytes += 2; m_cycles += 8; break;
		case 0x82: de.hi = res(de.hi, 0); m_bytes += 2; m_cycles += 8; break;
		case 0x83: de.lo = res(de.lo, 0); m_bytes += 2; m_cycles += 8; break;
		case 0x84: hl.hi = res(hl.hi, 0); m_bytes += 2; m_cycles += 8; break;
		case 0x85: hl.lo = res(hl.lo, 0); m_bytes += 2; m_cycles += 8; break;
		case 0x86:		
		{
		    advancecycles(6);
		    uint8_t temp = mem->readByte(hl.reg);
		    advancecycles(5);
		    mem->writeByte(hl.reg, res(temp, 0));
		    m_bytes += 2; 
		    m_cycles += 5;
		}
		break;
		case 0x8F: af.hi = res(af.hi, 1); m_bytes += 2; m_cycles += 8; break;
		case 0x88: bc.hi = res(bc.hi, 1); m_bytes += 2; m_cycles += 8; break;
		case 0x89: bc.lo = res(bc.lo, 1); m_bytes += 2; m_cycles += 8; break;
		case 0x8A: de.hi = res(de.hi, 1); m_bytes += 2; m_cycles += 8; break;
		case 0x8B: de.lo = res(de.lo, 1); m_bytes += 2; m_cycles += 8; break;
		case 0x8C: hl.hi = res(hl.hi, 1); m_bytes += 2; m_cycles += 8; break;
		case 0x8D: hl.lo = res(hl.lo, 1); m_bytes += 2; m_cycles += 8; break;
		case 0x8E:		
		{
		    advancecycles(6);
		    uint8_t temp = mem->readByte(hl.reg);
		    advancecycles(5);
		    mem->writeByte(hl.reg, res(temp, 1));
		    m_bytes += 2; 
		    m_cycles += 5;
		}
		break;
		case 0x97: af.hi = res(af.hi, 2); m_bytes += 2; m_cycles += 8; break;
		case 0x90: bc.hi = res(bc.hi, 2); m_bytes += 2; m_cycles += 8; break;
		case 0x91: bc.lo = res(bc.lo, 2); m_bytes += 2; m_cycles += 8; break;
		case 0x92: de.hi = res(de.hi, 2); m_bytes += 2; m_cycles += 8; break;
		case 0x93: de.lo = res(de.lo, 2); m_bytes += 2; m_cycles += 8; break;
		case 0x94: hl.hi = res(hl.hi, 2); m_bytes += 2; m_cycles += 8; break;
		case 0x95: hl.lo = res(hl.lo, 2); m_bytes += 2; m_cycles += 8; break;
		case 0x96:		
		{
		    advancecycles(6);
		    uint8_t temp = mem->readByte(hl.reg);
		    advancecycles(5);
		    mem->writeByte(hl.reg, res(temp, 2));
		    m_bytes += 2; 
		    m_cycles += 5;
		}
		break;
		case 0x9F: af.hi = res(af.hi, 3); m_bytes += 2; m_cycles += 8; break;
		case 0x98: bc.hi = res(bc.hi, 3); m_bytes += 2; m_cycles += 8; break;
		case 0x99: bc.lo = res(bc.lo, 3); m_bytes += 2; m_cycles += 8; break;
		case 0x9A: de.hi = res(de.hi, 3); m_bytes += 2; m_cycles += 8; break;
		case 0x9B: de.lo = res(de.lo, 3); m_bytes += 2; m_cycles += 8; break;
		case 0x9C: hl.hi = res(hl.hi, 3); m_bytes += 2; m_cycles += 8; break;
		case 0x9D: hl.lo = res(hl.lo, 3); m_bytes += 2; m_cycles += 8; break;
		case 0x9E:		
		{
		    advancecycles(6);
		    uint8_t temp = mem->readByte(hl.reg);
		    advancecycles(5);
		    mem->writeByte(hl.reg, res(temp, 3));
		    m_bytes += 2; 
		    m_cycles += 5;
		}
		break;
		case 0xA7: af.hi = res(af.hi, 4); m_bytes += 2; m_cycles += 8; break;
		case 0xA0: bc.hi = res(bc.hi, 4); m_bytes += 2; m_cycles += 8; break;
		case 0xA1: bc.lo = res(bc.lo, 4); m_bytes += 2; m_cycles += 8; break;
		case 0xA2: de.hi = res(de.hi, 4); m_bytes += 2; m_cycles += 8; break;
		case 0xA3: de.lo = res(de.lo, 4); m_bytes += 2; m_cycles += 8; break;
		case 0xA4: hl.hi = res(hl.hi, 4); m_bytes += 2; m_cycles += 8; break;
		case 0xA5: hl.lo = res(hl.lo, 4); m_bytes += 2; m_cycles += 8; break;
		case 0xA6:		
		{
		    advancecycles(6);
		    uint8_t temp = mem->readByte(hl.reg);
		    advancecycles(5);
		    mem->writeByte(hl.reg, res(temp, 4));
		    m_bytes += 2; 
		    m_cycles += 5;
		}
		break;
		case 0xAF: af.hi = res(af.hi, 5); m_bytes += 2; m_cycles += 8; break;
		case 0xA8: bc.hi = res(bc.hi, 5); m_bytes += 2; m_cycles += 8; break;
		case 0xA9: bc.lo = res(bc.lo, 5); m_bytes += 2; m_cycles += 8; break;
		case 0xAA: de.hi = res(de.hi, 5); m_bytes += 2; m_cycles += 8; break;
		case 0xAB: de.lo = res(de.lo, 5); m_bytes += 2; m_cycles += 8; break;
		case 0xAC: hl.hi = res(hl.hi, 5); m_bytes += 2; m_cycles += 8; break;
		case 0xAD: hl.lo = res(hl.lo, 5); m_bytes += 2; m_cycles += 8; break;
		case 0xAE:		
		{
		    advancecycles(6);
		    uint8_t temp = mem->readByte(hl.reg);
		    advancecycles(5);
		    mem->writeByte(hl.reg, res(temp, 5));
		    m_bytes += 2; 
		    m_cycles += 5;
		}
		break;
		case 0xB7: af.hi = res(af.hi, 6); m_bytes += 2; m_cycles += 8; break;
		case 0xB0: bc.hi = res(bc.hi, 6); m_bytes += 2; m_cycles += 8; break;
		case 0xB1: bc.lo = res(bc.lo, 6); m_bytes += 2; m_cycles += 8; break;
		case 0xB2: de.hi = res(de.hi, 6); m_bytes += 2; m_cycles += 8; break;
		case 0xB3: de.lo = res(de.lo, 6); m_bytes += 2; m_cycles += 8; break;
		case 0xB4: hl.hi = res(hl.hi, 6); m_bytes += 2; m_cycles += 8; break;
		case 0xB5: hl.lo = res(hl.lo, 6); m_bytes += 2; m_cycles += 8; break;
		case 0xB6:		
		{
		    advancecycles(6);
		    uint8_t temp = mem->readByte(hl.reg);
		    advancecycles(5);
		    mem->writeByte(hl.reg, res(temp, 6));
		    m_bytes += 2; 
		    m_cycles += 5;
		}
		break;
		case 0xBF: af.hi = res(af.hi, 7); m_bytes += 2; m_cycles += 8; break;
		case 0xB8: bc.hi = res(bc.hi, 7); m_bytes += 2; m_cycles += 8; break;
		case 0xB9: bc.lo = res(bc.lo, 7); m_bytes += 2; m_cycles += 8; break;
		case 0xBA: de.hi = res(de.hi, 7); m_bytes += 2; m_cycles += 8; break;
		case 0xBB: de.lo = res(de.lo, 7); m_bytes += 2; m_cycles += 8; break;
		case 0xBC: hl.hi = res(hl.hi, 7); m_bytes += 2; m_cycles += 8; break;
		case 0xBD: hl.lo = res(hl.lo, 7); m_bytes += 2; m_cycles += 8; break;
		case 0xBE:		
		{
		    advancecycles(6);
		    uint8_t temp = mem->readByte(hl.reg);
		    advancecycles(5);
		    mem->writeByte(hl.reg, res(temp, 7));
		    m_bytes += 2; 
		    m_cycles += 5;
		}
		break;

		default: cout << "Unrecognized extended opcode at 0xcb" << hex << (int) opcode << endl; printvalues(); exit(1); break;
	    }
	}
}
