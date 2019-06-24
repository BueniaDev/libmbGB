// This file is part of libmbGB.
// Copyright (C) 2019 Buenia.
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

#include "../../include/libmbGB/cpu.h"
using namespace gb;

namespace gb
{
    int CPU::executenextopcode(uint8_t opcode)
    {
	int cycles = 0;	

	switch (opcode)
	{
	    case 0x01: bc.setreg(getimmword()); cycles = 12; break;
	    case 0x02: load8intomem(bc.getreg(), af.gethi()); cycles = 8; break;
	    case 0x03: bc.setreg(increg(bc.getreg())); cycles = 8; break;
	    case 0x04: bc.sethi(incregbyte(bc.gethi())); cycles = 4; break;
	    case 0x05: bc.sethi(decregbyte(bc.gethi())); cycles = 4; break;
	    case 0x06: bc.sethi(getimmbyte()); cycles = 8; break;
	    case 0x07: af.sethi(rlc(af.gethi())); setzero(false); cycles = 4; break;
	    case 0x0A: af.sethi(store8frommem(bc.getreg())); cycles = 8; break;
	    case 0x0B: bc.setreg(decreg(bc.getreg())); cycles = 8; break;
	    case 0x0C: bc.setlo(incregbyte(bc.getlo())); cycles = 4; break;
	    case 0x0D: bc.setlo(decregbyte(bc.getlo())); cycles = 4; break;
	    case 0x0E: bc.setlo(getimmbyte()); cycles = 8; break;
	    case 0x0F: af.sethi(rrc(af.gethi())); setzero(false); cycles = 4; break;
	    case 0x11: de.setreg(getimmword()); cycles = 12; break;
	    case 0x12: load8intomem(de.getreg(), af.gethi()); cycles = 8; break;
	    case 0x13: de.setreg(increg(de.getreg())); cycles = 8; break;
	    case 0x14: de.sethi(incregbyte(de.gethi())); cycles = 4; break;
	    case 0x15: de.sethi(decregbyte(de.gethi())); cycles = 4; break;
	    case 0x16: de.sethi(getimmbyte()); cycles = 8; break;
	    case 0x17: af.sethi(rl(af.gethi())); setzero(false); cycles = 4; break;
	    case 0x18: reljump(getimmsignedbyte()); cycles = 12; break;
	    case 0x1A: af.sethi(store8frommem(de.getreg())); cycles = 8; break;
	    case 0x1B: de.setreg(decreg(de.getreg())); cycles = 8; break;
	    case 0x1C: de.setlo(incregbyte(de.getlo())); cycles = 4; break;
	    case 0x1D: de.setlo(decregbyte(de.getlo())); cycles = 4; break;
	    case 0x1E: de.setlo(getimmbyte()); cycles = 8; break;
	    case 0x1F: af.sethi(rr(af.gethi())); setzero(false); cycles = 4; break;
	    case 0x20: cycles = reljumpcond(getimmsignedbyte(), !iszero()); break;
	    case 0x21: hl.setreg(getimmword()); cycles = 12; break;
	    case 0x22: load8intomem(hl.getreg(), af.gethi()); hl.setreg(increg(hl.getreg())); cycles = 8; break;
	    case 0x23: hl.setreg(increg(hl.getreg())); cycles = 8; break;
	    case 0x24: hl.sethi(incregbyte(hl.gethi())); cycles = 4; break;
	    case 0x25: hl.sethi(decregbyte(hl.gethi())); cycles = 4; break;
	    case 0x26: hl.sethi(getimmbyte()); cycles = 8; break;
	    case 0x28: cycles = reljumpcond(getimmsignedbyte(), iszero()); break;
	    case 0x2B: hl.setreg(decreg(hl.getreg())); cycles = 8; break;
	    case 0x2C: hl.setlo(incregbyte(hl.getlo())); cycles = 4; break;
	    case 0x2D: hl.setlo(decregbyte(hl.getlo())); cycles = 4; break;
	    case 0x2E: hl.setlo(getimmbyte()); cycles = 8; break;
	    case 0x30: cycles = reljumpcond(getimmsignedbyte(), !iscarry()); break;
	    case 0x31: sp = getimmword(); cycles = 12; break;
	    case 0x32: load8intomem(hl.getreg(), af.gethi()); hl.setreg(decreg(hl.getreg())); cycles = 8; break;
	    case 0x33: sp = increg(sp); cycles = 8; break;
	    case 0x34: load8intomem(hl.getreg(), incregbyte(store8frommem(hl.getreg()))); cycles = 12; break;
	    case 0x35: load8intomem(hl.getreg(), decregbyte(store8frommem(hl.getreg()))); cycles = 12; break;
	    case 0x36: load8intomem(hl.getreg(), getimmbyte()); cycles = 12; break;
	    case 0x38: cycles = reljumpcond(getimmsignedbyte(), iscarry()); break;
	    case 0x3B: sp = decreg(sp); cycles = 8; break;
	    case 0x3C: af.sethi(incregbyte(af.gethi())); cycles = 4; break;
	    case 0x3D: af.sethi(decregbyte(af.gethi())); cycles = 4; break;
	    case 0x3E: af.sethi(getimmbyte()); cycles = 8; break;
	    case 0x40: bc.sethi(bc.gethi()); cycles = 4; break;
	    case 0x41: bc.sethi(bc.getlo()); cycles = 4; break;
	    case 0x42: bc.sethi(de.gethi()); cycles = 4; break;
	    case 0x43: bc.sethi(de.getlo()); cycles = 4; break;
	    case 0x44: bc.sethi(hl.gethi()); cycles = 4; break;
	    case 0x45: bc.sethi(hl.getlo()); cycles = 4; break;
	    case 0x46: bc.sethi(store8frommem(hl.getreg())); cycles = 8; break;
	    case 0x47: bc.sethi(af.gethi()); cycles = 4; break;
	    case 0x48: bc.setlo(bc.gethi()); cycles = 4; break;
	    case 0x49: bc.setlo(bc.getlo()); cycles = 4; break;
	    case 0x4A: bc.setlo(de.gethi()); cycles = 4; break;
	    case 0x4B: bc.setlo(de.getlo()); cycles = 4; break;
	    case 0x4C: bc.setlo(hl.gethi()); cycles = 4; break;
	    case 0x4D: bc.setlo(hl.getlo()); cycles = 4; break;
	    case 0x4E: bc.setlo(store8frommem(hl.getreg())); cycles = 8; break;
	    case 0x4F: bc.setlo(af.gethi()); cycles = 4; break;
	    case 0x50: de.sethi(bc.gethi()); cycles = 4; break;
	    case 0x51: de.sethi(bc.getlo()); cycles = 4; break;
	    case 0x52: de.sethi(de.gethi()); cycles = 4; break;
	    case 0x53: de.sethi(de.getlo()); cycles = 4; break;
	    case 0x54: de.sethi(hl.gethi()); cycles = 4; break;
	    case 0x55: de.sethi(hl.getlo()); cycles = 4; break;
	    case 0x56: de.sethi(store8frommem(hl.getreg())); cycles = 8; break;
	    case 0x57: de.sethi(af.gethi()); cycles = 4; break;
	    case 0x58: de.setlo(bc.gethi()); cycles = 4; break;
	    case 0x59: de.setlo(bc.getlo()); cycles = 4; break;
	    case 0x5A: de.setlo(de.gethi()); cycles = 4; break;
	    case 0x5B: de.setlo(de.getlo()); cycles = 4; break;
	    case 0x5C: de.setlo(hl.gethi()); cycles = 4; break;
	    case 0x5D: de.setlo(hl.getlo()); cycles = 4; break;
	    case 0x5E: de.setlo(store8frommem(hl.getreg())); cycles = 8; break;
	    case 0x5F: de.setlo(af.gethi()); cycles = 4; break;
	    case 0x60: hl.sethi(bc.gethi()); cycles = 4; break;
	    case 0x61: hl.sethi(bc.getlo()); cycles = 4; break;
	    case 0x62: hl.sethi(de.gethi()); cycles = 4; break;
	    case 0x63: hl.sethi(de.getlo()); cycles = 4; break;
	    case 0x64: hl.sethi(hl.gethi()); cycles = 4; break;
	    case 0x65: hl.sethi(hl.getlo()); cycles = 4; break;
	    case 0x66: hl.sethi(store8frommem(hl.getreg())); cycles = 8; break;
	    case 0x67: hl.sethi(af.gethi()); cycles = 4; break;
	    case 0x68: hl.setlo(bc.gethi()); cycles = 4; break;
	    case 0x69: hl.setlo(bc.getlo()); cycles = 4; break;
	    case 0x6A: hl.setlo(de.gethi()); cycles = 4; break;
	    case 0x6B: hl.setlo(de.getlo()); cycles = 4; break;
	    case 0x6C: hl.setlo(hl.gethi()); cycles = 4; break;
	    case 0x6D: hl.setlo(hl.getlo()); cycles = 4; break;
	    case 0x6E: hl.setlo(store8frommem(hl.getreg())); cycles = 8; break;
	    case 0x6F: hl.setlo(af.gethi()); cycles = 4; break;
	    case 0x70: load8intomem(hl.getreg(), bc.gethi()); cycles = 8; break;
	    case 0x71: load8intomem(hl.getreg(), bc.getlo()); cycles = 8; break;
	    case 0x72: load8intomem(hl.getreg(), de.gethi()); cycles = 8; break;
	    case 0x73: load8intomem(hl.getreg(), de.getlo()); cycles = 8; break;
	    case 0x74: load8intomem(hl.getreg(), hl.gethi()); cycles = 8; break;
	    case 0x75: load8intomem(hl.getreg(), hl.getlo()); cycles = 8; break;
	    case 0x77: load8intomem(hl.getreg(), af.gethi()); cycles = 8; break;
	    case 0x78: af.sethi(bc.gethi()); cycles = 4; break;
	    case 0x79: af.sethi(bc.getlo()); cycles = 4; break;
	    case 0x7A: af.sethi(de.gethi()); cycles = 4; break;
	    case 0x7B: af.sethi(de.getlo()); cycles = 4; break;
	    case 0x7C: af.sethi(hl.gethi()); cycles = 4; break;
	    case 0x7D: af.sethi(hl.getlo()); cycles = 4; break;
	    case 0x7E: af.sethi(store8frommem(hl.getreg())); cycles = 8; break;
	    case 0x7F: af.sethi(af.gethi()); cycles = 4; break;
	    case 0x90: af.sethi(subreg(bc.gethi())); cycles = 4; break;
	    case 0x91: af.sethi(subreg(bc.getlo())); cycles = 4; break;
	    case 0x92: af.sethi(subreg(de.gethi())); cycles = 4; break;
	    case 0x93: af.sethi(subreg(de.getlo())); cycles = 4; break;
	    case 0x94: af.sethi(subreg(hl.gethi())); cycles = 4; break;
	    case 0x95: af.sethi(subreg(hl.getlo())); cycles = 4; break;
	    case 0x96: af.sethi(subreg(store8frommem(hl.getreg()))); cycles = 8; break;
	    case 0x97: af.sethi(subreg(af.gethi())); cycles = 4; break;
	    case 0xA8: xorreg(bc.gethi()); cycles = 4; break;
	    case 0xA9: xorreg(bc.getlo()); cycles = 4; break;
	    case 0xAA: xorreg(de.gethi()); cycles = 4; break;
	    case 0xAB: xorreg(de.getlo()); cycles = 4; break;
	    case 0xAC: xorreg(hl.gethi()); cycles = 4; break;
	    case 0xAD: xorreg(hl.getlo()); cycles = 4; break;
	    case 0xAE: xorreg(store8frommem(hl.getreg())); cycles = 8; break;
	    case 0xAF: xorreg(af.gethi()); cycles = 4; break;
	    case 0xB8: cmpreg(bc.gethi()); cycles = 4; break;
	    case 0xB9: cmpreg(bc.getlo()); cycles = 4; break;
	    case 0xBA: cmpreg(de.gethi()); cycles = 4; break;
	    case 0xBB: cmpreg(de.getlo()); cycles = 4; break;
	    case 0xBC: cmpreg(hl.gethi()); cycles = 4; break;
	    case 0xBD: cmpreg(hl.getlo()); cycles = 4; break;
	    case 0xBE: cmpreg(store8frommem(hl.getreg())); cycles = 8; break;
	    case 0xBF: cmpreg(af.gethi()); cycles = 4; break;
	    case 0xC0: cycles = retcond(!iszero()); break;
	    case 0xC1: bc.setreg(pop()); cycles = 12; break;
	    case 0xC4: cycles = callcond(getimmword(), !iszero()); break;
	    case 0xC5: push(bc.getreg()); cycles = 16; break;
	    case 0xC8: cycles = retcond(iszero()); break;
	    case 0xC9: ret(); cycles = 16; break;
	    case 0xCB: executenextcbopcode(getimmbyte()); break;
	    case 0xCC: cycles = callcond(getimmword(), iszero()); break;
	    case 0xCD: call(getimmword()); cycles = 24; break;
	    case 0xD0: cycles = retcond(!iszero()); break;
	    case 0xD1: de.setreg(pop()); cycles = 12; break;
	    case 0xD4: cycles = callcond(getimmword(), !iscarry()); break;
	    case 0xD5: push(de.getreg()); cycles = 16; break;
	    case 0xD8: cycles = retcond(iscarry()); break;
	    case 0xDC: cycles = callcond(getimmword(), iscarry()); break;
	    case 0xE0: load8intomem((0xFF00 + getimmbyte()), af.gethi()); cycles = 12; break;
	    case 0xE1: hl.setreg(pop()); cycles = 12; break;
	    case 0xE2: load8intomem((0xFF00 + bc.getlo()), af.gethi()); cycles = 8; break;
	    case 0xE5: push(hl.getreg()); cycles = 16; break;
	    case 0xEA: load8intomem(getimmword(), af.gethi()); cycles = 16; break;
	    case 0xEE: xorreg(getimmbyte()); cycles = 8; break;
	    case 0xF0: af.sethi(store8frommem((0xFF00 + getimmbyte()))); cycles = 8; break;
	    case 0xF1: af.setreg(pop()); cycles = 12; break;
	    case 0xF2: af.sethi(store8frommem((0xFF00 + bc.getlo()))); cycles = 8; break;
	    case 0xF5: push(af.getreg()); cycles = 16; break;
	    case 0xFA: af.sethi(store8frommem(getimmword())); cycles = 16; break;
	    case 0xFE: cmpreg(getimmbyte()); cycles = 8; break;
	    default: cout << "Unrecognized opcode at 0x" << uppercase << hex << (int)(opcode) << endl; exit(1); break;
	}

	return cycles;
    }

    int CPU::executenextcbopcode(uint8_t opcode)
    {
	int cycles = 0;	

	switch (opcode)
	{
	    case 0x00: bc.sethi(rlc(bc.gethi())); cycles = 8; break;
	    case 0x01: bc.setlo(rlc(bc.getlo())); cycles = 8; break;
	    case 0x02: de.sethi(rlc(de.gethi())); cycles = 8; break;
	    case 0x03: de.setlo(rlc(de.getlo())); cycles = 8; break;
	    case 0x04: hl.sethi(rlc(hl.gethi())); cycles = 8; break;
	    case 0x05: hl.setlo(rlc(hl.getlo())); cycles = 8; break;
	    case 0x06: load8intomem(hl.getreg(), rlc(store8frommem(hl.getreg()))); cycles = 16; break;
	    case 0x07: af.sethi(rlc(af.gethi())); cycles = 8; break;
	    case 0x08: bc.sethi(rrc(bc.gethi())); cycles = 8; break;
	    case 0x09: bc.setlo(rrc(bc.getlo())); cycles = 8; break;
	    case 0x0A: de.sethi(rrc(de.gethi())); cycles = 8; break;
	    case 0x0B: de.setlo(rrc(de.getlo())); cycles = 8; break;
	    case 0x0C: hl.sethi(rrc(hl.gethi())); cycles = 8; break;
	    case 0x0D: hl.setlo(rrc(hl.getlo())); cycles = 8; break;
	    case 0x0E: load8intomem(hl.getreg(), rrc(store8frommem(hl.getreg()))); cycles = 16; break;
	    case 0x0F: af.sethi(rrc(af.gethi())); cycles = 8; break;
	    case 0x10: bc.sethi(rl(bc.gethi())); cycles = 8; break;
	    case 0x11: bc.setlo(rl(bc.getlo())); cycles = 8; break;
	    case 0x12: de.sethi(rl(de.gethi())); cycles = 8; break;
	    case 0x13: de.setlo(rl(de.getlo())); cycles = 8; break;
	    case 0x14: hl.sethi(rl(hl.gethi())); cycles = 8; break;
	    case 0x15: hl.setlo(rl(hl.getlo())); cycles = 8; break;
	    case 0x16: load8intomem(hl.getreg(), rl(store8frommem(hl.getreg()))); cycles = 16; break;
	    case 0x17: af.sethi(rl(af.gethi())); cycles = 8; break;
	    case 0x18: bc.sethi(rr(bc.gethi())); cycles = 8; break;
	    case 0x19: bc.setlo(rr(bc.getlo())); cycles = 8; break;
	    case 0x1A: de.sethi(rr(de.gethi())); cycles = 8; break;
	    case 0x1B: de.setlo(rr(de.getlo())); cycles = 8; break;
	    case 0x1C: hl.sethi(rr(hl.gethi())); cycles = 8; break;
	    case 0x1D: hl.setlo(rr(hl.getlo())); cycles = 8; break;
	    case 0x1E: load8intomem(hl.getreg(), rr(store8frommem(hl.getreg()))); cycles = 16; break;
	    case 0x1F: af.sethi(rr(af.gethi())); cycles = 8; break;
	    case 0x40: bit(bc.gethi(), 0); cycles = 8; break;
	    case 0x41: bit(bc.getlo(), 0); cycles = 8; break;
	    case 0x42: bit(de.gethi(), 0); cycles = 8; break;
	    case 0x43: bit(de.getlo(), 0); cycles = 8; break;
	    case 0x44: bit(hl.gethi(), 0); cycles = 8; break;
	    case 0x45: bit(hl.getlo(), 0); cycles = 8; break;
	    case 0x46: bit(store8frommem(hl.getreg()), 0); cycles = 16; break;
	    case 0x47: bit(af.gethi(), 0); cycles = 8; break;
	    case 0x48: bit(bc.gethi(), 1); cycles = 8; break;
	    case 0x49: bit(bc.getlo(), 1); cycles = 8; break;
	    case 0x4A: bit(de.gethi(), 1); cycles = 8; break;
	    case 0x4B: bit(de.getlo(), 1); cycles = 8; break;
	    case 0x4C: bit(hl.gethi(), 1); cycles = 8; break;
	    case 0x4D: bit(hl.getlo(), 1); cycles = 8; break;
	    case 0x4E: bit(store8frommem(hl.getreg()), 1); cycles = 16; break;
	    case 0x4F: bit(af.gethi(), 1); cycles = 8; break;
	    case 0x50: bit(bc.gethi(), 2); cycles = 8; break;
	    case 0x51: bit(bc.getlo(), 2); cycles = 8; break;
	    case 0x52: bit(de.gethi(), 2); cycles = 8; break;
	    case 0x53: bit(de.getlo(), 2); cycles = 8; break;
	    case 0x54: bit(hl.gethi(), 2); cycles = 8; break;
	    case 0x55: bit(hl.getlo(), 2); cycles = 8; break;
	    case 0x56: bit(store8frommem(hl.getreg()), 2); cycles = 16; break;
	    case 0x57: bit(af.gethi(), 2); cycles = 8; break;
	    case 0x58: bit(bc.gethi(), 3); cycles = 8; break;
	    case 0x59: bit(bc.getlo(), 3); cycles = 8; break;
	    case 0x5A: bit(de.gethi(), 3); cycles = 8; break;
	    case 0x5B: bit(de.getlo(), 3); cycles = 8; break;
	    case 0x5C: bit(hl.gethi(), 3); cycles = 8; break;
	    case 0x5D: bit(hl.getlo(), 3); cycles = 8; break;
	    case 0x5E: bit(store8frommem(hl.getreg()), 3); cycles = 16; break;
	    case 0x5F: bit(af.gethi(), 3); cycles = 8; break;
	    case 0x60: bit(bc.gethi(), 4); cycles = 8; break;
	    case 0x61: bit(bc.getlo(), 4); cycles = 8; break;
	    case 0x62: bit(de.gethi(), 4); cycles = 8; break;
	    case 0x63: bit(de.getlo(), 4); cycles = 8; break;
	    case 0x64: bit(hl.gethi(), 4); cycles = 8; break;
	    case 0x65: bit(hl.getlo(), 4); cycles = 8; break;
	    case 0x66: bit(store8frommem(hl.getreg()), 4); cycles = 16; break;
	    case 0x67: bit(af.gethi(), 4); cycles = 8; break;
	    case 0x68: bit(bc.gethi(), 5); cycles = 8; break;
	    case 0x69: bit(bc.getlo(), 5); cycles = 8; break;
	    case 0x6A: bit(de.gethi(), 5); cycles = 8; break;
	    case 0x6B: bit(de.getlo(), 5); cycles = 8; break;
	    case 0x6C: bit(hl.gethi(), 5); cycles = 8; break;
	    case 0x6D: bit(hl.getlo(), 5); cycles = 8; break;
	    case 0x6E: bit(store8frommem(hl.getreg()), 5); cycles = 16; break;
	    case 0x6F: bit(af.gethi(), 5); cycles = 8; break;
	    case 0x70: bit(bc.gethi(), 6); cycles = 8; break;
	    case 0x71: bit(bc.getlo(), 6); cycles = 8; break;
	    case 0x72: bit(de.gethi(), 6); cycles = 8; break;
	    case 0x73: bit(de.getlo(), 6); cycles = 8; break;
	    case 0x74: bit(hl.gethi(), 6); cycles = 8; break;
	    case 0x75: bit(hl.getlo(), 6); cycles = 8; break;
	    case 0x76: bit(store8frommem(hl.getreg()), 6); cycles = 16; break;
	    case 0x77: bit(af.gethi(), 6); cycles = 8; break;
	    case 0x78: bit(bc.gethi(), 7); cycles = 8; break;
	    case 0x79: bit(bc.getlo(), 7); cycles = 8; break;
	    case 0x7A: bit(de.gethi(), 7); cycles = 8; break;
	    case 0x7B: bit(de.getlo(), 7); cycles = 8; break;
	    case 0x7C: bit(hl.gethi(), 7); cycles = 8; break;
	    case 0x7D: bit(hl.getlo(), 7); cycles = 8; break;
	    case 0x7E: bit(store8frommem(hl.getreg()), 7); cycles = 16; break;
	    case 0x7F: bit(af.gethi(), 7); cycles = 8; break;
	    default: cout << "Unrecognized extended opcode at 0xCB" << uppercase << hex << (int)(opcode) << endl; exit(1); break;
	}

	return cycles;
    }
}