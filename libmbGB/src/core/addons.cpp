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

#include "../../include/libmbGB/addons.h"
using namespace gb;

namespace gb
{
    SerialDevice::SerialDevice()
    {
    
    }
    
    SerialDevice::~SerialDevice()
    {
    
    }
    
    Disconnected::Disconnected()
    {
    
    }
    
    Disconnected::~Disconnected()
    {
    
    }
    
    BarcodeBoy::BarcodeBoy()
    {
    
    }
    
    BarcodeBoy::~BarcodeBoy()
    {
    
    }
    
    void BarcodeBoy::update()
    {
    	processbyte();
    	transfer();
    }
    
    void BarcodeBoy::processbyte()
    {
        if ((state != BCBState::Inactive) && ((recbyte == 0x10) || (recbyte == 0x07)))
        {
            if (state == BCBState::Active)
            {
                linkbyte = 0xFF;
                return;
            }
            else if (state == BCBState::Finished)
            {
                state = BCBState::Inactive;
                bcbcounter = 0;
            }
        }
    
    	switch (state)
    	{
    	    case BCBState::Inactive:
    	    {
    	    	if (bcbcounter < 2)
    	    	{
    	    	    linkbyte = 0xFF;
    	    	    bcbcounter += 1;
    	    	}
    	    	else if (bcbcounter < 4)
    	    	{
    	    	    if ((recbyte == 0x10) && (bcbcounter == 2))
    	    	    {
    	    	        linkbyte = 0x10;
    	    	        bcbcounter += 1;
    	    	    }
    	    	    else if ((recbyte == 0x7) && (bcbcounter == 3))
    	    	    {
    	    	        linkbyte = 0x07;
    	    	        bcbcounter += 1;
    	    	    }
    	    	}
    	    	
    	    	if (bcbcounter == 4)
    	    	{
    	    	    bcbcounter = 0;
    	    	    state = BCBState::Active;
    	    	}
    	    }
    	    break;
    	    case BCBState::SendBarcode:
    	    {
    	    	if (bcbcounter == 0)
    	    	{
    	    	    linkbyte = 0x2;
    	    	    bcbcounter += 1;
    	    	}
    	    	else if (bcbcounter < 14)
    	    	{
    	    	    linkbyte = testcode[(bcbcounter - 1)];
    	    	    bcbcounter += 1;
    	    	}
    	    	else if (bcbcounter == 14)
    	    	{
    	    	    linkbyte = 0x3;
    	    	    bcbcounter += 1;
    	    	}
    	    	else if (bcbcounter == 15)
    	    	{
    	    	    linkbyte = 0x2;
    	    	    bcbcounter += 1;
    	    	}
    	    	else if (bcbcounter < 29)
    	    	{
    	    	    linkbyte = testcode[(bcbcounter - 16)];
    	    	    bcbcounter += 1;
    	    	}
    	    	else if (bcbcounter == 29)
    	    	{
    	    	    linkbyte = 0x3;
    	    	    bcbcounter = 0;
    	    	    barcodeswiped = false;
    	    	    state = BCBState::Finished;
    	    	}
    	    }
    	    break;
    	}
    }
}
