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
	if (ismulticart)
	{
	    return mbc1mread(addr);
	}
	else
	{
	    return mbc1rread(addr);
	}
    }

    void MMU::mbc1write(uint16_t addr, uint8_t value)
    {
	if (ismulticart)
	{
	    mbc1mwrite(addr, value);
	}
	else
	{
	    mbc1rwrite(addr, value);
	}
    }

    uint8_t MMU::mbc1rread(uint16_t addr)
    {
	uint8_t temp = 0;

	if ((addr >= 0x4000) && (addr < 0x8000))
	{
	    temp = cartmem[(addr - 0x4000) + (currentrombank * 0x4000)];
	}
	else if ((addr >= 0xA000) && (addr < 0xC000))
	{
	    temp = rambanks[(addr - 0xA000) + (currentrambank * 0x2000)];
	}

	return temp;
    }

    void MMU::mbc1rwrite(uint16_t addr, uint8_t value)
    {
	if (addr < 0x2000)
	{
	    if (mbcramsize != 0)
	    {
		if ((value & 0xA) == 0xA)
		{
		    ramenabled = true;
		}
		else if ((value & 0xA) == 0x0)
		{
		    ramenabled = false;
		}
	    }
	}
	else if (addr < 0x4000)
	{
	    if (rommode)
	    {
		currentrombank = ((higherrombankbits << 5) | (value & 0x1F));
	    }
	    else
	    {
		currentrombank = (value & 0x1F);
	    }

	    for (int i = 0; i < 4; i++)
	    {
		if (currentrombank == specialrombanks[i])
		{
		    currentrombank += 1;
		    return;
		}
	    }
	}
	else if (addr < 0x6000)
	{
	    if (rommode)
	    {
		higherrombankbits = (value & 0x3);
	    }
	    else
	    {
		currentrambank = (value & 0x3);
	    }
	}
	else if (addr < 0x8000)
	{
	    if (value == 0)
	    {
		rommode = true;
	    }
	    else if (value == 1)
	    {
		rommode = false;
	    }
	}
	else if ((addr >= 0xA000) && (addr < 0xC000))
	{
	    if (ramenabled)
	    {
		rambanks[(addr - 0xA000) + (currentrambank * 0x2000)] = value;
	    }
	}
    }

    uint8_t MMU::mbc1mread(uint16_t addr)
    {
	uint8_t temp = 0;

	if (addr < 0x4000)
	{
	    if (!rommode)
	    {
		temp = rom[addr];
	    }
	    else
	    {
		int tempaddr = (higherrombankbits << 4);
		temp = cartmem[addr + (tempaddr * 0x4000)];
	    }
	}
	else if (addr < 0x8000)
	{
	    temp = cartmem[(addr - 0x4000) + (currentrombank * 0x4000)];
	}
	else if ((addr >= 0xA000) && (addr < 0xC000))
	{
	    if (ramenabled)
	    {
		temp = rambanks[(addr - 0xA000)];
	    }
	    else
	    {
		temp = 0x00;
	    }
	}

	return temp;
    }

    void MMU::mbc1mwrite(uint16_t addr, uint8_t value)
    {
	if (addr < 0x2000)
	{
	    if (mbcramsize != 0)
	    {
		if ((value & 0xA) == 0xA)
		{
		    ramenabled = true;
		}
		else if ((value & 0xA) == 0x0)
		{
		    ramenabled = false;
		}
	    }
	}
	else if (addr < 0x4000)
	{
	    currentrombank = ((higherrombankbits << 4) | (value & 0xF));
	}
	else if (addr < 0x6000)
	{
	    higherrombankbits = (value & 0x3);
	}
	else if (addr < 0x8000)
	{
	    rommode = TestBit(value, 0);
	}
	else if ((addr >= 0xA000) && (addr < 0xC000))
	{
	    if (ramenabled)
	    {
		rambanks[(addr - 0xA000)] = value;
	    }
	}
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
		switch (currentrambank)
		{
		    case 0x8: temp = realsecs; break;
		    case 0x9: temp = realmins; break;
		    case 0xA: temp = realhours; break;
		    case 0xB: temp = realdays; break;
		    case 0xC: temp = realdayshi; break;
		}
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
		    latchtimer();
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
		updatetimer();

		switch (currentrambank)
		{
		    case 0x8: realsecs = value; break;
		    case 0x9: realmins = value; break;
		    case 0xA: realhours = value; break;
		    case 0xB: realdays = value; break;
		    case 0xC: realdayshi = value; break;
		}
	    }
	}
    }

    void MMU::updatetimer()
    {
	time_t newtime = time(NULL);
	unsigned int difference = 0;

	if ((newtime > currenttime) && !TestBit(realdayshi, 6))
	{
	    difference = (unsigned int)(newtime - currenttime);
	    currenttime = newtime;
	}
	else
	{
	    currenttime = newtime;
	    return;
	}

	unsigned int newsecs = (realsecs + difference);

	if (newsecs == realsecs)
	{
	    return;
	}

	realsecs = (newsecs % 60);

	unsigned int newmins = (realmins + (newsecs / 60));

	if (newmins == realmins)
	{
	    return;
	}

	realmins = (newmins % 60);

	unsigned int newhours = (realhours + (newmins / 60));

	if (newhours == realhours)
	{
	    return;
	}

	realhours = (newhours % 24);

	unsigned int realdaysunsplit = ((TestBit(realdayshi, 0) << 8) | realdays);
	unsigned int newdays = (realdaysunsplit + (newhours / 24));

	if (newdays == realdaysunsplit)
	{
	    return;
	}

	realdays = newdays;
	realdayshi &= 0xFE;
	realdayshi |= TestBit(newdays, 8);

	if (newdays > 511)
	{
	    realdayshi = BitSet(realdayshi, 7);
	}
    }

    void MMU::latchtimer()
    {
	updatetimer();
	latchsecs = realsecs;
	latchmins = realmins;
	latchhours = realhours;
	latchdays = realdays;
	latchdayshi = realdayshi;
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
			    case 3:
			    {
				if (mbc7chipsize == 16)
				{
				    mbc7chipsize = 0;
				    mbc7chipbuf = 0xFFFF;
				    mbc7intstate = 5;
				    mbc7intvalue = 0;
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

    uint8_t MMU::wisdomtreeread(uint16_t addr)
    {
	uint8_t temp = 0;

	if (addr < 0x8000)
	{
	    temp = cartmem[(addr + (wisdomrombank * 0x4000))];
	}
	else if ((addr >= 0xA000) && (addr < 0xC000))
	{
	    temp = 0xFF;
	}

	return temp;
    }

    void MMU::wisdomtreewrite(uint16_t addr, uint8_t value)
    {
	if (addr < 0x4000)
	{
	    int bank = (addr & 0x3F);
	    wisdomrombank = (bank << 1);
	}
    }
};
