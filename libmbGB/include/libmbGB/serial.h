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
    class LIBMBGB_API Serial
    {
	public:
	    Serial(MMU& memory);
	    ~Serial();

	    void init();
	    void shutdown();

	    MMU& serialmem;
	    
	    SerialDevice *dev;
	    
	    void setserialdevice(SerialDevice *cb)
	    {
	        dev = cb;
	    }
	    
	    bool dump = false;
	    
	    void recieve(uint8_t byte)
	    {
	        bytetorecieve = byte;
	        serialmem.requestinterrupt(3);
	        serialcontrol &= 0x7F;
	        pendingrecieve = false;
	    }
	    
	    void signalready()
	    {
	        if (dev != NULL)
	        {
	            dev->deviceready(bytetotransfer, TestBit(serialcontrol, 0));
	            pendingrecieve = true;
	        }
	        else
	        {
	            cout << "Null pointer" << endl;
	        }
	    }
	    
	    bool swipedcard()
	    {
	    	return (dev != NULL) ? dev->swipedcard() : false;
	    }
	    
	    int serialcycles()
	    {
	        return (dev != NULL) ? dev->serialcycles() : 0;
	    }
		
	    uint8_t bytetorecieve = 0;
	    uint8_t bytetotransfer = 0;
	    uint8_t serialcontrol = 0;
	    
	    bool pendingrecieve = false;
		
	    uint8_t readserial(uint16_t addr);
	    void writeserial(uint16_t addr, uint8_t val);

	    int serialtimestamp = 0;
	    int serialclock = 0;
	    int shiftcounter = 0;

	    void updateserial();

	    inline void initserialclock(uint8_t initval)
	    {
		serialclock = initval;
	    }
	    
	    int gettransferrate()
	    {
	    	int temp = 0;
	    	
	    	if (serialcycles() != 0)
	    	{
	    	    temp = serialcycles();
	    	}
	    	else
	    	{
	    	    temp = ((TestBit(serialcontrol, 1) ? 16 : 512) >> serialmem.doublespeed);
	    	}

	    	return temp;
	    }
    };
};

#endif // LIBMBGB_SERIAL
