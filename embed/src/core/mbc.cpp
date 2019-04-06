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
            currentrambank = (value & 0x07);
        }
        else if ((address >= 0x6000) && (address < 0x8000))
        {
            // TODO: RTC
        }
        else if ((address >= 0xA000) && (address < 0xC000))
        {
            if (ramenabled)
            {
                uint16_t ramaddr = currentrambank * 0x2000;
                rambanks[(address - 0xA000) + ramaddr] = value;
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
            if (ramenabled)
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
