/*
    This file is part of libmbGB.
    Copyright (C) 2023 BueniaDev.

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

#ifndef LIBMBGB_SERIAL_H
#define LIBMBGB_SERIAL_H

#include "utils.h"
using namespace gb;

namespace gb
{
    class LIBMBGB_API GBSerialDevice
    {
	public:
	    GBSerialDevice()
	    {

	    }

	    ~GBSerialDevice()
	    {

	    }

	    virtual bool getClockBit(int)
	    {
		return false;
	    }

	    virtual void sendBit(bool)
	    {
		return;
	    }

	    virtual bool getBit()
	    {
		return true;
	    }
    };

    #include "addons.inl"

    class LIBMBGB_API GBSerial
    {
	public:
	    GBSerial();
	    ~GBSerial();

	    void init();
	    void shutdown();

	    void tickSerial();

	    uint8_t readIO(uint16_t addr);
	    void writeIO(uint16_t addr, uint8_t data);

	    void setIRQCallback(voidintfunc cb)
	    {
		irq_func = cb;
	    }

	    void connectSerialDevice(GBSerialDevice *cb)
	    {
		dev = cb;
	    }

	private:
	    uint8_t reg_sb = 0;
	    uint8_t reg_sc = 0;

	    voidintfunc irq_func;

	    GBSerialDevice *dev = NULL;

	    bool is_valid_device = false;
	    bool is_pending_receive = false;

	    int serial_clock = 0;
	    int shift_counter = 0;

	    bool transfer_signal = false;
	    bool prev_transfer_signal = false;

	    bool prev_inc = false;

	    void shiftSerialBit();

	    int getInternalClockBit()
	    {
		// TODO: Implement GBC stuff
		return testbit(reg_sc, 1) ? 2 : 7;
	    }

	    void fireInterrupt()
	    {
		if (irq_func)
		{
		    irq_func(3);
		}
	    }
    };
};

#endif // LIBMBGB_SERIAL