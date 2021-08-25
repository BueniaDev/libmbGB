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

#include "mmu.h"
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
	if (addr < 0x4000)
	{
	    temp = rom[addr];
	}
	else if (addr < 0x8000)
	{
	    temp = cartmem[(addr - 0x4000) + (currentrombank * 0x4000)];
	}
	else if ((addr >= 0xA000) && (addr < 0xC000))
	{
	    if (ramenabled)
	    {
	        temp = rambanks[(addr - 0xA000) + (currentrambank * 0x2000)];
	    }
	    else
	    {
		temp = 0x00;
	    }
	}

	return temp;
    }

    void MMU::mbc1rwrite(uint16_t addr, uint8_t value)
    {
	if (addr < 0x2000)
	{
	    if (externalrampres)
	    {
		ramenabled = ((value & 0xF) == 0xA);
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
	    if (externalrampres)
	    {
		ramenabled = ((value & 0xF) == 0xA);
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
	    rommode = testbit(value, 0);
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

	if (addr < 0x4000)
	{
	    temp = rom[addr];
	}
	else if (addr < 0x8000)
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
	    if (!testbit(upperbyte, 0))
	    {
		ramenabled = !ramenabled;
	    }
	}
	else if (addr < 0x4000)
	{
	    uint8_t upperbyte = (addr >> 8);
	    if (testbit(upperbyte, 0))
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
	if (addr < 0x4000)
	{
	    temp = rom[addr];
	}
	else if (addr < 0x8000)
	{
	    temp = cartmem[(addr - 0x4000) + (currentrombank * 0x4000)];
	}
	else if ((addr >= 0xA000) && (addr < 0xC000))
	{
	    if (currentrambank < 0x8)
	    {
		if (ramenabled)
		{
		    temp = rambanks[(addr - 0xA000) + (currentrambank * 0x2000)];
		}
		else
		{
		    temp = 0x00;
		}
	    }
	    else if (currentrambank <= 0xC)
	    {
		if (rtcenabled)
		{
		    switch (currentrambank)
		    {
			case 0x8: temp = latchsecs; break;
			case 0x9: temp = latchmins; break;
			case 0xA: temp = latchhours; break;
			case 0xB: temp = (latchdays & 0xFF); break;
			case 0xC: temp = ((latchdays >> 8) | (latchhalt << 6) | (latchdayscarry << 7)); break;
		    }
		}
		else
		{
		    temp = 0x00;
		}
	    }
	    else
	    {
		temp = 0x00;
	    }
	}

	return temp;
    }

    void MMU::mbc3write(uint16_t addr, uint8_t value)
    {
	if (addr < 0x2000)
	{
	    bool isramrtcenabled = ((value & 0xF) == 0xA);
	    ramenabled = (externalrampres && isramrtcenabled);
	    rtcenabled = (isrtcpres && isramrtcenabled);
	}
	else if (addr < 0x4000)
	{
	    currentrombank = (value & 0x7F);

	    if (currentrombank == 0)
	    {
		currentrombank = 1;
	    }

	    currentrombank &= (numrombanks - 1);
	}
	else if (addr < 0x6000)
	{
	    currentrambank = value;

	    if (!isrtcpres || (value < 0x8))
	    {
		currentrambank &= (numrambanks - 1);
	    }
	}
	else if (addr < 0x8000)
	{
	    if (rtcenabled == true)
	    {
		if ((value == 0) && !rtclatch)
		{
		    rtclatch = true;
		}
		else if ((value == 1) && rtclatch)
		{
		    latchtimer();
		    rtclatch = false;
		}
	    }
	}
	else if ((addr >= 0xA000) && (addr < 0xC000))
	{
	    if (ramenabled && (currentrambank < 0x8))
	    {
		rambanks[(addr - 0xA000) + (currentrambank * 0x2000)] = value;
	    }
	    else if (rtcenabled && ((currentrambank >= 0x8) && (currentrambank <= 0xC)))
	    {
		switch (currentrambank)
		{
		    case 0x8:
		    {
			timercounter = 0;
			realsecs = (value & 0x3F);
		    }
		    break;
		    case 0x9: realmins = (value & 0x3F); break;
		    case 0xA: realhours = (value & 0x1F); break;
		    case 0xB: realdays = ((realdays & 0x100) | value); break;
		    case 0xC:
		    {
			realdays = ((BitGetVal(value, 0) << 8) | (realdays & 0xFF));
			rtchalt = testbit(value, 6);
			realdayscarry = testbit(value, 7);
		    }
		    break;
		}
	    }
	}
    }

    void MMU::latchtimer()
    {
	latchsecs = realsecs;
	latchmins = realmins;
	latchhours = realhours;
	latchdays = realdays;
	latchhalt = rtchalt;
	latchdayscarry = realdayscarry;
    }

    void MMU::updatetimer()
    {
	if (rtchalt)
	{
	    return;
	}

	timercounter += 4;

	if (timercounter == 4194304)
	{
	    timercounter = 0;
	    realsecs = ((realsecs + 1) & 63);

	    if (realsecs == 60)
	    {
		realsecs = 0;
		realmins = ((realmins + 1) & 63);

		if (realmins == 60)
		{
		    realmins = 0;
		    realhours = ((realhours + 1) & 31);

		    if (realhours == 24)
		    {
			realhours = 0;
			realdays += 1;

			if (realdays == 512)
			{
			    realdays = 0;
			    realdayscarry = true;
			}
		    }
		}
	    }
	}
    }

    void MMU::inittimer()
    {
	time_t newtime = time(NULL);
	unsigned int difference = 0;

	// Set the Unix timestamp equal to the system time if it's set to 0
	// This helps prevent absurd timestamp differences from triggering
	// time-based save RAM errors in the mainline Gen 2 Pokemon games
	if (current_time <= 0)
	{
	    current_time = time(NULL);
	}

	// Check if there's a difference between the currently-saved timestamp
	// and the current system time
	if ((newtime > current_time) && !rtchalt)
	{
	    difference = (unsigned int)(newtime - current_time);
	    current_time = newtime;
	}
	else
	{
	    current_time = newtime;
	}

	// Update the RTC registers accordingly

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

	unsigned int newdays = (realdays + (newhours / 24));

	if (newdays == realdays)
	{
	    return;
	}

	realdays = newdays;

	realdays &= 0xFEFF;
	realdays |= BitGetVal(newdays, 8);

	if (newdays > 511)
	{
	    realdayscarry = true;
	}
    }

    uint8_t MMU::mbc5read(uint16_t addr)
    {
	uint8_t temp = 0;

	if (addr < 0x4000)
	{
	    temp = rom[addr];
	}
	else if (addr < 0x8000)
	{
	    temp = cartmem[(addr - 0x4000) + (currentrombank * 0x4000)];
	}
	else if ((addr >= 0xA000) && (addr < 0xC000))
	{
	    if (ramenabled)
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
	    if (externalrampres)
	    {
		ramenabled = ((value & 0xF) == 0xA);
	    }
	}
	else if (addr < 0x3000)
	{
	    currentrombank = ((currentrombank & 0x300) | value);
	}
	else if (addr < 0x4000)
	{
	    currentrombank = ((currentrombank & 0xFF) | ((value & 0x3) << 8));
	    currentrombank &= (numrombanks - 1);
	}
	else if (addr < 0x6000)
	{
	    currentrambank = (isrumblepres) ? (value & 0x07) : (value & 0x0F);
	    currentrambank &= (numrambanks - 1);

	    if (isrumblepres == true)
	    {
		if (setrumble)
		{
		    setrumble(testbit(value, 3));
		}
	    }
	}
	else if ((addr >= 0xA000) && (addr < 0xC000))
	{
	    if (ramenabled)
	    {
		uint16_t ramaddr = (currentrambank * 0x2000);
		rambanks[(addr - 0xA000) + ramaddr] = value;
	    }
	}

	return;
    }

    // MBC6 mapper (used by Net de Get) (WIP implementation)
    //
    // Note: Since the effective number of ROM and RAM banks are twice what
    // they usually would be, the size of each of those banks
    // is only half of the size that the banks would be
    // on other mappers
    //
    // TODO: This mapper uses custom flash ROM (by Macronix), which hasn't been
    // implemented in this mapper yet. However, it could be implemented in the
    // near future.

    uint8_t MMU::mbc6read(uint16_t addr)
    {
	uint8_t temp = 0x00;

	if (addr < 0x4000)
	{
	    temp = rom[addr];
	}
	else if (addr < 0x6000)
	{
	    if (!mbc6bankaflash)
	    {
		temp = cartmem[(addr - 0x4000) + (mbc6rombanka * 0x2000)];
	    }
	    else
	    {
		// TODO: Implement Flash reading
		temp = 0x00;
	    }
	}
	else if (addr < 0x8000)
	{
	    if (!mbc6bankbflash)
	    {
		temp = cartmem[(addr - 0x6000) + (mbc6rombankb * 0x2000)];
	    }
	    else
	    {
		// TODO: Implement Flash reading
		temp = 0x00;
	    }
	}
	else if ((addr >= 0xA000) && (addr < 0xB000))
	{
	    if (ramenabled)
	    {
		uint16_t ramaddr = (mbc6rambanka * 0x1000);
		temp = rambanks[(addr - 0xA000) + ramaddr];
	    }
	    else
	    {
		temp = 0x00;
	    }
	}
	else if ((addr >= 0xB000) && (addr < 0xC000))
	{
	    if (ramenabled)
	    {
		uint16_t ramaddr = (mbc6rambankb * 0x1000);
		temp = rambanks[(addr - 0xB000) + ramaddr];
	    }
	    else
	    {
		temp = 0x00;
	    }
	}

	return temp;
    }

    void MMU::mbc6write(uint16_t addr, uint8_t val)
    {
	if (addr < 0x400)
	{
	    if (externalrampres)
	    {
		ramenabled = ((val & 0x0F) == 0x0A);
	    }
	}
	else if (addr < 0x800)
	{
	    mbc6rambanka = (val & 0x07);
	}
	else if (addr < 0xC00)
	{
	    mbc6rambankb = (val & 0x07);
	}
	else if (addr < 0x1000)
	{
	    if (mbc6flashwriteenable)
	    {
		mbc6flashenable = testbit(val, 0);
	    }
	    else
	    {
		mbc6flashenable = false;
	    }
	}
	else if (addr < 0x2000)
	{
	    mbc6flashwriteenable = testbit(val, 0);
	}
	else if (addr < 0x2800)
	{
	    mbc6rombanka = (val & 0x7F);
	}
	else if (addr < 0x3000)
	{
	    if (val == 0x00)
	    {
		mbc6bankaflash = false;
	    }
	    else if (val == 0x08)
	    {
		cout << "Selecting Flash for bank A..." << endl;
		mbc6bankaflash = true;
	    }
	}
	else if (addr < 0x3800)
	{
	    mbc6rombankb = (val & 0x7F);
	}
	else if (addr < 0x4000)
	{
	    if (val == 0x00)
	    {
		mbc6bankbflash = false;
	    }
	    else if (val == 0x08)
	    {
		cout << "Selecting Flash for bank B..." << endl;
		mbc6bankbflash = true;
	    }
	}
	else if (addr < 0x6000)
	{
	    cout << "Unimplemented: ROM/Flash bank A writing" << endl;
	    exit(1);
	}
	else if (addr < 0x8000)
	{
	    cout << "Unimplemented: ROM/Flash bank B writing" << endl;
	    exit(1);
	}
	else if ((addr >= 0xA000) && (addr < 0xB000))
	{
	    if (ramenabled)
	    {
		uint16_t ramaddr = (mbc6rambanka * 0x1000);
		rambanks[(addr - 0xA000) + ramaddr] = val;
	    }
	}
	else if ((addr >= 0xB000) && (addr < 0xC000))
	{
	    if (ramenabled)
	    {
		uint16_t ramaddr = (mbc6rambankb * 0x1000);
		rambanks[(addr - 0xB000) + ramaddr] = val;
	    }
	}

	return;
    }

    uint8_t MMU::mbc7read(uint16_t addr)
    {
	uint8_t temp = 0;

	if (addr < 0x4000)
	{
	    temp = rom[addr];
	}
	else if (addr < 0x8000)
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
	    ramenabled = ((val & 0xF) == 0xA);
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
		    	setsensor();
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

    void MMU::updatesensor(gbGyro pos, bool ispressed)
    {
	mbc7gyroval = BitChange(mbc7gyroval, pos, ispressed);
    }

    void MMU::sensorpressed(gbGyro pos)
    {
	updatesensor(pos, true);
    }

    void MMU::sensorreleased(gbGyro pos)
    {
	updatesensor(pos, false);
    }

    void MMU::updategyro()
    {
	if (testbit(mbc7gyroval, gbGyro::gyLeft))
	{
	    xsensor += 2;

	    if (xsensor > 464)
	    {
		xsensor = 464;
	    }

	    if (xsensor < 0)
	    {
		xsensor = 10;
	    }
	}
	else if (testbit(mbc7gyroval, gbGyro::gyRight))
	{
	    xsensor -= 2;

	    if (xsensor < -464)
	    {
		xsensor = -464;
	    }

	    if (xsensor > 0)
	    {
		xsensor = -10;
	    }
	}
	else if (xsensor > 0)
	{
	    xsensor -= 2;

	    if (xsensor < 0)
	    {
		xsensor = 0;
	    }
	}
	else if (xsensor < 0)
	{
	    xsensor += 2;

	    if (xsensor > 0)
	    {
		xsensor = 0;
	    }
	}

	if (testbit(mbc7gyroval, gbGyro::gyUp))
	{
	    ysensor += 2;

	    if (ysensor > 464)
	    {
		ysensor = 464;
	    }

	    if (ysensor < 0)
	    {
		ysensor = 10;
	    }
	}
	else if (testbit(mbc7gyroval, gbGyro::gyDown))
	{
	    ysensor -= 2;

	    if (ysensor < -464)
	    {
		ysensor = -464;
	    }

	    if (ysensor > 0)
	    {
		ysensor = -10;
	    }
	}
	else if (ysensor > 0)
	{
	    ysensor -= 2;

	    if (ysensor < 0)
	    {
		ysensor = 0;
	    }
	}
	else if (ysensor < 0)
	{
	    ysensor += 2;

	    if (ysensor > 0)
	    {
		ysensor = 0;
	    }
	}
    }

    void MMU::setsensor()
    {
	updategyro();
	mbc7sensorx = (0x81D0 + xsensor);
	mbc7sensory = (0x81D0 + ysensor);
    }

    void MMU::writembc7eeprom(uint8_t val)
    {
	bool oldchipsel = mbc7chipsel;
	bool oldchipclk = mbc7chipclk;
	mbc7chipsel = testbit(val, 7);
	mbc7chipclk = testbit(val, 6);

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
	    if (mbc7idle && testbit(val, 1))
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
			mbc7chipbuf |= testbit(val, 1);
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
			mbc7chipbuf |= testbit(val, 1);
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
			mbc7chipbuf |= testbit(val, 1);
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
		mbc7intvalue = (int)(testbit(mbc7chipbuf, 15));
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

    uint8_t MMU::gbcameraread(uint16_t addr)
    {
	uint8_t temp = 0;

	if (addr < 0x4000)
	{
	    temp = rom[addr];
	}
	else if (addr < 0x8000)
	{
	    temp = cartmem[(addr - 0x4000) + (currentrombank * 0x4000)];
	}
	else if ((addr >= 0xA000) && (addr < 0xC000))
	{
	    if (cameramode)
	    {
		temp = readgbcamreg(addr);
	    }
	    else
	    {
		temp = rambanks[(addr - 0xA000) + (currentrambank * 0x2000)];
	    }
	}

	return temp;
    }

    void MMU::gbcamerawrite(uint16_t addr, uint8_t value)
    {
	if (addr < 0x2000)
	{
	    if (externalrampres)
	    {
		ramenabled = ((value & 0xF) == 0xA);
	    }
	}
	else if (addr < 0x4000)
	{
	    currentrombank = (value & 0xFF);
	    currentrombank &= (numrombanks - 1);
	}
	else if (addr < 0x5000)
	{
	    cameramode = testbit(value, 4);
	    camera_bank = (value & 0xF);

	    if (!cameramode)
	    {
		currentrambank = (value & 0xFF);
		currentrambank &= (numrambanks - 1);
	    }
	}
	else if ((addr >= 0xA000) && (addr < 0xC000))
	{
	    if (cameramode)
	    {
		writegbcamreg(addr, value);
	    }
	    else
	    {
		if (ramenabled)
		{
		    rambanks[(addr - 0xA000) + (currentrambank * 0x2000)] = value;
		}
	    }
	}

	return;
    }

    uint8_t MMU::readgbcamreg(uint16_t addr)
    {
	uint8_t temp = 0x00;

	int reg = (addr & 0x7F);

	if (reg == 0)
	{
	    temp = camera_trigger;
	}
	else
	{
	    temp = 0x00;
	}

	return temp;
    }

    void MMU::writegbcamreg(uint16_t addr, uint8_t val)
    {
	int reg = (addr & 0x7F);

	if (reg >= 0x36)
	{
	    return;
	}

	switch (reg)
	{
	    case 0x00:
	    {
		camera_trigger = (val & 0x6);
		if (testbit(val, 0) && camframe)
		{
		    camera_trigger = BitSet(camera_trigger, 0);
		    take_camera_pic();
		    camera_capture = true;
		}
	    }
	    break;
	    case 0x01: camera_outputedge = val; break;
	    case 0x02: camera_exposure = ((val << 8) | (camera_exposure & 0xFF)); break;
	    case 0x03: camera_exposure = ((camera_exposure & 0xFF00) | val); break;
	    case 0x04: camera_edge = val; break;
	    case 0x05: camera_voltage = val; break;
	    default: camera_matrix[(reg - 6)] = val; break;
	}

	return;
    }

    void MMU::updatecamera()
    {
	if (gbmbc != MBCType::Camera)
	{
	    return;
	}

	if (!camera_capture)
	{
	    return;
	}

	if (camera_clock > 0)
	{
	    camera_clock -= 4;

	    if (camera_clock <= 0)
	    {
		camera_clock = 0;
		camera_trigger = BitReset(camera_trigger, 0);
		camera_capture = false;
	    }
	}	
    }

    int MMU::camera_matrix_process(int val, int x, int y)
    {
	int xpos = (x & 3);
	int ypos = (y & 3);

	int base = ((ypos * 4 + xpos) * 3);

	int r0 = camera_matrix[base];
	int r1 = camera_matrix[(base + 1)];
	int r2 = camera_matrix[(base + 2)];

	if (val < r0)
	{
	    return 3;
	}
	else if (val < r1)
	{
	    return 2;
	}
	else if (val < r2)
	{
	    return 1;
	}

	return 0;
    }

    void MMU::take_camera_pic()
    {
	if (camframe)
	{
	    if (!camframe(cam_web_output))
	    {
		cout << "Could not get camera capture" << endl;
		return;
	    }
	}
	else
	{
	    return;
	}

	int cam_pm = ((camera_trigger >> 1) & 0x3);

	int pbits = static_cast<int>(cam_pm > 0);
	int mbits = (cam_pm >= 2) ? 2 : (1 - pbits);

	bool nbit = testbit(camera_outputedge, 7);
	int vhbits = ((camera_outputedge >> 5) & 0x3);

	uint16_t exposure = camera_exposure;

	const float edge_ratio_lut[8] = { 0.5, 0.75, 1.00, 1.25, 2.00, 3.00, 4.00, 5.00 };
	float edge_alpha = edge_ratio_lut[((camera_edge & 0x70) >> 4)];

	bool e3bit = testbit(camera_edge, 7);
	bool ibit = testbit(camera_edge, 3);

	camera_clock = (32446 + (nbit ? 0 : 512) + 16 * exposure);

	for (int i = 0; i < 128; i++)
	{
	    for (int j = 0; j < 120; j++)
	    {
		int value = cam_web_output[(i + (j * 128))];
		value = ((value * exposure) / 0x300);
		value = (128 + (((value - 128) * 1) / 8));
		cam_ret_output[(i + (j * 128))] = clamp(value, 0, 255);
	    }
	}

	if (ibit)
	{
	    for (int i = 0; i < (128 * 120); i++)
	    {
		cam_ret_output[i] = (255 - cam_ret_output[i]);
	    }
	}

	for (int i = 0; i < (128 * 120); i++)
	{
	    cam_ret_output[i] = (cam_ret_output[i] - 128);
	}

	array<int, (128 * 120)> temp_buf;

	int filter_mode = ((nbit << 3) | (vhbits << 1) | e3bit);

	switch (filter_mode)
	{
	    case 0x0:
	    {
		for (int i = 0; i < (128 * 120); i++)
		{
		    temp_buf[i] = cam_ret_output[i];
		}

		for (int i = 0; i < 128; i++)
		{
		    for (int j = 0; j < 120; j++)
		    {
			int ms = temp_buf[(i + (min((j + 1), 119) * 128))];
			int px = temp_buf[(i + (j * 128))];

			int value = 0;

			if (testbit(pbits, 0))
			{
			    value += px;
			}

			if (testbit(pbits, 1))
			{
			    value += ms;
			}

			if (testbit(mbits, 0))
			{
			    value -= px;
			}

			if (testbit(mbits, 1))
			{
			    value -= ms;
			}

			cam_ret_output[(i + (j * 128))] = clamp(value, -128, 127);
		    }
		}
	    }
	    break;
	    case 0x1:
	    {
		for (int i = 0; i < (128 * 120); i++)
		{
		    cam_ret_output[i] = 0;
		}
	    }
	    break;
	    case 0x2:
	    {
		for (int i = 0; i < 128; i++)
		{
		    for (int j = 0; j < 120; j++)
		    {
			int mw = cam_ret_output[(max(0, (i - 1)) + (j * 128))];	
			int me = cam_ret_output[(min((i + 1), 127) + (j * 128))];
			int px = cam_ret_output[(i + (j * 128))];

			int value = (px + ((2 * px - mw - me) * edge_alpha));

			temp_buf[(i + (j * 128))] = clamp(value, 0, 255);
		    }
		}

		for (int i = 0; i < 128; i++)
		{
		    for (int j = 0; j < 120; j++)
		    {
			int ms = temp_buf[(i + (min((j + 1), 119) * 128))];
			int px = temp_buf[(i + (j * 128))];

			int value = 0;

			if (testbit(pbits, 0))
			{
			    value += px;
			}

			if (testbit(pbits, 1))
			{
			    value += ms;
			}

			if (testbit(mbits, 0))
			{
			    value -= px;
			}

			if (testbit(mbits, 1))
			{
			    value -= ms;
			}

			cam_ret_output[(i + (j * 128))] = clamp(value, -128, 127);
		    }
		}
	    }
	    break;
	    case 0xE:
	    {
		for (int i = 0; i < 128; i++)
		{
		    for (int j = 0; j < 120; j++)
		    {
			int ms = cam_ret_output[(i + (min((j + 1), 119) * 128))];
			int mn = cam_ret_output[(i + (max(0, (j - 1)) * 128))];
			int mw = cam_ret_output[(max(0, (i - 1)) + (j * 128))];
			int me = cam_ret_output[(min((i + 1), 127) + (j * 128))];
			int px = cam_ret_output[(i + (j * 128))];

			int value = (px + ((4 * px - mw - me - mn - ms) * edge_alpha));

			temp_buf[(i + (j * 128))] = clamp(value, -128, 127);
		    }
		}

		for (int i = 0; i < (128 * 120); i++)
		{
		    cam_ret_output[i] = temp_buf[i];
		}
	    }
	    break;
	}

	for (int i = 0; i < (128 * 120); i++)
	{
	    cam_ret_output[i] = (cam_ret_output[i] + 128);
	}

	array<int, (128 * 112)> four_colors_buffer;

	for (int i = 0; i < 128; i++)
	{
	    for (int j = 0; j < 112; j++)
	    {
		four_colors_buffer[(i + (j * 128))] = camera_matrix_process(cam_ret_output[(i + ((j + 4) * 128))], i, j);
	    }
	}

	vector<uint8_t> final_buffer;

	for (int tile = 0; tile < 224; tile++)
	{
	    for (int xpos = 0; xpos < 8; xpos++)
	    {
		uint8_t res_byte1 = 0;
		uint8_t res_byte2 = 0;

		for (int ypos = 0; ypos < 8; ypos++)
		{
		    res_byte1 <<= 1;
		    res_byte2 <<= 1;

		    int current_pixel = (((tile * 8) % 128) + (xpos * 128) + (1024 * (tile / 16)) + ypos);
		    int final_color = four_colors_buffer[current_pixel];

		    switch (final_color)
		    {
			case 0:
			{
			    res_byte1 |= 0;
			    res_byte2 |= 0;
			}
			break;
			case 1:
			{
			    res_byte1 |= 1;
			    res_byte2 |= 0;
			}
			break;
			case 2:
			{
			    res_byte1 |= 0;
			    res_byte2 |= 1;
			}
			break;
			case 3:
			{
			    res_byte1 |= 1;
			    res_byte2 |= 1;
			}
			break;
		    }
		}

		final_buffer.push_back(res_byte1);
		final_buffer.push_back(res_byte2);
	    }
	}

	for (int i = 0; i < static_cast<int>(final_buffer.size()); i++)
	{
	    rambanks[(0x100 + i)] = final_buffer[i];
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
