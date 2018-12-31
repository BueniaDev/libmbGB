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
}
