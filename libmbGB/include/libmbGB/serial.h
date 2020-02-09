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

#ifndef LIBMBGB_SERIAL
#define LIBMBGB_SERIAL

#include "mmu.h"
#include "libmbgb_api.h"
#include "gpu.h" // This is for the custom RGB data type
#include "addons.h"
#include <iostream>
#include <functional>
#include <bitset>
#include <cmath> // This is required for ceil, which is used to calculate the printout's height
using namespace gb;
using namespace std;

namespace gb
{
    using serialfunc = function<void(uint8_t, bool)>;

    class LIBMBGB_API Serial
    {
	public:
	    Serial(MMU& memory);
	    ~Serial();

	    void init();
	    void shutdown();

	    MMU& serialmem;
		
		serialfunc linkready;

		bool dump = false;
			
		void setlinkcallback(serialfunc cb)
		{
			linkready = cb;
		}

		void signalready()
		{
		    if (linkready)
		    {
			dump = true;
			linkready(bytetotransfer, TestBit(sc, 0));
			pendingrecieve = true;
		    }
		}

		void disconnectedready(uint8_t unused1, bool unused2)
		{
		    return;
		}

		void recieve(uint8_t byte)
		{
		    bytetorecieve = byte;
		    serialmem.requestinterrupt(3);
		    sc &= 0x7F;
		    pendingrecieve = false;
		}

		void disrecieve(uint8_t byte)
		{
		    return;
		}
		
		uint8_t bytetotransfer = 0;
		uint8_t bytetorecieve = 0;
		
		uint8_t sb = 0;
		uint8_t sc = 0;
		
		uint8_t readserial(uint16_t addr);
		void writeserial(uint16_t addr, uint8_t val);

	    int serialclock = 0;
	    int shiftcounter = 0;

	    void updateserial();

	    bool pendingrecieve = false;

	    inline void initserialclock(uint8_t initval)
	    {
		serialclock = initval;
	    }
    };
};

#endif // LIBMBGB_SERIAL
