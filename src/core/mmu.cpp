#include "../../include/libmbGB/mmu.h"
#include <iostream>
#include <fstream>
#include <cstring>
#include <string>
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
    
    bool MMU::loadmmu(string filename)
    {
        memset(memorymap, 0, sizeof(memorymap));
        memset(cartmem, 0, sizeof(cartmem));
        memset(rambanks, 0, sizeof(rambanks));
        
        fstream file(filename.c_str(), ios::in | ios::binary);
        
        if (!file.is_open())
        {
            cout << "Error opening memory state" << endl;
            return false;
        }
        
        file.read((char*)&memorymap[0], 0x10000);
        file.read((char*)&cartmem[0], 0x200000);
        file.read((char*)&rambanks[0], 0x8000);
        file.close();
        return true;
    }
    
    bool MMU::savemmu(string filename)
    {
        fstream file(filename.c_str(), ios::out | ios::binary);
        
        if (!file.is_open())
        {
            cout << "Error opening save state" << endl;
            return false;
        }
        
        file.write((char*)&memorymap[0], 0x10000);
        file.write((char*)&cartmem[0], 0x200000);
        file.write((char*)&rambanks[0], 0x8000);
        file.close();
        return true;
    }

    void MMU::reset()
    {
        memset(memorymap, 0, sizeof(memorymap));
        memset(cartmem, 0, sizeof(cartmem));
        memset(rambanks, 0, sizeof(rambanks));
        memset(bios, 0, sizeof(bios));

        biosload = false;
        resetmem();

        cout << "MMU::Initialized" << endl;
    }

    void MMU::resetmem()
    {
	if (biosload == false)
	{
	    memorymap[0xFF0F] = 0xE0;
	    memorymap[0xFF10] = 0x80;
	    memorymap[0xFF11] = 0xBF;
	    memorymap[0xFF12] = 0xF3;
	    memorymap[0xFF14] = 0xBF;
	    memorymap[0xFF16] = 0x3F;
	    memorymap[0xFF19] = 0xBF;
	    memorymap[0xFF1A] = 0x7F;
	    memorymap[0xFF1B] = 0xFF;
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
    
    uint8_t MMU::readDirectly(uint16_t address)
    {
        return memorymap[address];
    }
    
    void MMU::writeDirectly(uint16_t address, uint8_t value)
    {
        memorymap[address] = value;
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
	
	if (address < 0x8000)
	{
        switch (mbctype)
        {
            case 0: return memorymap[address]; break;
            case 1: return mbc1read(address); break;
            case 2: return mbc2read(address); break;
            case 3: return mbc3read(address); break;
            case 5: return mbc5read(address); break;
        }
	}
    else if ((address >= 0xA000) && (address < 0xC000))
    {
        switch (mbctype)
        {
            case 0:
            {
                if (ramsize != 0)
                {
                    return memorymap[address];
                }
                else
                {
                    return 0xFF;
                }
            }
            break;
            case 1: return mbc1read(address); break;
            case 2: return mbc2read(address); break;
            case 3: return mbc3read(address); break;
            case 5: return mbc5read(address); break;
        }
    }
	else if (address == 0xFF00)
	{
	    return joypad->getjoypadstate();
	}
	else if (address == 0xFF0F)
	{
	    uint8_t value = memorymap[address];
	    value |= 0xE0;
	    return value;
	}
	else if ((address >= 0xFF10) && (address <= 0xFF3F))
	{
	    return audio->readapu(address);
	}
	else if ((address >= 0xFEA0) && (address <= 0xFEFF))
	{
	    return 0xFF;
	}

	return readDirectly(address);
    }

    void MMU::writeByte(uint16_t address, uint8_t value)
    {	
	if (address < 0x8000)
    {
        switch (mbctype)
        {
            case 0: return; break;
            case 1: mbc1write(address, value); break;
            case 2: mbc2write(address, value); break;
            case 3: mbc3write(address, value); break;
            case 5: mbc5write(address, value); break;
        }
    }
    else if ((address >= 0xA000) && (address < 0xC000))
    {
        switch (mbctype)
        {
            case 0:
            {
                if (ramsize != 0)
                {
                    memorymap[address] = value;
                }
                else
                {
                    return;
                }
            }
            break;
            case 1: mbc1write(address, value); break;
            case 2: mbc2write(address, value); break;
            case 3: mbc3write(address, value); break;
            case 5: mbc5write(address, value); break;
        }
    }
	else if ((address >= 0xE000) && (address < 0xFE00))
	{
	    memorymap[address] = value;
	    writeByte(address - 0x2000, value);
	}
	else if ((address >= 0xFEA0) && (address <= 0xFEFF))
	{
	    return;
	}
	else if (address == 0xFF00)
	{
	    joypad->write(value);
	}
	else if (address == 0xFF04)
	{
	    memorymap[address] = 0;
	}
	else if ((address >= 0xFF10) && (address <= 0xFF3F))
	{
	    audio->writeapu(address, value);
	}
	else if (address == 0xFF44)
	{
	    return;
	}
	else if (address == 0xFF46)
	{
	    uint16_t addr = (value << 8);

	    for (uint16_t i = 0; i < 0xA0; i++)
	    {
		memorymap[0xFE00 + i] = readByte(addr + i);
	    }
	}
    else
    {
        writeDirectly(address, value);
    }
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
    
    int MMU::getmbctype(uint8_t mbcval)
    {
        int temp = 0;
        switch (mbcval)
        {
            case 0: temp = 0; cout << "Type: ROM_ONLY" << endl; break;
            case 1: temp = 1; cout << "Type: MBC1" << endl; break;
            case 2: temp = 1; cout << "Type: MBC1 + RAM" << endl; break;
            case 3: temp = 1; cout << "Type: MBC1 + RAM + BATTERY" << endl; break;
            case 5: temp = 2; cout << "Type: MBC2" << endl; break;
            case 6: temp = 2; cout << "Type: MBC2 + BATTERY" << endl; break;
            case 8: temp = 0; cout << "Type: ROM + RAM" << endl; break;
            case 9: temp = 0; cout << "Type: ROM + RAM + BATTERY" << endl; break;
            case 17: temp = 3; cout << "Type: MBC3" << endl; break;
            case 18: temp = 3; cout << "Type: MBC3 + RAM" << endl; break;
            case 19: temp = 3; cout << "Type: MBC3 + RAM + BATTERY" << endl; break;
            case 25: temp = 5; cout << "Type: MBC5" << endl; break;
            case 26: temp = 5; cout << "Type: MBC5 + RAM" << endl; break;
            case 27: temp = 5; cout << "Type: MBC5 + RAM + BATTERY" << endl; break;
        }
        
        return temp;
    }
    
    int MMU::getramsize(uint8_t ramval)
    {
        int temp = 0;
        switch (ramval)
        {
            case 0: temp = 0; cout << "RAM: None" << endl; break;
            case 1: temp = 2; cout << "RAM: 2 KB" << endl; break;
            case 2: temp = 8; cout << "RAM: 8 KB" << endl; break;
            case 3: temp = 32; cout << "RAM: 32 KB" << endl; break;
        }
        
        return temp;
    }
    
    int MMU::getrombanks(uint8_t romval)
    {
        bool ismbc1 = ((cartmem[0x0147] >= 1) && (cartmem[0x0147] <= 3)) ? true : false;
        
        int banks = 0;
        switch (romval)
        {
            case 0: banks = 0; cout << "Size: 32 KB" << endl; break;
            case 1: banks = 4; cout << "Size: 64 KB" << endl; break;
            case 2: banks = 8; cout << "Size: 128 KB" << endl;break;
            case 3: banks = 16; cout << "Size: 256 KB" << endl; break;
            case 4: banks = 32; cout << "Size: 512 KB" << endl; break;
            case 5: banks = ismbc1 ? 63 : 64; cout << "Size: 1 MB" << endl; break;
            case 6: banks = ismbc1 ? 125: 128; cout << "Size: 2 MB" << endl; break;
            case 7: banks = 256; cout << "Size: 4 MB" << endl; break;
            case 82: banks = 72; cout << "Size: 1.1 MB" << endl; break;
            case 83: banks = 80; cout << "Size: 1.2 MB" << endl; break;
            case 84: banks = 96; cout << "Size: 1.5 MB" << endl; break;
        }
        
        return banks;
    }
    
    int MMU::getrambanks(int rambankval)
    {
        int banks = 0;
        
        switch (rambankval)
        {
            case 0: banks = 0; break;
            case 2: banks = 0; break;
            case 8: banks = 0; break;
            case 32: banks = 4; break;
        }
        
        return banks;
    }

    bool MMU::loadROM(string filename)
    {
	streampos size;

	cout << "Loading ROM: " << filename << endl;
	
	ifstream file(filename.c_str(), ios::in | ios::binary | ios::ate);

	if (file.is_open())
	{
	    size = file.tellg();

	    file.seekg(0, ios::beg);
	    file.read((char*)&cartmem[0], size);
	    file.close();
	    
        mbctype = getmbctype(cartmem[0x0147]);
        rombanks = getrombanks(cartmem[0x0148]);
        ramsize = getramsize(cartmem[0x0149]);
        rambank = getrambanks(ramsize);

	    if (notmbc == true)
	    {
		cout << "MMU::Error - MBC type not supported." << endl;
		return false;
	    }

	    memcpy(&memorymap[0x0], &cartmem[0], 0x8000);
	    cout << "MMU::" << filename << " succesfully loaded." << endl;
	    return true;
	}
	else
	{
	    cout << "MMU::" << filename << " could not be opened. Check file path or permissions." << endl;
	    return false;
	}
    }

    bool MMU::loadBIOS(string filename)
    {
	streampos size;

	cout << "Loading BIOS: " << filename << endl;
	
	ifstream file(filename.c_str(), ios::in | ios::binary | ios::ate);

	if (file.is_open())
	{
	    size = file.tellg();

	    if (size > 256)
	    {
		cout << "MMU::Error - BIOS is too big." << endl;
		return false;
	    }

	    file.seekg(0, ios::beg);
	    file.read((char*)&bios[0], size);
	    file.close();
	    cout << "BIOS succesfully loaded." << endl;
	    return true;
	}
	else
	{
	    cout << "MMU::BIOS could not be opened. Check file path or permissions." << endl;
	    return false;
	}
    }
}
