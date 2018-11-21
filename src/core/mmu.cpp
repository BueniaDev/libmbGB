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
        cout << "MMU::Initialized" << endl;
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
		cout << "MMU::Exiting BIOS" << endl;
	    }
	}

	return memorymap[address];
    }

    void MMU::writeByte(uint16_t address, uint8_t value)
    {
	if (address == 0xFF44)
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
