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
		ramenabled = ((value & 0x0F) == 0x0A);
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
	    else
	    {
		// TODO: RTC
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
		ramenabled = ((value & 0x0F) == 0x0A);
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
		// TODO: RTC
	    }
	}
	else if (addr < 0x8000)
	{
	    // TODO: RTC
	}
	else if ((addr >= 0xA000) && (addr < 0xC000))
	{
	    if (currentrambank <= 3)
	    {
		uint16_t currentaddr = (currentrambank * 0x2000);
		rambanks[(addr - 0xA000) + currentaddr] = value;
	    }
	    else
	    {
		// TODO: RTC
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
		ramenabled = ((value & 0x0F) == 0x0A);
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
};
