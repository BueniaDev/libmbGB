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
	    resetBIOS();
	}

	CPU::~CPU()
	{
	    cout << "CPU::Shuting down..." << endl;
	}


	void CPU::reset()
	{
	    af.reg = 0x01B0;
	    bc.reg = 0x0013;
	    de.reg = 0x00D8;
	    hl.reg = 0x014D;
	    pc = 0x0100;
	    sp = 0xFFFE;

	    stopped = false;
	    halted = false;

	    interrupt = false;
	    interruptdelay = false;

	    cout << "CPU::Initialized" << endl;
	}

	void CPU::resetBIOS()
	{
	    af.reg = 0x0000;
	    bc.reg = 0x0000;
	    de.reg = 0x0000;
	    hl.reg = 0x0000;
	    pc = 0x0000;
	    sp = 0x0000;

	    stopped = false;
	    halted = false;

	    interrupt = false;
	    interruptdelay = false;

	    cout << "CPU::Initialized" << endl;
	}

	void CPU::dointerrupts()
	{
	    if (interruptdelay)
	    {
		interruptdelay = false;
		interrupt = true;
	    }
	    else if (interrupt)
	    {
		for (int i = 0; i < 5; i++)
		{
		    serviceinterrupt(i);
		}
	    }
	    else
	    {
		return;
	    }
	}

	void CPU::requestinterrupt(int id)
	{
	    uint8_t req = mem->readByte(0xFF0F);
	    req = BitSet(req, id);
	    mem->writeByte(0xFF0F, req);
	}

	void CPU::serviceinterrupt(int id)
	{
	    if (TestBit(mem->readByte(0xFFFF), id) && (TestBit(mem->readByte(0xFF0F), id)))
	    {
		interrupt = false;
		halted = false;

		uint8_t req = mem->readByte(0xFF0F);
		BitReset(req, id);
		mem->writeByte(0xFF0F, req);

		sp--;
		mem->writeByte(sp, (pc >> 8) & 0xFF);
		sp--;
		mem->writeByte(sp, pc & 0xFF);
		switch (id)
		{
		    case 0: pc = 0x40; break;
		    case 1: pc = 0x48; break;
		    case 2: pc = 0x50; break;
		    case 3: pc = 0x58; break;
		    case 4: pc = 0x60; break;
		}

		m_cycles += 36;
	    }
	}

	void CPU::executenextopcode()
	{
	    if (!halted)
	    {
	    	uint8_t opcode = mem->readByte(pc++);
		executeopcode(opcode);
	    }
	    else
	    {
		m_cycles += 4;
	    }
	}

	// Stolen
	void CPU::daa()
	{
	    uint8_t tempF = af.lo & 0x50;

	    if (TestBit(af.lo, subtract))
	    {
		if (TestBit(af.lo, half))
		{
		    af.hi -= 0x06;
		}

		if (TestBit(af.lo, carry))
		{
		    af.hi -= 0x60;
		}
	    }
	    else
	    {
		if ((TestBit(af.lo, carry)) || (af.hi > 0x99))
		{
		    if ((TestBit(af.lo, half)) || (af.hi & 0x0F) > 0x09)
		    {
			af.hi += 0x66;
		    }
		    else
		    {
			af.hi += 0x60;
		    }
		    tempF |= 0x10;
		}
		else if ((TestBit(af.lo, half)) || (af.hi & 0x0F) > 0x09)
		{
		    af.hi += 0x06;
		}
	    }

	    if (af.hi == 0)
	    {
		tempF = BitSet(tempF, zero);
	    }

	    af.lo = tempF;

	    m_cycles += 4;
	}

	void CPU::stop()
	{
	    stopped = true;
	    halted = true;
	    pc++;
	}

	uint8_t CPU::add8bit(uint8_t regone, uint8_t regtwo, bool carry)
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
		af.lo = BitSet(af.lo, zero);
	    }

	    BitReset(af.lo, subtract);

	    uint16_t halftest = regone & 0xF;
	    halftest += adding & 0xF;

	    if ((halftest & 0x10) == 0x10)
	    {
		af.lo = BitSet(af.lo, half);
	    }

	    if ((regone + adding) > 0xFF)
	    {
		af.lo = BitSet(af.lo, carry);
	    }

	    return regone + adding;
	}

	uint8_t CPU::sub8bit(uint8_t regone, uint8_t regtwo, bool carry)
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

	    if ((regone - sub) == 0)
	    {
		af.lo = BitSet(af.lo, zero);
	    }

	    af.lo = BitSet(af.lo, subtract);

	    if (regone < sub)
	    {
		af.lo = BitSet(af.lo, carry);
	    }

	    int16_t halftest = (regone & 0xF);
	    halftest -= (sub & 0xF);
	
	    if (halftest < 0)
	    {
		af.lo = BitSet(af.lo, half);
	    }

	    return regone - sub;
	}

	uint8_t CPU::and8bit(uint8_t regone, uint8_t regtwo)
	{
	    af.lo = 0;

	    if ((regone & regtwo) == 0)
	    {
		af.lo = BitSet(af.lo, zero);
	    }

	    af.lo = BitReset(af.lo, subtract);
	    af.lo = BitSet(af.lo, half);
	    af.lo = BitReset(af.lo, carry);

	    return regone & regtwo;
	}

	uint8_t CPU::or8bit(uint8_t regone, uint8_t regtwo)
	{
	    af.lo = 0;

	    if ((regone | regtwo) == 0)
	    {
		af.lo = BitSet(af.lo, zero);
	    }

	    af.lo = BitReset(af.lo, subtract);
	    af.lo = BitReset(af.lo, half);
	    af.lo = BitReset(af.lo, carry);

	    return regone | regtwo;
	}

	uint8_t CPU::xor8bit(uint8_t regone, uint8_t regtwo)
	{
	    af.lo = 0;

	    if ((regone ^ regtwo) == 0)
	    {
		af.lo = BitSet(af.lo, zero);
	    }

	    af.lo = BitReset(af.lo, subtract);
	    af.lo = BitReset(af.lo, half);
	    af.lo = BitReset(af.lo, carry);

	    return regone ^ regtwo;
	}

	uint8_t CPU::inc8bit(uint8_t regone)
	{
	    uint8_t carryflag = TestBit(af.lo, carry) ? 1 : 0;
	    regone++;

	    af.lo = 0;

	    if (regone == 0)
	    {
		af.lo = BitSet(af.lo, zero);
	    }
	    else
	    {
		af.lo = BitReset(af.lo, zero);
	    }

	    BitReset(af.lo, subtract);

	    if ((regone & 0xF) == 0)
	    {
		af.lo = BitSet(af.lo, half);
	    }
	    else
	    {
		af.lo = BitReset(af.lo, half);
	    }

	    if (carryflag == 1)
	    {
		af.lo = BitSet(af.lo, carry);
	    }

	    return regone;
	}

	uint8_t CPU::dec8bit(uint8_t regone)
	{
	    uint8_t carryflag = TestBit(af.lo, carry) ? 1 : 0;
	    regone--;

	    af.lo = 0;

	    if (regone == 0)
	    {
		af.lo = BitSet(af.lo, zero);
	    }
	    else
	    {
		af.lo = BitReset(af.lo, zero);
	    }

	    af.lo = BitSet(af.lo, subtract);

	    if ((regone & 0xF) == 0xF)
	    {
		af.lo = BitSet(af.lo, half);
	    }
	    else
	    {
		af.lo = BitReset(af.lo, half);
	    }

	    if (carryflag == 1)
	    {
		af.lo = BitSet(af.lo, carry);
	    }

	    return regone;
	}

	uint8_t CPU::compare8bit(uint8_t regone, uint8_t regtwo)
	{
	    uint8_t sub = regtwo;

	    af.lo = 0;

	    if ((regone - sub) == 0)
	    {
		af.lo = BitSet(af.lo, zero);
	    }

	    af.lo = BitSet(af.lo, subtract);

	    if (regone < sub)
	    {
		af.lo = BitSet(af.lo, carry);
	    }

	    int16_t htest = (regone & 0xF);
	    htest -= (sub & 0xF);

	    if (htest < 0)
	    {
		af.lo = BitSet(af.lo, half);
	    }

	    return regone - sub;
	}

	uint16_t CPU::add16bit(uint16_t regone, uint16_t regtwo)
	{
	    af.lo = 0;

	    af.lo = BitReset(af.lo, subtract);

	    if ((regone + regtwo) > 0xFFFF)
	    {
		af.lo = BitSet(af.lo, carry);
	    }
	    else
	    {
		af.lo = BitReset(af.lo, carry);
	    }

	    if (((regone & 0xFF00) & 0xF) + ((regtwo >> 8) & 0xF))
	    {
		af.lo = BitSet(af.lo, half);
	    }
	    else
	    {
		af.lo = BitReset(af.lo, half);
	    }

	    return regone + regtwo;
	}

	uint16_t CPU::adds16bit(uint16_t regone, uint8_t regtwo)
	{
	    int16_t regtwobsx = (int16_t)((int8_t)regtwo);
	    uint16_t result = regone + regtwobsx;

	    af.lo = 0;

	    af.lo = BitReset(af.lo, zero);
	    af.lo = BitReset(af.lo, subtract);

	    if (((regone & 0xFF) + regtwo) > 0xFF)
	    {
		af.lo = BitSet(af.lo, carry);
	    }
	    else
	    {
		af.lo = BitReset(af.lo, carry);
	    }

	    if (((regone & 0xF) + (regtwo & 0xF)) > 0xF)
	    {
		af.lo = BitSet(af.lo, half);
	    }
	    else
	    {
		af.lo = BitReset(af.lo, half);
	    }

	    return result;
	}

	uint8_t CPU::swap(uint8_t regone)
	{
	    af.lo = 0;
	    uint8_t tempone = (regone & 0xF) << 4;
	    uint8_t temptwo = (regone >> 4) & 0xF;
	    regone = (tempone | temptwo);

	    if (regone == 0)
	    {
		af.lo = BitSet(af.lo, zero);
	    }

	    af.lo = BitReset(af.lo, subtract);
	    af.lo = BitReset(af.lo, half);
	    af.lo = BitReset(af.lo, carry);

	    return regone;
	}

	uint8_t CPU::rl(uint8_t regone)
	{	    
	    uint8_t oldcarry = TestBit(af.lo, carry) ? 1 : 0;
	    af.lo = 0;

	    uint8_t carryflag = TestBit(regone, 7) ? 1 : 0;
	    regone = (regone << 1) + oldcarry;

	    if (carryflag == 1)
	    {
		af.lo = BitSet(af.lo, carry);
	    }

	    if (regone == 0)
	    {
		af.lo = BitSet(af.lo, zero);
	    }

	    af.lo = BitReset(af.lo, subtract);
	    af.lo = BitReset(af.lo, half);

	    return regone;
	}

	uint8_t CPU::rlc(uint8_t regone)
	{
	    uint8_t carryflag = TestBit(af.lo, carry) ? 1 : 0;

	    af.lo = 0;

	    regone = (regone << 1) + carryflag;

	    if (carryflag == 1)
	    {
		af.lo = BitSet(af.lo, carry);
	    }

	    if (regone == 0)
	    {
		af.lo = BitSet(af.lo, zero);
	    }

	    af.lo = BitReset(af.lo, subtract);
	    af.lo = BitReset(af.lo, half);

	    return regone;
	}

	uint8_t CPU::rr(uint8_t regone)
	{
	    uint8_t oldcarry = TestBit(af.lo, carry) ? 1 : 0;
	    af.lo = 0;

	    uint8_t carryflag = TestBit(af.lo, 0) ? 1 : 0;
	    regone = (regone >> 1) + (oldcarry << 7);

	    if (carryflag == 1)
	    {
		af.lo = BitSet(af.lo, carry);
	    }

	    if (regone == 0)
	    {
		af.lo = BitSet(af.lo, zero);
	    }

	    af.lo = BitReset(af.lo, subtract);
	    af.lo = BitReset(af.lo, half);

	    return regone;
	}

	uint8_t CPU::rrc(uint8_t regone)
	{
	    af.lo = 0;

	    uint8_t carryflag = TestBit(regone, 0) ? 1 : 0;

	    regone = (regone >> 1) | (carryflag << 7);

	    if (carryflag == 1)
	    {
		af.lo = BitSet(af.lo, carry);
	    }

	    if (regone == 0)
	    {
		af.lo = BitSet(af.lo, zero);
	    }

	    af.lo = BitReset(af.lo, subtract);
	    af.lo = BitReset(af.lo, half);

	    return regone;
	}

	uint8_t CPU::sla(uint8_t regone)
	{
	    af.lo = 0;

	    regone <<= 1;

	    if (TestBit(regone, 7))
	    {
		af.lo = BitSet(af.lo, carry);
	    }

	    if (regone == 0)
	    {
		af.lo = BitSet(af.lo, zero);
	    }

	    af.lo = BitReset(af.lo, subtract);
	    af.lo = BitReset(af.lo, half);

	    return regone;
	}

	uint8_t CPU::sra(uint8_t regone)
	{
	    af.lo = 0;

	    regone >>= 1;

	    if (TestBit(regone, 7))
	    {
		af.lo = BitSet(regone, 7);
	    }

	    if (TestBit(regone, 0))
	    {
		af.lo = BitSet(af.lo, carry);
	    }

	    if (regone == 0)
	    {
		af.lo = BitSet(af.lo, zero);
	    }

	    af.lo = BitReset(af.lo, subtract);
	    af.lo = BitReset(af.lo, half);

	    return regone;
	}

	uint8_t CPU::srl(uint8_t regone)
	{
	    af.lo = 0;

	    regone >>= 1;

	    if (TestBit(regone, 0))
	    {
		af.lo = BitSet(af.lo, carry);
	    }

	    if (regone == 0)
	    {
		af.lo = BitSet(af.lo, zero);
	    }

	    af.lo = BitReset(af.lo, subtract);
	    af.lo = BitReset(af.lo, half);

	    return regone;
	}

	void CPU::bit(uint8_t regone, int bit)
	{
	    af.lo = 0;
	    if (TestBit(regone, bit))
	    {
		af.lo = BitReset(af.lo, zero);
	    }
	    else
	    {
		af.lo = BitSet(af.lo, zero);
	    }

	    af.lo = BitReset(af.lo, subtract);
	    af.lo = BitSet(af.lo, half);
	}

	uint8_t CPU::set(uint8_t regone, int bit)
	{
	    regone = BitSet(regone, bit);
	    return regone;
	}

	uint8_t CPU::res(uint8_t regone, int bit)
	{
	    regone = BitReset(regone, bit);
	    return regone;
	}

	void CPU::jr(uint8_t regone)
	{
            if (TestBit(regone, zero))
            {
                regone--;
                regone = ~regone;
                pc -= regone;
            }
            else
            {
                pc += regone;
            }
	}
}
