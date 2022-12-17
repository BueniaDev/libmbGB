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

	    virtual void init()
	    {
		return;
	    }

	    virtual void sendByte(uint8_t, bool is_mode)
	    {
		if (is_mode)
		{
		    transfer();
		}
	    }

	    void setCallback(voidfunc cb)
	    {
		rec_func = cb;
	    }

	    void transfer()
	    {
		if (rec_func)
		{
		    rec_func();
		}
	    }

	    virtual uint8_t getByte()
	    {
		return 0xFF;
	    }

	private:
	    bool is_transfering = false;

	    voidfunc rec_func;
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

		if (dev != NULL)
		{
		    dev->setCallback([&]() -> void
		    {
			receive();
		    });
		}
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

	    int getTransferRate()
	    {
		return testbit(reg_sc, 1) ? 16 : 512;
	    }

	    void fireInterrupt()
	    {
		if (irq_func)
		{
		    irq_func(3);
		}
	    }

	    void sendByte(uint8_t data, bool is_mode)
	    {
		dev->sendByte(data, is_mode);
	    }

	    uint8_t getByte()
	    {
		return dev->getByte();
	    }

	    void receive()
	    {
		reg_sb = getByte();
		fireInterrupt();
		reg_sc &= 0x7F;
		is_pending_receive = false;
	    }

	    void signalTransfer()
	    {
		sendByte(reg_sb, testbit(reg_sc, 0));
		is_pending_receive = true;
	    }
    };
};

#endif // LIBMBGB_SERIAL