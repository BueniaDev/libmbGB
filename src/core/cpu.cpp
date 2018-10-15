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
}
