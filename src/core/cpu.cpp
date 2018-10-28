#include "../../include/libmbGB/cpu.h"
#include "../../include/libmbGB/mmu.h"
#include "../../include/libmbGB/utils.h"
#include <iostream>
using namespace std;
using namespace gb;

namespace gb
{
	CPU::CPU()
	{
	    if (mem->biosload)
	    {
		CPUResetBIOS();
	    }
	    else
	    {
		CPUReset();
	    }
	}

	CPU::~CPU()
	{
	    cout << "CPU::Shuting down..." << endl;
	}


	void CPU::CPUReset()
	{
	    af.reg = 0x01B0;
	    bc.reg = 0x0013;
	    de.reg = 0x00D8;
	    hl.reg = 0x014D;
	    pc = 0x0100;
	    sp = 0xFFFE;
	}

	void CPU::CPUResetBIOS()
	{
	    af.reg = 0x0000;
	    bc.reg = 0x0000;
	    de.reg = 0x0000;
	    hl.reg = 0x0000;
	    pc = 0x0000;
	    sp = 0x0000;
	}

	void CPU::executenextopcode()
	{
	    uint8_t opcode = mem->readByte(pc++);
	    if (opcode != 0xCB)
	    {
		executeopcode(opcode);
	    }
	    else
	    {
		opcode = mem->readByte(pc++);		
		executecbopcode(opcode);
	    }
	}
	
	// Stolen
	void CPU::daa()
	{
	    uint8_t tempF = af.lo & 0x50;

	    if ((af.lo & 0x40) == 0x40)
	    {
		if ((af.lo & 0x20) == 0x20)
		{
		    af.hi -= 0x06;
		}

		if ((af.lo & 0x10) == 0x10)
		{
		    af.hi -= 0x60;
		}
	    }
	    else
	    {
		if (((af.lo & 0x10) == 0x10) || (af.hi > 0x99))
		{
		    if (((af.lo & 0x20) == 0x20) || (af.hi & 0x0F) > 0x09)
		    {
			af.hi += 0x66;
		    }
		    else
		    {
			af.hi += 0x60;
		    }
		    tempF |= 0x10;
		}
		else if (((af.lo & 0x20) == 0x20) || (af.hi & 0x0F) > 0x09)
		{
		    af.hi += 0x06;
		}
	    }

	    if (af.hi == 0)
	    {
		tempF |= 0x80;
	    }
	    af.lo = tempF;

	    m_cycles += 4;
	}

	void CPU::load8bit(uint8_t regone, uint8_t regtwo, int cycles)
	{
	    regone = regtwo;
	    m_cycles += cycles;
	}

	void CPU::load16bit(uint16_t regone, uint16_t regtwo, int cycles)
	{
	    regone = regtwo;
	    m_cycles += cycles;
	}

	void CPU::pushontostack(uint16_t regone, int cycles)
	{
	    uint8_t hi = regone >> 8;
	    uint8_t lo = regone & 0xFF;
	    mem->writeByte(sp--, hi);
	    mem->writeByte(sp--, lo);

	    m_cycles += cycles;
	}

	void CPU::popontostack(uint16_t regone, int cycles)
	{
	    uint16_t tempword = mem->readByte(sp + 1) << 8;
	    tempword |= mem->readByte(sp);
	    sp += 2;
	
	    regone = tempword;
	    m_cycles += cycles;
	}

	void CPU::add8bit(uint8_t regone, uint8_t regtwo, int cycles, bool carry)
	{
	    uint8_t adding = regtwo;

	    if (carry)
	    {
		if (TestBit(af.lo, carry))
		{
		    adding++;
		}
	    }

	    af.lo = 0;

	    if ((regone + adding) == 0)
	    {
		BitSet(af.lo, zero);
	    }

	    BitReset(af.lo, subtract);

	    uint16_t halftest = regone & 0xF;
	    halftest += adding & 0xF;

	    if ((halftest & 0x10) == 0x10)
	    {
		BitSet(af.lo, half);
	    }

	    if ((regone + adding) > 0xFF)
	    {
		BitSet(af.lo, carry);
	    }

	    regone += adding;
	    m_cycles += cycles;
	}

	void CPU::sub8bit(uint8_t regone, uint8_t regtwo, int cycles, bool carry)
	{
	    uint8_t sub = regtwo;

	    if (carry)
	    {
		if (TestBit(af.lo, carry))
		{
		    sub++;
		}
	    }

	    af.lo = 0;

	    if (regone == 0)
	    {
		BitSet(af.lo, zero);
	    }

	    BitSet(af.lo, subtract);

	    if (regone < sub)
	    {
		BitSet(af.lo, carry);
	    }

	    int16_t halftest = (regone & 0xF);
	    halftest -= (sub & 0xF);
	
	    if (halftest < 0)
	    {
		BitSet(af.lo, half);
	    }

	    regone -= sub;
	    m_cycles += cycles;
	}

	void CPU::and8bit(uint8_t regone, uint8_t regtwo, int cycles)
	{
	    af.lo = 0;

	    if ((regone & regtwo) == 0)
	    {
		BitSet(af.lo, zero);
	    }

	    BitReset(af.lo, subtract);
	    BitSet(af.lo, half);
	    BitReset(af.lo, carry);

	    regone &= regtwo;
	    m_cycles += cycles;
	}

	void CPU::or8bit(uint8_t regone, uint8_t regtwo, int cycles)
	{
	    af.lo = 0;

	    if ((regone | regtwo) == 0)
	    {
		BitSet(af.lo, zero);
	    }

	    BitReset(af.lo, subtract);
	    BitReset(af.lo, half);
	    BitReset(af.lo, carry);

	    regone |= regtwo;
	    m_cycles += cycles;
	}

	void CPU::xor8bit(uint8_t regone, uint8_t regtwo, int cycles)
	{
	    af.lo = 0;

	    if ((regone ^ regtwo) == 0)
	    {
		BitSet(af.lo, zero);
	    }

	    BitReset(af.lo, subtract);
	    BitReset(af.lo, half);
	    BitReset(af.lo, carry);

	    regone ^= regtwo;
	    m_cycles += cycles;
	}

	void CPU::inc8bit(uint8_t regone, int cycles)
	{
	    regone++;	

	    af.lo = 0;    

	    if (regone == 0)
	    {
		BitSet(af.lo, zero);
	    }
	    else
	    {
		BitReset(af.lo, zero);
	    }

	    BitReset(af.lo, subtract);

	    if ((regone & 0xF) == 0xF)
	    {
		BitSet(af.lo, half);
	    }
	    else
	    {
		BitReset(af.lo, half);
	    }

	    m_cycles += cycles;
	}

	void CPU::dec8bit(uint8_t regone, int cycles)
	{
	    regone--;

	    af.lo = 0;

	    if (regone == 0)
	    {
		BitSet(af.lo, zero);
	    }
	    else
	    {
		BitReset(af.lo, zero);
	    }

	    BitSet(af.lo, subtract);

	    if ((regone & 0xF) == 0)
	    {
		BitSet(af.lo, half);
	    }
	    else
	    {
		BitReset(af.lo, half);
	    }

	    m_cycles += cycles;
	}

	void CPU::add16bit(uint16_t regone, uint16_t regtwo, int cycles)
	{
	    af.lo = 0;

	    BitReset(af.lo, subtract);

	    if ((regone + regtwo) > 0xFFFF)
	    {
		BitSet(af.lo, carry);
	    }
	    else
	    {
		BitReset(af.lo, carry);
	    }

	    if (((regone & 0xFF00) & 0xF) + ((regtwo >> 8) & 0xF))
	    {
		BitSet(af.lo, half);
	    }
	    else
	    {
		BitReset(af.lo, half);
	    }

	    regone += regtwo;
	    m_cycles += cycles;
	}

	void CPU::adds16bit(uint16_t regone, uint8_t regtwo, int cycles)
	{
	    int16_t regtwobsx = (int16_t)((int8_t)regtwo);
	    uint16_t result = regone + regtwobsx;
	
	    af.lo = 0;

	    BitReset(af.lo, zero);
	    BitReset(af.lo, subtract);

	    if (((regone & 0xFF) + regtwo) > 0xFF)
	    {
		BitSet(af.lo, carry);
	    }
	    else
	    {
		BitReset(af.lo, carry);
	    }

	    if (((regone & 0xF) + (regtwo & 0xF)) > 0xF)
	    {
		BitSet(af.lo, half);
	    }
	    else
	    {
		BitReset(af.lo, half);
	    }

	    regone = result;
	    m_cycles += cycles;
	}

	void CPU::inc16bit(uint16_t regone, int cycles)
	{
	    regone++;
	    m_cycles += cycles;
	}

	void CPU::dec16bit(uint16_t regone, int cycles)
	{
	    regone--;
	    m_cycles += cycles;
	}
}
