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

namespace gb
{
    Serial::Serial(MMU& memory) : serialmem(memory)
    {

    }

    Serial::~Serial()
    {

    }

    SerialDevice::SerialDevice()
    {

    }

    SerialDevice::~SerialDevice()
    {

    }

    void Serial::init()
    {
	initserialclock((serialmem.divider & 0xFF));
	cout << "Serial::Initialized" << endl;
    }

    void Serial::shutdown()
    {
	cout << "Serial::Shutting down..." << endl;
    }

    void Serial::updateserial()
    {
	serialclock += 4;

	if (bitstoshift == 0 && TestBit(serialmem.sc, 7))
	{
	    bitstoshift = 8;
	}

	if (bitstoshift > 0 && !transfersignal && prevtransfersignal)
	{
	    shiftserialbit();
	}

	prevtransfersignal = transfersignal;

	bool serialinc = (TestBit(serialclock, selectclockbit()) && usinginternalclock());

	if (!serialinc && previnc)
	{
	    transfersignal = !transfersignal;
	}

	previnc = serialinc;
    }
};
