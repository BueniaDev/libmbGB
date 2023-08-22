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
	    }
	    break;
	}
    }

    void GBSerial::shiftSerialBit()
    {
	dev->sendBit(testbit(reg_sb, 7));
	reg_sb <<= 1;
	reg_sb |= dev->getBit();

	if (--shift_counter == 0)
	{
	    reg_sc &= 0x7F;
	    fireInterrupt();
	}
    }

    void GBSerial::tickSerial()
    {
	serial_clock += 1;

	if ((shift_counter == 0) && testbit(reg_sc, 7))
	{
	    shift_counter = 8;
	}

	if ((shift_counter > 0) && !transfer_signal && prev_transfer_signal)
	{
	    shiftSerialBit();
	}

	prev_transfer_signal = transfer_signal;

	bool serial_inc = false;

	if (testbit(reg_sc, 0))
	{
	    serial_inc = testbit(serial_clock, getInternalClockBit());
	}
	else
	{
	    serial_inc = dev->getClockBit(serial_clock);
	}

	if (!serial_inc && prev_inc)
	{
	    transfer_signal = !transfer_signal;
	}

	prev_inc = serial_inc;
    }
};