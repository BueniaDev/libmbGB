#include "../../include/libmbGB/mmu.h"
#include <iostream>
#include <cstring>
using namespace gb;
using namespace std;

namespace gb
{
    MMU::MMU()
    {
        reset();
    }

    MMU::~MMU()
    {
        cout << "MMU::Shutting down..." << endl;
    }

    void MMU::reset()
    {
        memset(memorymap, 0, sizeof(memorymap));
	memset(bios, 0, sizeof(bios));

	biosload = false;
	resetmem();

        cout << "MMU::Initialized" << endl;
    }

    void MMU::resetmem()
    {
	if (biosload == false)
	{
	    memorymap[0xFF0F] = 0xE1;
	    memorymap[0xFF10] = 0x80;
	    memorymap[0xFF11] = 0xBF;
	    memorymap[0xFF12] = 0xF3;
	    memorymap[0xFF14] = 0xBF;
	    memorymap[0xFF16] = 0x3F;
	    memorymap[0xFF19] = 0xBF;
	    memorymap[0xFF1A] = 0x7F;
	    memorymap[0xFF1C] = 0x9F;
	    memorymap[0xFF1E] = 0xBF;
	    memorymap[0xFF20] = 0xFF;
	    memorymap[0xFF23] = 0xBF;
	    memorymap[0xFF24] = 0x77;
	    memorymap[0xFF25] = 0xF3;
	    memorymap[0xFF26] = 0xF1;
	    memorymap[0xFF40] = 0x91;
	    memorymap[0xFF47] = 0xFC;
	    memorymap[0xFF48] = 0xFF;
	    memorymap[0xFF49] = 0xFF;
	}
	else
	{
	    return;
	}
    }

    uint8_t MMU::readByte(uint16_t address)
    {
	if (biosload == true)
	{        
	    if (address < 0x0100)
	    {
		return bios[address];
	    }
	    else if (address == 0x0100)
	    {
		biosload = false;		
		cout << "MMU::Exiting BIOS" << endl;
	    }
	}

	return memorymap[address];
    }

    void MMU::writeByte(uint16_t address, uint8_t value)
    {
	if (address == 0xFF0F)
	{
	    value |= 0xE0;
	    memorymap[address] = value;
	}	
	else if (address == 0xFF44)
	{
	    memorymap[address] = 0;
	}        

	memorymap[address] = value;
    }

    uint16_t MMU::readWord(uint16_t address)
    {
        return (readByte(address + 1) << 8) | readByte(address);
    }

    void MMU::writeWord(uint16_t address, uint16_t value)
    {
        writeByte(address, (value & 0xFF));
        writeByte((address + 1), (value >> 8));
    }

    int8_t MMU::readsByte(uint16_t address)
    {
	return (int8_t)readByte(address);
    }
}
