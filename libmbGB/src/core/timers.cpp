// This file is part of libmbGB.
// Copyright (C) 2020 Buenia.
//
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

#include "../../include/libmbGB/timers.h"
using namespace gb;
using namespace std;
using namespace std::placeholders;

namespace gb
{
    Timers::Timers(MMU& memory) : timermem(memory)
    {
	for (int i = 0xFF04; i <= 0xFF07; i++)
	{
	    timermem.addmemoryreadhandler(i, bind(&Timers::readtimer, this, _1));
	    timermem.addmemorywritehandler(i, bind(&Timers::writetimer, this, _1, _2));
	}
    }

    Timers::~Timers()
    {

    }

    void Timers::init()
    {
	if (timermem.isdmgmode())
	{
	    if (timermem.isdmgconsole())
	    {
		divider = 0xABCC;
	    }
	    else
	    {
		divider = 0x267C;
	    }
	}
	else
	{
	    divider = 0x1EA0;
	}
		
	cout << "Timers::Initialized" << endl;
    }

    void Timers::shutdown()
    {
	cout << "Timers::Shutting down..." << endl;
    }
	
    uint8_t Timers::readtimer(uint16_t addr)
    {
	uint8_t temp = 0;
		
	switch ((addr & 0xFF))
	{
	    case 0x04: temp = (divider >> 8); break;
	    case 0x05: temp = tima; break;
	    case 0x06: temp = tma; break;
	    case 0x07: temp = (tac | 0xF8); break;
	}
		
	return temp;
    }
	
    void Timers::writetimer(uint16_t addr, uint8_t value)
    {
	switch ((addr & 0xFF))
	{
	    case 0x04: divider = 0x0000; break;
	    case 0x05: tima = value; break;
	    case 0x06: tma = value; break;
	    case 0x07: tac = (value & 0x07); break;
	}
    }

    void Timers::updatetimer()
    {
	divider += 4;

	if (timaoverflownotinterrupted)
	{
	    tima = tma;
	    timaoverflownotinterrupted = false;
	}

	if (timaoverflow)
	{
	    if (prevtimaval == tima)
	    {
		timaoverflownotinterrupted = true;
		tima = tma;
		timermem.requestinterrupt(2);
	    }
	    else
	    {
		timaoverflownotinterrupted = false;
	    }

	    timaoverflow = false;
	}

	bool divtickbit = (divbit[tac & 0x3] & divider);
	bool timainc = (divtickbit && TestBit(tac, 2));

	if (!timainc && prevtimainc)
	{
	    timaoverflow = (++tima == 0x00);
	}

	prevtimaval = tima;
	prevtimainc = timainc;
    }
};
