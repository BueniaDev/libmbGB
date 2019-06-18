// This file is part of libmbGB.
// Copyright (C) 2019 Buenia.ui
// libmbGB is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// libmbGB is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with libmbGB.  If not, see <https://www.gnu.org/licenses/>.

#include "../../include/libmbGB/cpu.h"
using namespace gb;

namespace gb
{
    CPU::CPU(MMU& memory) : mem(memory)
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

	pc = 0;
	sp = 0;

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
	    
	    // TODO: HDMA transfer stuff

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
		// TODO: Halted ticking stuff
		cycles -= 4;
	    }
	}

	return cycles;
    }

    void CPU::hardwaretick(int cycles)
    {
	for (; cycles != 0; cycles -= 4)
	{
	    // TODO: Hardware ticking
	}
    }

    uint16_t CPU::getimmword()
    {
	hardwaretick(4);
	uint16_t temp = mem.readWord(pc + 1);
	hardwaretick(4);

	return temp;
    }
};