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
    CPU::CPU(MMU& memory, GPU& graphics) : mem(memory), gpu(graphics)
    {

    }

    CPU::~CPU()
    {

    }

    void CPU::init()
    {
	if (!mem.biosload)
	{
	    initnobios();
	}
	else
	{
	    initbios();
	}
    }

    void CPU::initnobios()
    {
	if (mem.gbmode == Mode::DMG)
	{
	    if (mem.gameboy == Console::DMG)
	    {
		af.setreg(0x01B0);
		bc.setreg(0x0013);
		de.setreg(0x00D8);
		hl.setreg(0x014D);
	    }
	    else if (mem.gameboy == Console::CGB)
	    {
		af.setreg(0x1180);
		bc.setreg(0x0000);
		de.setreg(0x0008);
		hl.setreg(0x007C);
	    }
	}
	else if (mem.gbmode == Mode::CGB)
	{
	    af.setreg(0x1180);
	    bc.setreg(0x0000);
	    de.setreg(0xFF56);
	    hl.setreg(0x000D);   
	}

	pc = 0x0100;
	sp = 0xFFFE;

	cout << "CPU::Initialized" << endl;
    }

    void CPU::initbios()
    {
	af.setreg(0x0000);
	bc.setreg(0x0000);
	de.setreg(0x0000);
	hl.setreg(0x0000);   

	pc = 0;
	sp = 0;

	cout << "CPU::Initialized" << endl;
    }

    void CPU::shutdown()
    {
	cout << "CPU::Shutting down..." << endl;
    }

    void CPU::printregs()
    {
	cout << "AF: " << hex << (int)(af.getreg()) << endl;
	cout << "BC: " << hex << (int)(bc.getreg()) << endl;
	cout << "DE: " << hex << (int)(de.getreg()) << endl;
	cout << "HL: " << hex << (int)(hl.getreg()) << endl;
	cout << "PC: " << hex << (int)(pc) << endl;
	cout << "SP: " << hex << (int)(sp) << endl;
	cout << "IF: " << hex << (int)(mem.readByte(0xFF0F)) << endl;
	cout << "IE: " << hex << (int)(mem.readByte(0xFFFF)) << endl;
	cout << "IME: " << (int)(interruptmasterenable) << endl;
	cout << "IMA: " << (int)(enableinterruptsdelayed) << endl;
	cout << "REI: " << (int)(mem.requestedenabledinterrupts()) << endl;
	cout << "LCD: " << (int)(mem.ispending(1)) << endl;
	cout << "(FF41): " << hex << (int)(mem.readByte(0xFF41)) << endl;
	cout << "STAT IRQ: " << (int)(mem.statinterruptsignal) << endl;
	cout << endl;
    }

    int CPU::handleinterrupts()
    {
	int temp = 0;

	if (interruptmasterenable)
	{
	    if (mem.requestedenabledinterrupts())
	    {
		hardwaretick(12);

		// printregs();

		interruptmasterenable = false;

		if (mem.ispending(0))
		{
		    mem.clearinterrupt(0);
		    serviceinterrupt(0x40);
		}
		else if (mem.ispending(1))
		{
		    mem.clearinterrupt(1);
		    serviceinterrupt(0x48);
		}
		else if (mem.ispending(2))
		{
		    mem.clearinterrupt(2);
		    serviceinterrupt(0x50);
		}
		else if (mem.ispending(3))
		{
		    mem.clearinterrupt(3);
		    serviceinterrupt(0x58);
		}
		else if (mem.ispending(4))
		{
		    mem.clearinterrupt(4);
		    serviceinterrupt(0x60);
		}

		if (state == CPUState::Halted)
		{
		    state = CPUState::Running;
		}

		temp = 20;
	    }
	}
	else if (state == CPUState::Halted)
	{
	    if (mem.requestedenabledinterrupts())
	    {
		state = CPUState::Running;
		temp = 0;
	    }
	}

	return temp;
    }

    void CPU::serviceinterrupt(uint16_t addr)
    {	
	sp -= 2;
	hardwaretick(4);
	mem.writeWord(sp, pc);
	hardwaretick(4);

	pc = addr;
	// printregs();
    }

    int CPU::runfor(int cycles)
    {
	while (cycles > 0)
	{
	    if (state == CPUState::Stopped)
	    {
		// TODO: CPU stopping
		cycles -= 4;
		continue;
	    }

	    /*
	    if (pc == 0x0100)
	    {
		printregs();
	    }
	    */

	    // cout << (char)(af.gethi()) << endl;
	    
	    // TODO: HDMA transfer stuff

	    cycles -= handleinterrupts();

	    if (state == CPUState::Running)
	    {
		cycles -= executenextopcode(mem.readByte(pc++));
	    }
	    else if (state == CPUState::HaltBug)
	    {
		cycles -= executenextopcode(mem.readByte(pc));
		state = CPUState::Running;
	    }
	    else if (state == CPUState::Halted)
	    {
		haltedtick(4);
		cycles -= 4;
	    }

	    /*
	    printregs();

	    if (pc == 0x01DA)
	    {
		printregs();
		exit(1);
	    }
	    */
	}

	return cycles;
    }

    void CPU::enabledelayedinterrupts()
    {
	interruptmasterenable = (interruptmasterenable || enableinterruptsdelayed);
	enableinterruptsdelayed = false;
    }

    void CPU::hardwaretick(int cycles)
    {
	for (; cycles != 0; cycles -= 4)
	{
	    enabledelayedinterrupts();
	    gpu.updatelcd();
	    mem.ifwrittenthiscycle = false;
	}
    }

    void CPU::haltedtick(int cycles)
    {
	for (; cycles != 0; cycles -= 4)
	{
	    gpu.updatelcd();
	}
    }
};
