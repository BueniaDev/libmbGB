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

#include "../../include/libmbGB/mmu.h"
#include <iostream>
using namespace gb;
using namespace std;

namespace gb
{
    uint8_t MMU::mbc1read(uint16_t addr)
    {
	uint8_t temp = 0;

	if ((addr >= 0x4000) && (addr < 0x8000))
	{
	    temp = cartmem[(addr - 0x4000) + (currentrombank * 0x4000)];
	}
	else if ((addr >= 0xA000) && (addr < 0xC000))
	{
	    if (ramenabled)
	    {
		if (!rommode)
		{
		    temp = rambanks[addr - 0xA000];
		}
		else
		{
		    int ramaddr = 0;
		    if (currentrambank != 0)
		    {
			ramaddr = (currentrambank * 0x2000);
		    }

		    temp = rambanks[(addr - 0xA000) + ramaddr];
		}
	    }
	    else
	    {
		temp = 0xFF;
	    }
	}

	return temp;
    }

    void MMU::mbc1write(uint16_t addr, uint8_t value)
    {
	if (addr < 0x2000)
	{
	    if (mbcramsize != 0)
	    {
		if ((value & 0xF) == 0xA)
		{
		    ramenabled = true;
		}
		else if ((value & 0xF) == 0)
		{
		    ramenabled = false;
		}
	    }
	}
	else if (addr < 0x4000)
	{
	    if (!rommode)
	    {    
		currentrombank = (value & 0x1F) | (higherrombankbits << 5);
	    }
	    else
	    {
		currentrombank = (value & 0x1F);
	    }

	    for (uint8_t specialbank : specialrombanks)
	    {
		if (currentrombank == specialbank)
		{
		    currentrombank += 1;
		}
	    }

	    currentrombank &= (numrombanks - 1);
	}
	else if (addr < 0x6000)
	{
	    if (rommode)
	    {
		currentrambank = (value & 0x03);
		currentrambank &= (numrambanks - 1);
	    }
	    else
	    {
		higherrombankbits = (value & 0x3);
		currentrombank = (currentrombank & 0x1F) | (higherrombankbits << 5);

		for (uint8_t specialbank : specialrombanks)
	        {
		    if (currentrombank == specialbank)
		    {
			currentrombank += 1;
		    }
		}

		currentrombank &= (numrombanks - 1);
	    }
	}
	else if (addr < 0x8000)
	{
	    uint8_t data = (value & 0x01);
	    rommode = (data == 0) ? false : true;

	    if (!rommode)
	    {
		currentrambank = 0;
	    }
	}
	else if ((addr >= 0xA000) && (addr < 0xC000))
	{
	    if (ramenabled)
	    {
		if (!rommode)
		{
		    rambanks[addr - 0xA000] = value;
		}
		else
		{
		    uint16_t ramaddr = (currentrambank * 0x2000);
		    rambanks[(addr - 0xA000) + ramaddr] = value;
		}
	    }
	}

	return;
    }

    uint8_t MMU::mbc2read(uint16_t addr)
    {
	uint8_t temp = 0;

	if ((addr >= 0x4000) && (addr < 0x8000))
	{
	    temp = cartmem[(addr - 0x4000) + (currentrombank * 0x4000)];
	}
	else if ((addr >= 0xA000) && (addr < 0xC000))
	{
	    if (ramenabled)
	    {
		temp = rambanks[addr - 0xA000];
	    }
	    else
	    {
		temp = 0xFF;
	    }
	}

	return temp;
    }

    void MMU::mbc2write(uint16_t addr, uint8_t value)
    {
	if (addr < 0x2000)
	{
	    uint8_t upperbyte = (addr >> 8);
	    if (!TestBit(upperbyte, 0))
	    {
		ramenabled = !ramenabled;
	    }
	}
	else if (addr < 0x4000)
	{
	    uint8_t upperbyte = (addr >> 8);
	    if (TestBit(upperbyte, 0))
	    {
		currentrombank = (value & 0x0F);
	    }
	}
	else if ((addr >= 0xA000) && (addr < 0xA200))
	{
	    if (ramenabled)
	    {
		rambanks[addr - 0xA000] = value;
	    }
	}

	return;
    }

    uint8_t MMU::mbc3read(uint16_t addr)
    {
	uint8_t temp = 0;

	if ((addr >= 0x4000) && (addr < 0x8000))
	{
	    temp = cartmem[(addr - 0x4000) + (currentrombank * 0x4000)];
	}
	else if ((addr >= 0xA000) && (addr < 0xC000))
	{
	    if (currentrambank <= 3)
	    {
		uint16_t currentaddr = (currentrambank * 0x2000);
		temp = rambanks[(addr - 0xA000) + currentaddr];
	    }
	    else if (rtcenabled && ((currentrambank >= 0x8) && (currentrambank <= 0xC)))
	    {
		cout << "Reading RTC register..." << endl;
		temp = 0xFF;
	    }
	}
	else
	{
	    temp = 0xFF;
	}

	return temp;
    }

    void MMU::mbc3write(uint16_t addr, uint8_t value)
    {
	if (addr < 0x2000)
	{
	    if (mbcramsize != 0)
	    {
		if ((value & 0xF) == 0xA)
		{
		    ramenabled = true;
		}
		else if ((value & 0xF) == 0)
		{
		    ramenabled = false;
		}

		if (isrtcpres)
		{
		    rtcenabled = ramenabled;
		}
	    }
	}
	else if (addr < 0x4000)
	{
	    currentrombank = (value & 0x7F);

	    if (currentrombank == 0)
	    {
		currentrombank += 1;
	    }

	    currentrombank &= (numrombanks - 1);
	}
	else if (addr < 0x6000)
	{
	    if (value <= 3)
	    {
		currentrambank = value;
		currentrambank &= (numrambanks - 1);
	    }
	    else
	    {
		if (rtcenabled)
		{
		    if ((value >= 0x08) && (value <= 0x0C))
		    {
			cout << "Selecting RTC register " << hex << (int)(value) << endl;
			currentrambank = value;
		    }
		}
	    }
	}
	else if (addr < 0x8000)
	{
	    if (rtcenabled)
	    {
		if ((rtclatch1 == true) && (value == 0))
		{
		    rtclatch1 = false;
		}
		else if ((rtclatch2 == true) && (value == 1))
		{
		    cout << "Latching RTC register..." << endl;
		    rtclatch1 = rtclatch2 = true;
		}
	    }
	}
	else if ((addr >= 0xA000) && (addr < 0xC000))
	{
	    if (currentrambank <= 3)
	    {
		uint16_t currentaddr = (currentrambank * 0x2000);
		rambanks[(addr - 0xA000) + currentaddr] = value;
	    }
	    else if (rtcenabled && ((currentrambank >= 0x8) && (currentrambank <= 0xC)))
	    {
		cout << "Register: " << hex << (int)(currentrambank) << endl;
		cout << "Value: " << hex << (int)(value) << endl;
		cout << endl;
	    }
	}
    }

    uint8_t MMU::mbc5read(uint16_t addr)
    {
	uint8_t temp = 0;

	if ((addr >= 0x4000) && (addr < 0x8000))
	{
	    temp = cartmem[(addr - 0x4000) + (currentrombank * 0x4000)];
	}
	else if ((addr >= 0xA000) && (addr < 0xC000))
	{
	    if (ramenabled && mbcramsize > 0)
	    {
		int ramaddr = 0;
		if (currentrambank != 0)
		{
		    ramaddr = (currentrambank * 0x2000);
		}

		temp = rambanks[(addr - 0xA000) + ramaddr];
	    }
	    else
	    {
		temp = 0x00;
	    }
	}

	return temp;
    }

    void MMU::mbc5write(uint16_t addr, uint8_t value)
    {
	if (addr < 0x2000)
	{
	    if (mbcramsize != 0)
	    {
		if ((value & 0xF) == 0xA)
		{
		    ramenabled = true;
		}
		else if ((value & 0xF) == 0)
		{
		    ramenabled = false;
		}
	    }
	}
	else if (addr < 0x3000)
	{
	    currentrombank = ((currentrombank & 0x100) | value);
	    currentrombank &= (numrombanks - 1);
	}
	else if (addr < 0x4000)
	{
	    currentrombank = ((currentrombank & 0xFF) | ((value & 0x1) << 9));
	    currentrombank &= (numrombanks - 1);
	}
	else if (addr < 0x6000)
	{
	    currentrambank = (isrumblepres) ? (value & 0x07) : (value & 0x0F);
	    currentrambank &= (numrambanks - 1);

	    if (isrumblepres == true)
	    {
		if (TestBit(value, 3))
		{
		    if (setrumble)
		    {
		        setrumble(true);
		    }
		}
		else
		{
		    if (setrumble)
		    {
		        setrumble(false);
		    }
		}
	    }
	}
	else if ((addr >= 0xA000) && (addr < 0xC000))
	{
	    if (ramenabled && mbcramsize > 0)
	    {
		uint16_t ramaddr = (currentrambank * 0x2000);
		rambanks[(addr - 0xA000) + ramaddr] = value;
	    }
	}

	return;
    }

    uint8_t MMU::mbc7read(uint16_t addr)
    {
	uint8_t temp = 0;

	if ((addr >= 0x4000) && (addr < 0x8000))
	{
	    temp = cartmem[(addr - 0x4000) + (currentrombank * 0x4000)];
	}
	else if ((addr >= 0xA000) && (addr < 0xC000))
	{
	    if (ramsecenabled)
	    {
		temp = readmbc7ram(addr);
	    }
	    else
	    {
		temp = 0xFF;
	    }
	}

	return temp;
    }

    void MMU::mbc7write(uint16_t addr, uint8_t val)
    {
	if (addr < 0x2000)
	{
	    if ((val & 0xF) == 0xA)
	    {
		ramenabled = true;
	    }
	    else if ((val & 0xF) == 0)
	    {
		ramenabled = false;
	    }
	}
	else if (addr < 0x4000)
	{
	    currentrombank = (val & 0x7F);
	    currentrombank &= (numrombanks - 1);
	}
	else if (addr < 0x6000)
	{
	    if ((ramenabled == true) && (val == 0x40))
	    {
		ramsecenabled = true;
	    }
	    else
	    {
		ramsecenabled = false;
	    }
	}
	else if ((addr >= 0xA000) && (addr < 0xC000))
	{
	    if (ramsecenabled)
	    {
		writembc7ram(addr, val);
	    }
	    return;
	}
    }

    uint8_t MMU::readmbc7ram(uint16_t addr)
    {
	uint8_t temp = 0;

	if ((addr >= 0xA000) && (addr < 0xB000))
	{
	    int tempaddr = ((addr >> 4) & 0xF);

	    switch (tempaddr)
	    {
		case 0x2: temp = (mbc7sensorx & 0xFF); break;
		case 0x3: temp = (mbc7sensorx >> 8); break;
		case 0x4: temp = (mbc7sensory & 0xFF); break;
		case 0x5: temp = (mbc7sensory >> 8); break;
		case 0x6: temp = 0x00; break;
		case 0x7: temp = 0xFF; break;
		case 0x8: temp = mbc7intvalue; break;
		default: temp = 0x00; break;
	    }

	    return temp;
	}	
	else
	{
	    return 0xFF;
	}
    }

    void MMU::writembc7ram(uint16_t addr, uint8_t val)
    {
	if ((addr >= 0xA000) && (addr < 0xB000))
	{
	    int tempaddr = ((addr >> 4) & 0xF);

	    switch (tempaddr)
	    {
		case 0x0:
		{
		    if (val == 0x55)
		    {
			mbc7sensorx = 0x8000;
			mbc7sensory = 0x8000;
		    }
		}
		break;
		case 0x1:
		{
		    if (val == 0xAA)
		    {
			setsensor(mbc7sensorx, mbc7sensory);
		    }
		}
		break;
		case 0x8: writembc7eeprom(val); break;
		default: return; break;
	    }

	    return;
	}	
	else
	{
	    return;
	}
    }

    void MMU::writembc7eeprom(uint8_t val)
    {
	bool oldchipsel = mbc7chipsel;
	bool oldchipclk = mbc7chipclk;
	mbc7chipsel = TestBit(val, 7);
	mbc7chipclk = TestBit(val, 6);

	if (!oldchipsel && mbc7chipsel)
	{
	    if (mbc7intstate == 5)
	    {
		if (mbc7wenable)
		{
		    uint16_t tempaddr = ((mbc7chipaddr & 0x7F) << 1);
		    rambanks[tempaddr] = (mbc7chipbuf >> 8);
		    rambanks[(tempaddr + 1)] = (mbc7chipbuf & 0xFF);
		}

		mbc7intstate = 0;
		mbc7intvalue = 1;
	    }
	    else
	    {
		mbc7idle = true;
		mbc7intstate = 0;
	    }
	}

	if (!oldchipclk && mbc7chipclk)
	{
	    if (mbc7idle && TestBit(val, 1))
	    {
		mbc7idle = false;
		mbc7intstate = 1;
	    }
	    else
	    {
		switch (mbc7intstate)
		{
		    case 1:
		    {
			mbc7chipbuf <<= 1;
			mbc7chipbuf |= TestBit(val, 1);
			mbc7chipsize += 1;

			if (mbc7chipsize == 2)
			{
			    mbc7chipcmd = (mbc7chipbuf & 0x3);
			    mbc7chipsize = 0;
			    mbc7intstate = 2;
			}
		    }
		    break;
		    case 2:
		    {
			mbc7chipbuf <<= 1;
			mbc7chipbuf |= TestBit(val, 1);
			mbc7chipsize += 1;

			if (mbc7chipsize == 8)
			{
			    mbc7chipaddr = (mbc7chipbuf & 0xFF);
			    mbc7chipsize = 0;
			    mbc7intstate = 3;

			    if (mbc7chipcmd == 0)
			    {
				if ((mbc7chipaddr >> 6) == 0)
				{
				    mbc7wenable = false;
				    mbc7intstate = 0;
				}
				else if ((mbc7chipaddr >> 6) == 3)
				{
				    mbc7wenable = true;
				    mbc7intstate = 0;
				}
			    }
			}
		    }
		    break;
		    case 3:
		    {
			mbc7chipbuf <<= 1;
			mbc7chipbuf |= TestBit(val, 1);
			mbc7chipsize += 1;

			switch (mbc7chipcmd)
			{
			    case 0:
			    {
				if (mbc7chipsize == 16)
				{

				    switch ((mbc7chipaddr >> 6))
				    {
					case 0:
					{
					    mbc7wenable = false;
					    mbc7intstate = 0;
					}
					break;
					case 1:
					{
					    if (mbc7wenable)
					    {
						for (int i = 0; i < 256; i++)
						{
						    rambanks[(i << 1)] = (mbc7chipbuf >> 8);
						    rambanks[((i << 1) + 1)] = (mbc7chipbuf & 0xFF);
						}
					    }

					    mbc7intstate = 5;
					}
					break;
					case 2:
					{
					    if (mbc7wenable)
					    {
						for (int i = 0; i < 256; i++)
						{
						    rambanks[(i << 1)] = 0xFF;
						    rambanks[((i << 1) + 1)] = 0xFF;
						}
					    }

					    mbc7intstate = 5;
					}
					break;
					case 3:
					{
					    mbc7wenable = true;
					    mbc7intstate = 0;
					}
					break;
				    }

				    mbc7chipsize = 0;
				}
			    }
			    break;
			    case 1:
			    {
				if (mbc7chipsize == 16)
				{
				    mbc7chipsize = 0;
				    mbc7intstate = 5;
				    mbc7intvalue = 0;
				}
			    }
			    break;
			    case 2:
			    {
				if (mbc7chipsize == 1)
				{
				    uint16_t tempaddr = ((mbc7chipaddr & 0x7F) << 1);
				    mbc7chipbuf = ((rambanks[tempaddr] << 8) | (rambanks[(tempaddr + 1)]));
				    mbc7intstate = 4;
				    mbc7chipsize = 0;
				}
			    }
			    break;
			    default: cout << "Unrecognized EEPROM command of " << dec << (int)(mbc7chipcmd) << endl; exit(1); break;
			}
		    }
		    break;
		}
	    }
	}

	if (oldchipclk && !mbc7chipclk)
	{
	    if (mbc7intstate == 4)
	    {
		mbc7intvalue = (int)(TestBit(mbc7chipbuf, 15));
		mbc7chipbuf <<= 1;
		mbc7chipsize += 1;

		if (mbc7chipsize == 16)
		{
		    mbc7chipsize = 0;
		    mbc7intstate = 0;
		}
	    }
	}
    }
};
