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
    CPU::CPU(MMU& memory, GPU& graphics, Timers& timers, Serial& serial) : mem(memory), gpu(graphics), timer(timers), link(serial)
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
		af.reg = (0x01B0);
		bc.reg = (0x0013);
		de.reg = (0x00D8);
		hl.reg = (0x014D);
	    }
	    else if (mem.gameboy == Console::CGB)
	    {
		af.reg = (0x1180);
		bc.reg = (0x0000);
		de.reg = (0x0008);
		hl.reg = (0x007C);
	    }
	}
	else if (mem.gbmode == Mode::CGB)
	{
	    af.reg = (0x1180);
	    bc.reg = (0x0000);
	    de.reg = (0xFF56);
	    hl.reg = (0x000D);   
	}

	pc = 0x0100;
	sp = 0xFFFE;

	interruptmasterenable = false;
	enableinterruptsdelayed = false;

	state = CPUState::Running;
	

	cout << "CPU::Initialized" << endl;
    }

    void CPU::initbios()
    {
	af.reg = (0x0000);
	bc.reg = (0x0000);
	de.reg = (0x0000);
	hl.reg = (0x0000);   

	pc = 0;
	sp = 0;

	interruptmasterenable = false;
	enableinterruptsdelayed = false;

	state = CPUState::Running;

	cout << "CPU::Initialized" << endl;
    }

    void CPU::shutdown()
    {
	cout << "CPU::Shutting down..." << endl;
    }

    bool CPU::loadcpu(string filename)
    {
	ifstream file(filename.c_str(), ios::binary);

	if (!file.is_open())
	{
	    cout << "CPU::Error opening CPU state" << endl;
	    return false;
	}

	file.seekg(0);

	file.read((char*)&af.hi, sizeof(af.hi));
	file.read((char*)&af.lo, sizeof(af.lo));
	file.read((char*)&bc.hi, sizeof(bc.hi));
	file.read((char*)&bc.lo, sizeof(bc.lo));
	file.read((char*)&de.hi, sizeof(de.hi));
	file.read((char*)&de.lo, sizeof(de.lo));
	file.read((char*)&hl.hi, sizeof(hl.hi));
	file.read((char*)&hl.lo, sizeof(hl.lo));

	file.read((char*)&pc, sizeof(pc));
	file.read((char*)&sp, sizeof(sp));
	file.read((char*)&state, sizeof(uint8_t));
	file.read((char*)&interruptmasterenable, sizeof(interruptmasterenable));
	file.read((char*)&enableinterruptsdelayed, sizeof(enableinterruptsdelayed));

	printregs();

	loaded = true;

	file.close();
	return true;
    }

    bool CPU::savecpu(string filename)
    {
	ofstream file(filename.c_str(), ios::binary | ios::trunc);

	if (!file.is_open())
	{
	    cout << "CPU::Error opening CPU state" << endl;
	    return false;
	}

	printregs();

	file.write((char*)&af.hi, sizeof(af.hi));
	file.write((char*)&af.lo, sizeof(af.lo));
	file.write((char*)&bc.hi, sizeof(bc.hi));
	file.write((char*)&bc.lo, sizeof(bc.lo));
	file.write((char*)&de.hi, sizeof(de.hi));
	file.write((char*)&de.lo, sizeof(de.lo));
	file.write((char*)&hl.hi, sizeof(hl.hi));
	file.write((char*)&hl.lo, sizeof(hl.lo));

	file.write((char*)&pc, sizeof(pc));
	file.write((char*)&sp, sizeof(sp));
	file.write((char*)&state, sizeof(uint8_t));
	file.write((char*)&interruptmasterenable, sizeof(interruptmasterenable));
	file.write((char*)&enableinterruptsdelayed, sizeof(enableinterruptsdelayed));

	file.close();
	return true;
    }

    void CPU::printregs()
    {
	cout << "AF: " << hex << (int)(af.reg) << endl;
	cout << "BC: " << hex << (int)(bc.reg) << endl;
	cout << "DE: " << hex << (int)(de.reg) << endl;
	cout << "HL: " << hex << (int)(hl.reg) << endl;
	cout << "PC: " << hex << (int)(pc) << endl;
	cout << "SP: " << hex << (int)(sp) << endl;
	cout << "IF: " << hex << (int)(mem.readByte(0xFF0F)) << endl;
	cout << "IE: " << hex << (int)(mem.readByte(0xFFFF)) << endl;
	cout << "IME: " << (int)(interruptmasterenable) << endl;
	cout << "IMA: " << (int)(enableinterruptsdelayed) << endl;
	cout << "REI: " << (int)(mem.requestedenabledinterrupts()) << endl;
	cout << "LCD: " << (int)(mem.ispending(1)) << endl;
	cout << "TIMA: " << hex << (int)(mem.readByte(0xFF05)) << endl;
	cout << "Opcode: " << hex << (int)(mem.readByte(pc)) << endl;
	cout << endl;
    }

    int CPU::handleinterrupts()
    {
	int temp = 0;

	if (interruptmasterenable)
	{
	    if (mem.requestedenabledinterrupts())
	    {		

		interruptmasterenable = false;

		hardwaretick(8);
		load8intomem(--sp, (pc >> 8));
		hardwaretick(4);

		uint16_t interruptvector = 0x0000;

		if (mem.ispending(0))
		{
		    mem.clearinterrupt(0);
		    interruptvector = 0x0040;
		    // serviceinterrupt(0x40);
		}
		else if (mem.ispending(1))
		{
		    mem.clearinterrupt(1);
		    interruptvector = 0x0048;
		    // serviceinterrupt(0x48);
		}
		else if (mem.ispending(2))
		{
		    mem.clearinterrupt(2);
		    interruptvector = 0x0050;
		    // serviceinterrupt(0x50);
		}
		else if (mem.ispending(3))
		{
		    mem.clearinterrupt(3);
		    interruptvector = 0x0058;
		    // serviceinterrupt(0x58);
		}
		else if (mem.ispending(4))
		{
		    mem.clearinterrupt(4);
		    interruptvector = 0x0060;
		    // serviceinterrupt(0x60);
		}

		load8intomem(--sp, (pc & 0xFF));

		pc = interruptvector;

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
	    }

	    temp = 0;
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
    }

    int CPU::runfor(int cycles)
    {
	while (cycles > 0)
	{
	    if (state == CPUState::Stopped)
	    {
		stoppedtick();
		cycles -= 4;
		continue;
	    }
	    
	    // TODO: HDMA transfer stuff

	    cycles -= handleinterrupts();

	    int temp = 0;

	    if (state == CPUState::Running)
	    {
		temp = executenextopcode(mem.readByte(pc++));
		cycles -= temp;
	    }
	    else if (state == CPUState::HaltBug)
	    {
		temp = executenextopcode(mem.readByte(pc));
		cycles -= temp;
		state = CPUState::Running;
	    }
	    else if (state == CPUState::Halted)
	    {
		haltedtick(4);
		temp = 4;
		cycles -= 4;
	    }

	    // cout << hex << (int)(mem.readByte(0xFFFF)) << endl;
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
	    timer.updatetimer();
	    updatedma();
	    link.updateserial();
	    gpu.updatelcd();
	    mem.ifwrittenthiscycle = false;
	}
    }

    void CPU::haltedtick(int cycles)
    {
	for (; cycles != 0; cycles -= 4)
	{
	    timer.updatetimer();
	    link.updateserial();
	    gpu.updatelcd();
	}
    }
};
