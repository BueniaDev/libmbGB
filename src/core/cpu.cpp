#include "../../include/libmbGB/cpu.h"
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
}
