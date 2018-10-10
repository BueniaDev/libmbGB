#include "../../include/libmbGB/cpu.h"
#include "../../include/libmbGB/utils.h"
#include <iostream>
using namespace std;
using namespace gb;

namespace gb
{
	CPU::CPU()
	{
	    if (biosload)
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
		if ((af.lo & 0x10) == 0x10 || af.hi > 0x99)
		{
		    if ((af.lo & 0x20) == 0x20 || (af.hi & 0x0F) > 0x09)
		    {
			af.hi += 0x66;
		    }
		    else
		    {
			af.hi += 0x60;
		    }
		    tempF |= 0x10;
		}
		else if ((af.lo & 0x20) == 0x20) || (af.hi & 0x0F) > 0x09)
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
}
