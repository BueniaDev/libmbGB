/*
    This file is part of libmbGB.
    Copyright (C) 2022 BueniaDev.

    libmbGB is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    libmbGB is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with libmbGB.  If not, see <https://www.gnu.org/licenses/>.
*/

#include "serial.h"
using namespace gb;
using namespace std;

namespace gb
{
    GBSerial::GBSerial()
    {

    }

    GBSerial::~GBSerial()
    {

    }

    void GBSerial::init()
    {
	serial_clock = 0;
	shift_counter = 0;

	if (dev == NULL)
	{
	    connectSerialDevice(new GBSerialDevice());
	}

	dev->init();
    }

    void GBSerial::shutdown()
    {
	return;
    }

    uint8_t GBSerial::readIO(uint16_t addr)
    {
	uint8_t data = 0;
	addr &= 0xFF;

	switch (addr)
	{
	    case 0x01: data = reg_sb; break;
	    case 0x02: data = reg_sc; break;
	    default: throw mbGBUnmappedMemory(addr, true); break;
	}

	return data;
    }

    void GBSerial::writeIO(uint16_t addr, uint8_t data)
    {
	addr &= 0xFF;

	switch (addr)
	{
	    case 0x01: reg_sb = data; break;
	    case 0x02:
	    {
		reg_sc = data;
		is_pending_receive = false;
	    }
	    break;
	}
    }

    void GBSerial::tickSerial()
    {
	if (!testbit(reg_sc, 7) || is_pending_receive)
	{
	    return;
	}

	int cycles = getTransferRate();

	if (testbit(reg_sc, 0))
	{
	    serial_clock += 1;

	    if (serial_clock == cycles)
	    {
		serial_clock = 0;

		shift_counter += 1;

		if (shift_counter == 8)
		{
		    shift_counter = 0;
		    signalTransfer();
		}
	    }
	}
	else
	{
	    signalTransfer();
	}
    }
};