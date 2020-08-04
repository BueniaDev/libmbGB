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

#ifndef LIBMBGB_ADDONS
#define LIBMBGB_ADDONS

#include "libmbgb_api.h"
#include "gpu.h" // This is for the custom RGB data type
#include <iostream>
#include <functional>
#include <bitset>
#include <cmath> // This is required for ceil, which is used to calculate the printout's height
#include <ctime>
using namespace gb;
using namespace std;

namespace gb
{
    using linkfunc = function<void(uint8_t)>;
    
    class LIBMBGB_API SerialDevice
    {
        public:
            SerialDevice();
            ~SerialDevice();
            
            virtual void setlinkcallback(linkfunc cb) = 0;
            virtual void deviceready(uint8_t byte, bool ismode) = 0;
            virtual void update() = 0;
            virtual void transfer() = 0;
            virtual void swipebarcode() = 0;
            virtual bool swipedcard() = 0; // This is required for proper emulation of the BTB and the Barcode Boy
            virtual int serialcycles() = 0;
    };
    
    class LIBMBGB_API Disconnected : public SerialDevice
    {
        public:
            Disconnected();
            ~Disconnected();
            
            linkfunc dislink;
            
            void setlinkcallback(linkfunc cb)
            {
                dislink = cb;
            }
            
            void deviceready(uint8_t byte, bool ismode)
            {
            	if (ismode)
            	{
            	    update();
            	}
            }
            
            void update()
            {
            	transfer();
            }
            
            void transfer()
            {
            	if (dislink)
            	{
            	    dislink(0xFF);
            	}
            }
            
            void swipebarcode()
            {
            	return;
            }
            
            bool swipedcard()
            {
                return false;
            }
            
            int serialcycles()
            {
                return 0;
            }
    };
    
    class LIBMBGB_API BarcodeBoy : public SerialDevice
    {
    	public:
    	    BarcodeBoy();
    	    ~BarcodeBoy();
    	    
    	    uint8_t recbyte = 0;
    	    uint8_t linkbyte = 0;
    	    
    	    linkfunc powerlink;
    	    
    	    bool barcodeswiped = false;
    	    
    	    void setlinkcallback(linkfunc cb)
    	    {
    	    	powerlink = cb;
    	    }
    	    
    	    void swipebarcode()
    	    {
    	    	if (state == BCBState::Active)
    	    	{
    	    	    state = BCBState::SendBarcode;
    	    	    barcodeswiped = true;
    	    	}
    	    }
    	    
    	    void update();
    	    void processbyte();
    	    
    	    enum BCBState : int
    	    {
    	        Inactive = 0,
    	        Active = 1,
    	        SendBarcode = 2,
    	        Finished = 3,
    	    };
    	    
    	    BCBState state = BCBState::Inactive;
    	    
    	    array<uint8_t, 13> testcode = {0x34, 0x39, 0x30, 0x32, 0x37, 0x37, 0x36, 0x38, 0x30, 0x39, 0x33, 0x36, 0x37};
    	    
    	    int bcbcounter = 0;
    	    
    	    void transfer()
    	    {
    	        if (powerlink)
    	        {
    	            powerlink(linkbyte);
    	        }
    	    }
    	    
    	    void deviceready(uint8_t byte, bool ismode)
    	    {
    	        if (ismode)
    	        {
    	            recbyte = byte;
    	            update();
    	        }
    	        else if (barcodeswiped)
    	        {
    	            update();
    	        }
    	    }
    	    
    	    bool swipedcard()
    	    {
    	    	return barcodeswiped;
    	    }
    	    
    	    int serialcycles()
    	    {
    	        return 0;
    	    }
    };
};

#endif // LIBMBGB_ADDONS
