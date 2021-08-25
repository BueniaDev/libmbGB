/*
    This file is part of libmbGB.
    Copyright (C) 2021 BueniaDev.

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

#ifndef LIBMBGB_INFRARED
#define LIBMBGB_INFRARED

#include "mmu.h"
#include "libmbgb_api.h"
#include "irdevices.h"
#include <iostream>
#include <functional>
#include <bitset>
using namespace gb;
using namespace std;

namespace gb
{
    class LIBMBGB_API Infrared
    {
	public:
	    Infrared(MMU& memory);
	    ~Infrared();

	    InfraredDevice *dev;

	    void setirdevice(InfraredDevice *cb)
	    {
		dev = cb;
	    }

	    MMU& mem;

	    void init();
	    void shutdown();

	    uint8_t readinfrared(uint16_t addr);
	    void writeinfrared(uint16_t addr, uint8_t val);

	    void updateinfrared();

	    uint8_t infrared_reg = 0xFF;
	    bool ir_signal = false;
	    bool ir_send = false;
    };
};


#endif // LIBMBGB_INFRARED