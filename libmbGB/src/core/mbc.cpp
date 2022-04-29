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

#include "mmu.h"
#include <iostream>
using namespace gb;
using namespace std;

namespace gb
{
    // MBC1 mapper (supports both regular MBC1 and MBC1M)
    // TODO: Implement MBC1S mapper
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

    // Regular MBC1 mapper
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
	else if (inRange(addr, 0xA000, 0xC000))
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
	else if (inRange(addr, 0xA000, 0xC000))
	{
	    if (ramenabled)
	    {
		rambanks[(addr - 0xA000) + (currentrambank * 0x2000)] = value;
	    }
	}
    }

    // MBC1M mapper
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
	else if (inRange(addr, 0xA000, 0xC000))
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
	else if (inRange(addr, 0xA000, 0xC000))
	{
	    if (ramenabled)
	    {
		rambanks[(addr - 0xA000)] = value;
	    }
	}
    }

    // MBC2 mapper
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
	else if (inRange(addr, 0xA000, 0xC000))
	{
	    if (ramenabled)
	    {
		temp = (rambanks[(addr & 0x1FF)] | 0xF0);
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
	if (addr < 0x4000)
	{
	    if (!testbit(addr, 8))
	    {
		ramenabled = ((value & 0xF) == 0xA);
	    }
	    else
	    {
		currentrombank = (value & 0xF);

		if (currentrombank == 0)
		{
		    currentrombank += 1;
		}

		currentrombank %= numrombanks;
	    }

	}
	else if (inRange(addr, 0xA000, 0xC000))
	{
	    if (ramenabled)
	    {
		rambanks[(addr & 0x1FF)] = (value & 0xF);
	    }
	}
    }

    // MBC3 mapper
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
	else if (inRange(addr, 0xA000, 0xC000))
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
	else if (inRange(addr, 0xA000, 0xC000))
	{
	    if (ramenabled && (currentrambank < 0x8))
	    {
		rambanks[(addr - 0xA000) + (currentrambank * 0x2000)] = value;
	    }
	    else if (rtcenabled && inRangeEx(currentrambank, 0x8, 0xC))
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
			realdays = ((getbitval(value, 0) << 8) | (realdays & 0xFF));
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
	if (!isrtcpres)
	{
	    return;
	}

	if (gbmbc == MBCType::MBC3)
	{
	    updatembc3rtc();
	}

	if (gbmbc == MBCType::TPP1)
	{
	    updatetpp1rtc();
	}
    }

    void MMU::updatembc3rtc()
    {
	if (rtchalt)
	{
	    return;
	}

	int atomic_increase = doublespeed ? 2 : 4;
	timercounter += atomic_increase;

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
	if (!isrtcpres)
	{
	    return;
	}

	if (gbmbc == MBCType::MBC3)
	{
	    initmbc3rtc();
	}
    }

    void MMU::initmbc3rtc()
    {
	time_t newtime = time(NULL);
	uint32_t difference = 0;

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
	    difference = uint32_t(newtime - current_time);
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
	realdays |= getbitval(newdays, 8);

	if (newdays > 511)
	{
	    realdayscarry = true;
	}
    }

    void MMU::loadmbc3rtc(vector<uint8_t> saveram, size_t ramsize)
    {
	size_t size = saveram.size();

	if (batteryenabled && (size == (ramsize + 48)))
	{
	    realsecs = saveram[ramsize];
	    realmins = saveram[(ramsize + 4)];
	    realhours = saveram[(ramsize + 8)];
	    uint8_t realdayslo = saveram[(ramsize + 12)];
	    uint8_t realdayshi = saveram[(ramsize + 16)];
	    latchsecs = saveram[(ramsize + 20)];
	    latchmins = saveram[(ramsize + 24)];
	    latchhours = saveram[(ramsize + 28)];
	    uint8_t latchdayslo = saveram[(ramsize + 32)];
	    uint8_t latchdayshi = saveram[(ramsize + 36)];

	    realdays = ((getbitval(realdayshi, 0) << 8) | realdayslo);
	    rtchalt = testbit(realdayshi, 6);
	    realdayscarry = testbit(realdayshi, 7);

	    latchdays = ((getbitval(latchdayshi, 0) << 8) | latchdayslo);
	    latchhalt = testbit(latchdayshi, 6);
	    latchdayscarry = testbit(latchdayshi, 7);

	    for (int i = 0; i < 8; i++)
	    {
		current_time |= (saveram[(ramsize + 40 + i)] << (i << 3));
	    }

	    if (current_time <= 0)
	    {
		current_time = time(NULL);
	    }

	    inittimer();
	}
    }

    void MMU::savembc3rtc(vector<uint8_t> &saveram)
    {
	size_t size = saveram.size();

	if (batteryenabled)
	{
	    inittimer();
	    saveram.resize((size + 48), 0);
	    saveram[size] = realsecs;
	    saveram[(size + 4)] = realmins;
	    saveram[(size + 8)] = realhours;
	    saveram[(size + 12)] = (realdays & 0xFF);
	    saveram[(size + 16)] = (((realdays >> 8) & 0x1) | (rtchalt << 6) | (realdayscarry << 7));
	    saveram[(size + 20)] = latchsecs;
	    saveram[(size + 24)] = latchmins;
	    saveram[(size + 28)] = latchhours;
	    saveram[(size + 32)] = (latchdays & 0xFF);
	    saveram[(size + 36)] = (((latchdays >> 8) & 0x1) | (latchhalt << 6) | (latchdayscarry << 7));

	    for (int i = 0; i < 8; i++)
	    {
		saveram[(size + 40 + i)] = ((uint8_t)(current_time >> (i << 3)));
	    }
	}
    }

    // MBC5 mapper
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
	else if (inRange(addr, 0xA000, 0xC000))
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
		bool is_rumble = (rumble_strength != 0);

		if (testbit(value, 3) != is_rumble)
		{
		    rumble_strength = (is_rumble) ? 0 : 3;
		}
	    }
	}
	else if (inRange(addr, 0xA000, 0xC000))
	{
	    if (ramenabled)
	    {
		uint16_t ramaddr = (currentrambank * 0x2000);
		rambanks[(addr - 0xA000) + ramaddr] = value;
	    }
	}

	return;
    }

    // MBC6 mapper (used by Net de Get) (WIP)
    //
    // Note: Since the effective number of ROM and RAM banks are twice what
    // they usually would be, the size of each of those banks
    // is only half of the size that the banks would be
    // on other mappers.
    //
    // TODO: This mapper uses custom flash ROM (by Macronix), which hasn't been
    // implemented yet. However, it could be implemented in the
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
	else if (inRange(addr, 0xA000, 0xB000))
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
	else if (inRange(addr, 0xB000, 0xC000))
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
	else if (inRange(addr, 0xA000, 0xB000))
	{
	    if (ramenabled)
	    {
		uint16_t ramaddr = (mbc6rambanka * 0x1000);
		rambanks[(addr - 0xA000) + ramaddr] = val;
	    }
	}
	else if (inRange(addr, 0xB000, 0xC000))
	{
	    if (ramenabled)
	    {
		uint16_t ramaddr = (mbc6rambankb * 0x1000);
		rambanks[(addr - 0xB000) + ramaddr] = val;
	    }
	}

	return;
    }

    // MBC7 mapper (used by Kirby Tilt n' Tumble, Koro Koro Kirby and Command Master)
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
	else if (inRange(addr, 0xA000, 0xB000))
	{
	    if (ramenabled && ramsecenabled)
	    {
		temp = mbc7_reg_read(addr);
	    }
	    else
	    {
		temp = 0xFF;
	    }
	}
	else if (inRange(addr, 0xB000, 0xC000))
	{
	    temp = 0xFF;
	}

	return temp;
    }

    void MMU::mbc7write(uint16_t addr, uint8_t val)
    {
	if (addr < 0x2000)
	{
	    switch (val & 0xF)
	    {
		case 0x0:
		{
		    ramenabled = false;
		    ramsecenabled = false;
		}
		break;
		case 0xA: ramenabled = true; break;
		default: break;
	    }
	}
	else if (addr < 0x4000)
	{
	    currentrombank = (val & 0x7F);
	    currentrombank %= numrombanks;
	}
	else if (addr < 0x6000)
	{
	    ramsecenabled = (val == 0x40);
	}
	else if (inRange(addr, 0xA000, 0xB000))
	{
	    if (ramenabled && ramsecenabled)
	    {
		mbc7_reg_write(addr, val);
	    }
	}
    }

    uint8_t MMU::mbc7_reg_read(uint16_t addr)
    {
	int reg_num = ((addr >> 4) & 0xF);
	uint8_t temp = 0;

	switch (reg_num)
	{
	    case 2: temp = (mbc7_sensorx & 0xFF); break;
	    case 3: temp = (mbc7_sensorx >> 8); break;
	    case 4: temp = (mbc7_sensory & 0xFF); break;
	    case 5: temp = (mbc7_sensory >> 8); break;
	    case 8: temp = read_mbc7_eeprom(); break;
	    default:
	    {
		cout << "Unrecognized register read from address of 0xax" << hex << int(reg_num) << "x" << endl;
		exit(0);
		temp = 0;
	    }
	    break;
	}

	return temp;
    }

    void MMU::mbc7_reg_write(uint16_t addr, uint8_t value)
    {
	int reg_num = ((addr >> 4) & 0xF);

	switch (reg_num)
	{
	    case 0:
	    {
		mbc7_latch = ((value & 0x55) == 0x55);

		if (mbc7_latch)
		{
		    mbc7_sensorx = 0x8000;
		    mbc7_sensory = 0x8000;
		}
	    }
	    break;
	    case 1:
	    {
		mbc7_latch &= ((value & 0xAA) == 0xAA);

		if (mbc7_latch)
		{
		    latch_mbc7_gyro();
		    mbc7_latch = false;
		}
	    }
	    break;
	    case 8: write_mbc7_eeprom(value); break;
	    default:
	    {
		cout << "Unrecognized register write to address of 0xax" << hex << int(reg_num) << "x, value of " << hex << int(value) << endl;
		exit(0);
	    }
	    break;
	}
    }

    uint8_t MMU::read_mbc7_eeprom()
    {
	uint8_t temp = 0;
	temp = changebit(temp, 7, mbc7_cs);
	temp = changebit(temp, 6, mbc7_clk);
	temp = changebit(temp, 1, mbc7_di);
	temp = changebit(temp, 0, mbc7_do);

	/*
	cout << "Reading EEPROM:" << endl;
	cout << "DO pin: " << int(mbc7_do) << endl;
	cout << "DI pin: " << int(mbc7_di) << endl;
	cout << "CLK pin: " << int(mbc7_clk) << endl;
	cout << "CS pin: " << int(mbc7_cs) << endl;
	cout << endl;
	*/
	
	return temp;
    }

    void MMU::write_mbc7_eeprom(uint8_t value)
    {
	/*
	cout << "Writing EEPROM:" << endl;
	cout << "DI pin: " << getbitval(value, 1) << endl;
	cout << "CLK pin: " << getbitval(value, 6) << endl;
	cout << "CS pin: " << getbitval(value, 7) << endl;
	cout << endl;
	*/

	bool prev_clk = mbc7_clk;

	mbc7_cs = testbit(value, 7);
	mbc7_clk = testbit(value, 6);
	mbc7_di = testbit(value, 1);

	if (!prev_clk && mbc7_clk)
	{
	    switch (mbc7_state)
	    {
		case MBC7State::StartBit:
		{
		    if (mbc7_cs && mbc7_di)
		    {
			mbc7_state = MBC7State::Command;
			mbc7_do = false;
		    }
		}
		break;
		case MBC7State::Command:
		{
		    mbc7_buffer <<= 1;
		    mbc7_buffer |= mbc7_di;

		    mbc7_length += 1;

		    if (mbc7_length == 10)
		    {
			process_mbc7_command();
			mbc7_length = 0;
			mbc7_buffer = 0;
		    }
		}
		break;
		case MBC7State::Read:
		{
		    mbc7_do = testbit(mbc7_read_value, 15);
		    mbc7_read_value <<= 1;
		    mbc7_length += 1;

		    if (mbc7_length == 16)
		    {
			mbc7_length = 0;
			mbc7_read_value = 0;
			mbc7_state = MBC7State::StartBit;
		    }
		}
		break;
		case MBC7State::Write:
		{
		    mbc7_write_value <<= 1;
		    mbc7_write_value |= mbc7_di;
		    mbc7_length += 1;

		    if (mbc7_length == 16)
		    {
			mbc7_length = 0;

			if (mbc7_erase_enabled)
			{
			    rambanks[mbc7_write_addr] = (mbc7_write_value >> 8);
			    rambanks[mbc7_write_addr + 1] = (mbc7_write_value & 0xFF);

			    mbc7_do = true;
			}

			mbc7_write_value = 0;

			mbc7_state = MBC7State::StartBit;
		    }
		}
		break;
		case MBC7State::WriteAll:
		{
		    mbc7_write_value <<= 1;
		    mbc7_write_value |= mbc7_di;
		    mbc7_length += 1;

		    if (mbc7_length == 16)
		    {
			mbc7_length = 0;

			if (mbc7_erase_enabled)
			{
			    for (size_t index = 0; index < rambanks.size(); index += 2)
			    {
				rambanks[index] = (mbc7_write_value >> 8);
				rambanks[index + 1] = (mbc7_write_value & 0xFF);
			    }

			    mbc7_do = true;
			}

			mbc7_write_value = 0;

			mbc7_state = MBC7State::StartBit;
		    }
		}
		break;
		default: break;
	    }
	}
    }

    void MMU::process_mbc7_command()
    {
	int cmd_type = ((mbc7_buffer >> 8) & 0x3);

	switch (cmd_type)
	{
	    case 0:
	    {
		int subcmd_type = ((mbc7_buffer >> 6) & 0x3);

		switch (subcmd_type)
		{
		    // EWDS
		    case 0:
		    {
			mbc7_erase_enabled = false;
			mbc7_state = MBC7State::StartBit;
		    }
		    break;
		    // WRAL
		    case 1:
		    {
			mbc7_state = MBC7State::WriteAll;
		    }
		    break;
		    // ERAL
		    case 2:
		    {
			if (mbc7_erase_enabled)
			{
			    for (size_t index = 0; index < rambanks.size(); index += 2)
			    {
				rambanks[index] = 0xFF;
				rambanks[index + 1] = 0xFF;
			    }

			    mbc7_do = true;
			}

			mbc7_state = MBC7State::StartBit;
		    }
		    break;
		    // EWEN
		    case 3:
		    {
			mbc7_erase_enabled = true;
			mbc7_state = MBC7State::StartBit;
		    }
		    break;
		    default:
		    {
			cout << "Unrecognized sub-command type of " << dec << int(subcmd_type) << endl;
			exit(0);
		    }
		    break;
		}
	    }
	    break;
	    // WRITE
	    case 1:
	    {
		uint32_t eeprom_addr = (mbc7_buffer & 0xFF);

		if (numrombanks != 128)
		{
		    eeprom_addr &= 0x7F;
		}

		eeprom_addr <<= 1;
		mbc7_write_addr = eeprom_addr;
		mbc7_state = MBC7State::Write;
	    }
	    break;
	    // READ
	    case 2:
	    {
		uint32_t eeprom_addr = (mbc7_buffer & 0xFF);

		if (numrombanks != 128)
		{
		    eeprom_addr &= 0x7F;
		}

		eeprom_addr <<= 1;

		mbc7_read_value = ((rambanks[eeprom_addr] << 8) | rambanks[eeprom_addr + 1]);
		mbc7_state = MBC7State::Read;
	    }
	    break;
	    // ERASE
	    case 3:
	    {
		uint32_t eeprom_addr = (mbc7_buffer & 0xFF);

		if (numrombanks != 128)
		{
		    eeprom_addr &= 0x7F;
		}

		eeprom_addr <<= 1;

		if (mbc7_erase_enabled)
		{
		    rambanks[eeprom_addr] = 0xFF;
		    rambanks[eeprom_addr + 1] = 0xFF;
		    mbc7_do = true;
		}

		mbc7_state = MBC7State::StartBit;
	    }
	    break;
	    default:
	    {
		cout << "Unrecognized MBC7 EEPROM command of " << dec << int(cmd_type) << endl;
		exit(0);
	    }
	    break;
	}
    }

    void MMU::set_accel_values(float x, float y)
    {
	sensorx = int(0x70 * x);
	sensory = int(0x70 * y);
    }

    void MMU::latch_mbc7_gyro()
    {
	mbc7_sensorx = (0x81D0 + sensorx);
	mbc7_sensory = (0x81D0 + sensory);
    }

    // GB Camera mapper (used by the Game Boy Camera, obviously)

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
	else if (inRange(addr, 0xA000, 0xC000))
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
	else if (inRange(addr, 0xA000, 0xC000))
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
		    camera_trigger = setbit(camera_trigger, 0);
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
	if (!isgbcamera())
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
		camera_trigger = resetbit(camera_trigger, 0);
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

	for (size_t i = 0; i < final_buffer.size(); i++)
	{
	    rambanks[(0x100 + i)] = final_buffer[i];
	}
    }

    // HuC3 mapper (WIP) (used in several Hudson Soft titles)
    // Much of this implementation was derived from endrift's documentation,
    // as well as the source code of both gbe-plus and SameBoy.
    // Even then, there's still a massive amount of unknowns in regards
    // to this mapper, and reverse-engineering work is still very much ongoing.
    // This implementation will be updated as more info about 
    // this specific mapper becomes publicly available.

    uint8_t MMU::huc3read(uint16_t addr)
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
	else if (inRange(addr, 0xA000, 0xC000))
	{
	    switch (huc3_mode)
	    {
		case 0x0:
		case 0xA:
		{
		    temp = rambanks[(addr - 0xA000) + (currentrambank * 0x2000)];
		}
		break;
		case 0xB:
		case 0xC:
		{
		    temp = huc3_value;
		}
		break;
		case 0xD:
		{
		    temp = 0xFF;
		}
		break;
		default:
		{
		    cout << "Unrecognized read under HuC3 mode of " << hex << int(huc3_mode) << endl;
		    exit(0);
		    temp = 0;
		}
		break;
	    }
	}

	return temp;
    }

    void MMU::huc3write(uint16_t addr, uint8_t value)
    {
	if (addr < 0x2000)
	{
	    huc3_mode = (value & 0xF);
	}
	else if (addr < 0x4000)
	{
	    currentrombank = (value & 0x7F);

	    if (currentrombank == 0)
	    {
		currentrombank += 1;
	    }
	}
	else if (addr < 0x6000)
	{
	    currentrambank = (value & 0x3);
	}
	else if (addr < 0x8000)
	{
	    // TODO: What effects do writes have on this region?
	    // (needs hardware verification)
	    return;
	}
	else if (inRange(addr, 0xA000, 0xC000))
	{
	    switch (huc3_mode)
	    {
		case 0x0: break;
		case 0xA:
		{
		    rambanks[(addr - 0xA000) + (currentrambank * 0x2000)] = value;
		}
		break;
		case 0xB:
		{
		    huc3_value = (0x80 | value);
		}
		break;
		case 0xD:
		{
		    huc3_commit();
		}
		break;
		default:
		{
		    cout << "Unrecognized write under HuC3 mode of " << hex << int(huc3_mode) << endl;
		    exit(0);
		}
		break;
	    }
	}
    }

    void MMU::huc3_commit()
    {
	int reg_num = ((huc3_value >> 4) & 0x7);
	int reg_val = (huc3_value & 0xF);

	switch (reg_num)
	{
	    case 1:
	    {
		// TODO: Implement HuC3 registers
		if ((huc3_index & 0xF8) == 0x10)
		{
		    cout << "Latching Huc3 RTC..." << endl;
		}

		huc3_value &= 0xF0;

		if (testbit(huc3_value, 4))
		{
		    huc3_index += 1;
		}
	    }
	    break;
	    case 3:
	    {
		if (testbit(huc3_value, 4))
		{
		    huc3_index += 1;
		}
	    }
	    break;
	    case 4:
	    {
		huc3_index = ((huc3_index & 0xF0) | reg_val);
	    }
	    break;
	    case 5:
	    {
		huc3_index = ((huc3_index & 0xF) | (reg_val << 4));
	    }
	    break;
	    case 6:
	    {
		switch (reg_val)
		{
		    case 0x0:
		    {
			cout << "Latching HuC3 RTC..." << endl;
		    }
		    break;
		    case 0x1:
		    {
			cout << "Setting HuC3 RTC..." << endl;
		    }
		    break;
		    case 0x2:
		    {
			cout << "Unimplemented: HuC3 read-only mode" << endl;
		    }
		    break;
		    case 0xE:
		    {
			cout << "Playing HuC3 tone..." << endl;
		    }
		    break;
		    default:
		    {
			cout << "Unrecognized HuC3 register value of " << hex << int(reg_val) << endl;
			exit(0);
		    }
		    break;
		}

		huc3_value = 0xE1;
	    }
	    break;
	    default:
	    {
		cout << "Unrecognized HuC3 register number of " << hex << int(reg_num) << endl;
		exit(0);
	    }
	    break;
	}
    }

    // HuC1 mapper (used most notably by Pokemon Card GB) (WIP)
    uint8_t MMU::huc1read(uint16_t addr)
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
	else if (inRange(addr, 0xA000, 0xC000))
	{
	    if (huc1_ir_trigger)
	    {
		// TODO: Implement IR
		temp = 0xC0;
	    }
	    else
	    {
		uint32_t huc1_ram_bank = (huc1_bank_mode) ? currentrambank : 0;
		temp = rambanks[(addr - 0xA000) + (huc1_ram_bank * 0x2000)];
	    }
	}

	return temp;
    }

    void MMU::huc1write(uint16_t addr, uint8_t value)
    {
	if (addr < 0x2000)
	{
	    huc1_ir_trigger = ((value & 0xF) == 0xE);
	}
	else if (addr < 0x4000)
	{
	    currentrombank = (value & 0x3F);

	    if (currentrombank == 0)
	    {
		currentrombank += 1;
	    }
	}
	else if (addr < 0x6000)
	{
	    currentrambank = (value & 0x3);
	}
	else if (addr < 0x8000)
	{
	    huc1_bank_mode = testbit(value, 0);
	}
	else if (inRange(addr, 0xA000, 0xC000))
	{
	    if (huc1_ir_trigger)
	    {
		// TODO: Implement IR
		if (testbit(value, 0))
		{
		    cout << "Turning IR signal on..." << endl;
		}
		else
		{
		    cout << "Shutting IR signal off..." << endl;
		}
	    }
	    else
	    {
		uint32_t huc1_ram_bank = (huc1_bank_mode) ? currentrambank : 0;
		rambanks[(addr - 0xA000) + (huc1_ram_bank * 0x2000)] = value;
	    }
	}
    }

    // Wisdom Tree mapper (used by Wisdom Tree's unlicensed Game Boy games)

    uint8_t MMU::wisdomtreeread(uint16_t addr)
    {
	uint8_t temp = 0;

	if (addr < 0x8000)
	{
	    temp = cartmem[(addr + (wisdomrombank * 0x4000))];
	}
	else if (inRange(addr, 0xA000, 0xC000))
	{
	    temp = 0xFF;
	}

	return temp;
    }

    void MMU::wisdomtreewrite(uint16_t addr, uint8_t value)
    {
	(void)value;
	if (addr < 0x4000)
	{
	    int bank = (addr & 0x3F);
	    wisdomrombank = (bank << 1);
	}
    }

    // TPP1 mapper (custom homebrew mapper by TwitchPlaysPokemon)
    // (https://github.com/TwitchPlaysPokemon/tpp1)
    // TODO: Implement RTC saves

    uint8_t MMU::tpp1read(uint16_t addr)
    {
	uint8_t temp = 0;

	if (addr < 0x4000)
	{
	    temp = rom[addr];
	}
	else if (addr < 0x8000)
	{
	    uint16_t tpp1_rom_bank = (((mr1_reg << 8) | mr0_reg) % numrombanks);
	    temp = cartmem[((addr - 0x4000) + (tpp1_rom_bank * 0x4000))];
	}
	else if (inRange(addr, 0xA000, 0xC000))
	{
	    switch (map_control)
	    {
		case TPP1Mapped::ControlReg: temp = read_tpp1_control_reg(addr); break;
		case TPP1Mapped::SRAMRead:
		case TPP1Mapped::SRAMReadWrite:
		{
		    uint16_t tpp1_ram_bank = (mr2_reg % numrambanks);
		    temp = rambanks[(addr - 0xA000) + (tpp1_ram_bank * 0x2000)];
		}
		break;
		case TPP1Mapped::RTCLatched: temp = read_tpp1_rtc(addr); break;
		default: temp = 0; break;
	    }
	}

	return temp;
    }

    void MMU::tpp1write(uint16_t addr, uint8_t value)
    {
	if (addr < 0x4000)
	{
	    write_tpp1_control_reg(addr, value);
	}
	else if (addr < 0x8000)
	{
	    return;
	}
	else if (inRange(addr, 0xA000, 0xC000))
	{
	    switch (map_control)
	    {
		case TPP1Mapped::ControlReg: break;
		case TPP1Mapped::SRAMRead: break;
		case TPP1Mapped::SRAMReadWrite:
		{
		    uint16_t tpp1_ram_bank = mr2_reg;
		    rambanks[(addr - 0xA000) + (tpp1_ram_bank * 0x2000)] = value;
		}
		break;
		case TPP1Mapped::RTCLatched: write_tpp1_rtc(addr, value); break;
		default: break;
	    }
	}
    }

    uint8_t MMU::read_tpp1_control_reg(uint16_t addr)
    {
	uint8_t temp = 0;
	int reg_addr = (addr & 3);

	switch (reg_addr)
	{
	    case 0: temp = mr0_reg; break;
	    case 1: temp = mr1_reg; break;
	    case 2: temp = mr2_reg; break;
	    case 3: temp = mr4_reg; break;
	    default: break;
	}

	return temp;
    }

    void MMU::write_tpp1_control_reg(uint16_t addr, uint8_t value)
    {
	int reg_addr = (addr & 3);

	switch (reg_addr)
	{
	    case 0: mr0_reg = value; break;
	    case 1: mr1_reg = value; break;
	    case 2: mr2_reg = value; break;
	    case 3: write_tpp1_mr3(value); break;
	    default: break;   
	}
    }

    uint8_t MMU::read_tpp1_rtc(uint16_t addr)
    {
	if (!isrtcpres)
	{
	    return 0x00;
	}

	uint8_t temp = 0;
	int reg_addr = (addr & 3);

	switch (reg_addr)
	{
	    case 0: temp = tpp1_latched_week; break;
	    case 1: temp = ((tpp1_latched_day << 5) | tpp1_latched_hours); break;
	    case 2: temp = tpp1_latched_minutes; break;
	    case 3: temp = tpp1_latched_seconds; break;
	    default: break;
	}

	return temp;
    }

    void MMU::write_tpp1_rtc(uint16_t addr, uint8_t value)
    {
	if (!isrtcpres)
	{
	    return;
	}

	int reg_addr = (addr & 3);

	switch (reg_addr)
	{
	    case 0:
	    {
		tpp1_latched_week = value;
	    }
	    break;
	    case 1:
	    {
		tpp1_latched_day = ((value >> 5) & 0x7);
		tpp1_latched_hours = (value & 0x1F);
	    }
	    break;
	    case 2:
	    {
		tpp1_latched_minutes = value;
	    }
	    break;
	    case 3:
	    {
		tpp1_latched_seconds = value;
	    }
	    break;
	    default: break;   
	}
    }

    void MMU::write_tpp1_mr3(uint8_t value)
    {
	switch (value)
	{
	    case 0x00: map_control = TPP1Mapped::ControlReg; break;
	    case 0x02: map_control = TPP1Mapped::SRAMRead; break;
	    case 0x03: map_control = TPP1Mapped::SRAMReadWrite; break;
	    case 0x05: map_control = TPP1Mapped::RTCLatched; break;
	    case 0x10:
	    {
		tpp1_latched_week = tpp1_rtc_week;
		tpp1_latched_day = tpp1_rtc_day;
		tpp1_latched_hours = tpp1_rtc_hours;
		tpp1_latched_minutes = tpp1_rtc_minutes;
		tpp1_latched_seconds = tpp1_rtc_seconds;
	    }
	    break;
	    case 0x11:
	    {
		tpp1_rtc_week = tpp1_latched_week;
		tpp1_rtc_day = tpp1_latched_day;
		tpp1_rtc_hours = (tpp1_latched_hours & 0x1F);
		tpp1_rtc_minutes = (tpp1_latched_minutes & 0x3F);
		tpp1_rtc_seconds = (tpp1_latched_seconds & 0x3F);
	    }
	    break;
	    case 0x14:
	    {
		mr4_reg = resetbit(mr4_reg, 3);
	    }
	    break;
	    case 0x18:
	    {
		mr4_reg = resetbit(mr4_reg, 2);
	    }
	    break;
	    case 0x19:
	    {
		mr4_reg = setbit(mr4_reg, 2);
	    }
	    break;
	    case 0x20:
	    case 0x21:
	    case 0x22:
	    case 0x23:
	    {
		// Only set rumble strength if rumble is present
		if (isrumblepres)
		{
		    rumble_strength = (value & 3);

		    // If multiple rumble speeds aren't supported,
		    // the highest rumble strength allowed is 1
		    if (!is_multi_rumble)
		    {
			rumble_strength = clamp(rumble_strength, 0, 1);
		    }

		    mr4_reg = ((mr4_reg & 0xFC) | rumble_strength);
		}
		else
		{
		    mr4_reg &= 0xFC;
		}
	    }
	    break;
	    default:
	    {
		cout << "Unrecognized TPP1 MR3 value of " << hex << int(value) << endl;
		exit(0);
	    }
	    break;
	}
    }

    void MMU::updatetpp1rtc()
    {
	if (!testbit(mr4_reg, 2))
	{
	    return;
	}

	int atomic_increase = doublespeed ? 2 : 4;
	timercounter += atomic_increase;

	if (timercounter == 4194304)
	{
	    timercounter = 0;
	    tpp1_rtc_seconds = ((tpp1_rtc_seconds + 1) & 63);

	    if (tpp1_rtc_seconds == 60)
	    {
		tpp1_rtc_seconds = 0;
		tpp1_rtc_minutes += 1;

		if (tpp1_rtc_minutes == 60)
		{
		    tpp1_rtc_minutes = 0;
		    tpp1_rtc_hours += 1;

		    if (tpp1_rtc_hours == 24)
		    {
			tpp1_rtc_hours = 0;
			tpp1_rtc_day += 1;

			if (tpp1_rtc_day == 7)
			{
			    tpp1_rtc_day = 0;
			    tpp1_rtc_week += 1;

			    if (tpp1_rtc_week == 256)
			    {
				tpp1_rtc_week = 0;
				mr4_reg = setbit(mr4_reg, 3);
			    }
			}
		    }
		}
	    }
	}
    }
};
