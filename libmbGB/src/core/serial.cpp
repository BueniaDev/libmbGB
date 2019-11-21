// This file is part of libmbGB.
// Copyright (C) 2019 Buenia.
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

#include "../../include/libmbGB/serial.h"
using namespace gb;
using namespace std::placeholders;

namespace gb
{
    Serial::Serial(MMU& memory) : serialmem(memory)
    {
		for (int i = 0xFF01; i <= 0xFF02; i++)
		{
			serialmem.addmemoryreadhandler(i, bind(&Serial::readserial, this, _1));
			serialmem.addmemorywritehandler(i, bind(&Serial::writeserial, this, _1, _2));
		}
    }

    Serial::~Serial()
    {

    }

    void Serial::init()
    {
		if (serialmem.isdmgmode())
		{
		    if (serialmem.isdmgconsole())
		    {
			initserialclock(0xCC);
		    }
		    else
		    {
			initserialclock(0x7C);
		    }
		}
		else
		{
		    initserialclock(0xA0);
		}
		
	cout << "Serial::Initialized" << endl;
    }

    void Serial::shutdown()
    {
	cout << "Serial::Shutting down..." << endl;
    }
	
	uint8_t Serial::readserial(uint16_t addr)
	{
		uint8_t temp = 0;
		
		switch ((addr & 0xFF))
		{
			case 0x01: temp = (linkready) ? bytetorecieve : 0xFF; break;
			case 0x02: temp = (sc & 0x7E); break;
		}
		
		return temp;
	}
	
	void Serial::writeserial(uint16_t addr, uint8_t val)
	{
		switch ((addr & 0xFF))
		{
			case 0x01: bytetotransfer = val; break;
			case 0x02: 
			{
			    sc = val;
			    pendingrecieve = false;
			}
			break;
		}
	}

    void Serial::updateserial()
    {
	if (!TestBit(sc, 7) || pendingrecieve)
	{
	    return;
	}

	int cycles = (serialmem.doublespeed) ? 256 : 512;

	if (TestBit(sc, 0))
	{
	    serialclock += 4;

	    if (serialclock == cycles)
	    {
		serialclock = 0;
		shiftcounter += 1;

		if (shiftcounter == 8)
		{
		    signalready();
		    shiftcounter = 0;
		}
	    }
	}
	else
	{
	    signalready();
	}
    }
};
