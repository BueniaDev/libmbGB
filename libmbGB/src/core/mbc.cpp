#include "../../include/libmbGB/mmu.h"
#include <iostream>
#include <cstring>
using namespace gb;
using namespace std;

namespace gb
{
    uint8_t MMU::mbc1read(uint16_t address)
    {
        if ((address >= 0x4000) && (address < 0x8000))
        {
            uint16_t newaddr = (address - 0x4000);
            return cartmem[newaddr + (currentrombank * 0x4000)];
        }
        else if ((address >= 0xA000) && (address < 0xC000))
        {
            if (ramenabled)
            {
                if (rommode == 0)
                {
                    return rambanks[address - 0xA000];
                }
                else
                {
                    int ramaddress = 0;
                    if (currentrambank != 0)
                    {
                        ramaddress = currentrambank * 0x2000;
                    }
                    
                    return rambanks[(address - 0xA000) + ramaddress];
                }
            }
            else
            {
                return 0xFF;
            }
        }
        
	return memorymap[address];
    }

    void MMU::mbc1write(uint16_t address, uint8_t value)
    {
        if (address < 0x2000)
        {
            if (ramsize != 0)
            {
                ramenabled = ((value & 0x0F) == 0x0A);
            }
        }
        else if ((address >= 0x2000) && (address < 0x4000))
        {
            if (rommode == 0)
            {
                currentrombank = (value & 0x1F) | (higherrombankbits << 5);
            }
            else
            {
                currentrombank = value & 0x1F;
            }
            
            for (uint8_t specialbank : specialrombanks)
            {
                if (currentrombank == specialbank)
                {
                    currentrombank++;
                }
            }
            
            currentrombank &= (rombanks - 1);
        }
        else if ((address >= 0x4000) && (address < 0x6000))
        {
            if (rommode == 1)
            {
                currentrambank = value & 0x03;
                currentrambank &= (rambank - 1);
            }
            else
            {
                higherrombankbits = value & 0x03;
                currentrombank = (currentrombank & 0x1F) | (higherrombankbits << 5);
                
                for (uint8_t specialbank : specialrombanks)
                {
                    if (currentrombank == specialbank)
                    {
                        currentrombank++;
                    }
                }
                
                currentrombank &= (rombanks - 1);
            }
        }
        else if ((address >= 0x6000) && (address < 0x8000))
        {
            uint8_t data = (value & 0x01);
            rommode = (data == 0) ? 0 : 1;
            if (rommode == 0)
            {
                currentrambank = 0;
            }
        }
        else if ((address >= 0xA000) && (address < 0xC000))
        {
            if (ramenabled)
            {
                if (rommode == 0)
                {
                    rambanks[address - 0xA000] = value;
                }
                else
                {
                    uint16_t ramaddr = currentrambank * 0x2000;
                    rambanks[(address - 0xA000) + ramaddr] = value;
                }
            }
        }
        else
        {
            memorymap[address] = value;
        }
    }
    
    uint8_t MMU::mbc2read(uint16_t address)
    {
        if ((address >= 0x4000) && (address < 0x8000))
        {
            int romaddr = currentrombank * 0x4000;
            return cartmem[(address - 0x4000) + romaddr];
        }
        else if ((address >= 0xA000) && (address <= 0xA1FF))
        {
            if (ramenabled)
            {
                return rambanks[(address - 0xA000)];
            }
            else
            {
                return 0xFF;
            }
        }
        
        return memorymap[address];
    }
    
    void MMU::mbc2write(uint16_t address, uint8_t value)
    {
        if (address < 0x2000)
        {
            uint8_t upperbyte = (address >> 8);
            if (!TestBit(upperbyte, 0))
            {
                ramenabled = !ramenabled;
            }
        }
        else if ((address >= 0x2000) && (address < 0x4000))
        {
            uint8_t upperbyte = (address >> 8);
            if (TestBit(upperbyte, 0))
            {
                currentrombank = value & 0x0F;
            }
        }
        else if ((address >= 0xA000) && (address <= 0xA1FF))
        {
            if (ramenabled)
            {
                rambanks[(address - 0xA000)] = value;
            }
        }
        else
        {
            memorymap[address] = value;
        }
    }
    
    uint8_t MMU::mbc3read(uint16_t address)
    {
        if ((address >= 0x4000) && (address <= 0x7FFF))
        {
            uint16_t newaddr = (address - 0x4000);
            return cartmem[newaddr + (currentrombank * 0x4000)];
        }
        else if ((address >= 0xA000) && (address < 0xC000))
        {
            if (ramrtcselect < 0x04)
            {
          	currentrambank = ramrtcselect;      
	  	uint16_t ramaddr = currentrambank * 0x2000;
                return rambanks[(address - 0xA000) + ramaddr];
            }
	    else if (rtc && ramrtcselect >= 0x08 && ramrtcselect < 0x0D)
	    {
		updatetimer();

		switch (ramrtcselect)
		{
		    case 0x08: return latchsecs; break;
		    case 0x09: return latchmins; break;
		    case 0x0A: return latchhours; break;
		    case 0x0B: return latchdays; break;
		    case 0x0C: return latchdayshi; break;
		    default: break;
		}
	    }
        }

        return memorymap[address];
    }
    
    void MMU::mbc3write(uint16_t address, uint8_t value)
    {
        if (address < 0x2000)
        {
            if (ramsize != 0)
            {
                ramenabled = ((value & 0x0F) == 0x0A);
            }
        }
        else if ((address >= 0x2000) && (address < 0x4000))
        {
            currentrombank = (value & 0x7F);
            
            for (uint8_t specialbank : specialmbc3banks)
            {
                if (currentrombank == specialbank)
                {
                    currentrombank++;
                }
            }
            
            currentrombank &= (rombanks - 1);
        }
        else if ((address >= 0x4000) && (address < 0x6000))
        {
            ramrtcselect = (value % 0x0D);
        }
        else if ((address >= 0x6000) && (address < 0x8000))
        {
            if (!latch && TestBit(value, 0) && rtc)
	    {
		latchtimer();
	    }

	    latch = TestBit(value, 0);
        }
        else if ((address >= 0xA000) && (address < 0xC000))
        {
            if (ramrtcselect < 0x04)
            {
          	currentrambank = ramrtcselect;      
	  	uint16_t ramaddr = currentrambank * 0x2000;
                rambanks[(address - 0xA000) + ramaddr] = value;
            }
	    else if (rtc && ramrtcselect >= 0x08 && ramrtcselect < 0x0D)
	    {
		updatetimer();

		switch (ramrtcselect)
		{
		    case 0x08: realsecs = value; break;
		    case 0x09: realmins = value; break;
		    case 0x0A: realhours = value; break;
		    case 0x0B: realdays = value; break;
		    case 0x0C: realdayshi = value; break;
		    default: break;
		}
	    }
        }
    }
    
    uint8_t MMU::mbc5read(uint16_t address)
    {
        if ((address >= 0x4000) && (address < 0x8000))
        {
            uint16_t newaddr = (address - 0x4000);
            return cartmem[newaddr + (currentrombank * 0x4000)];
        }
        else if ((address >= 0xA000) && (address < 0xC000))
        {
            if (ramenabled)
            {
                int ramaddress = 0;
                if (currentrambank != 0)
                {
                    ramaddress = currentrambank * 0x2000;
                }
                
                return rambanks[(address - 0xA000) + ramaddress];
            }
            else
            {
                return 0xFF;
            }
        }
        
        return memorymap[address];
    }

    void MMU::updatetimer()
    {
	time_t newtime = time(nullptr);
	unsigned int difference = 0;

	if ((newtime > currenttime) & !TestBit(realdayshi, 6))
	{
	    difference = (unsigned int)(newtime - currenttime);
	    currenttime = newtime;
	}
	else
	{
	    currenttime = newtime;
	    return;
	}

	unsigned int newseconds = realsecs + difference;

	if (newseconds == realsecs)
	{
	    return;
	}

	realsecs = newseconds % 60;

	unsigned int newmins = realmins + (newseconds / 60);

	if (newmins == realmins)
	{
	    return;
	}

	realmins = newmins % 60;

	unsigned int newhours = realhours + (newmins / 60);

	if (newhours == realhours)
	{
	    return;
	}

	realhours = newhours % 24;

	unsigned int realdaysunsplit = ((realdayshi & 0x1) << 8) | realdays;
	unsigned int newdays = realdaysunsplit + (newhours / 24);
	if (newdays == realdaysunsplit)
	{
	    return;
	}

	realdays = newdays;
	
	realdayshi &= 0xFE;
	realdayshi |= (newdays >> 8) & 0x1;

	if (newdays > 511)
	{
	    realdayshi |= 0x80;
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
    
    void MMU::mbc5write(uint16_t address, uint8_t value)
    {
        if (address < 0x2000)
        {
            if (ramsize != 0)
            {
                ramenabled = ((value & 0x0F) == 0x0A);
            }
        }
        else if ((address >= 0x2000) && (address < 0x3000))
        {
            currentrombank = (currentrombank & 0x100) | value;
        }
        else if ((address >= 0x3000) && (address < 0x4000))
        {
            currentrombank = (currentrombank & 0xFF) | ((value & 0x1) << 9);
        }
        else if ((address >= 0x4000) && (address < 0x6000))
        {
            currentrambank = (value & 0x0F);
        }
        else if ((address >= 0xA000) && (address < 0xC000))
        {
            if (ramenabled && ramsize > 0)
            {
                uint16_t ramaddr = currentrambank * 0x2000;
                rambanks[(address - 0xA000) + ramaddr] = value;
            }
        }
        else
        {
            memorymap[address] = value;
        }
    }
}
