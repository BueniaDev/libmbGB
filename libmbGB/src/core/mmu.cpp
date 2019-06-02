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
        memset((memorymap + 0x8000), 0, 0x8000);
        memset(cartmem, 0, sizeof(cartmem));
        memset(rambanks, 0, sizeof(rambanks));
        memset(vram, 0, sizeof(vram));
        memset(wram, 0, sizeof(wram));
        memset(gbcbgpallete, 0, sizeof(gbcbgpallete));
        memset(gbcobjpallete, 0, sizeof(gbcobjpallete));
        
        fstream file(filename.c_str(), ios::in | ios::binary);
        
        if (!file.is_open())
        {
            cout << "Error opening memory state" << endl;
            return false;
        }
        
        file.read((char*)&memorymap[0x8000], 0x8000);
        file.read((char*)&cartmem[0], 0x800000);
        file.read((char*)&rambanks[0], 0x8000);
        file.read((char*)&vram[0], 0x4000);
        file.read((char*)&wram[0], 0x8000);
        file.read((char*)&gbcbgpallete[0], 0x40);
        file.read((char*)&gbcobjpallete[0], 0x40);
	file.read((char*)&doublespeed, sizeof(doublespeed));
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
        
        file.write((char*)&memorymap[0x8000], 0x8000);
        file.write((char*)&cartmem[0], 0x800000);
        file.write((char*)&rambanks[0], 0x8000);
        file.write((char*)&vram[0], 0x4000);
        file.write((char*)&wram[0], 0x8000);
        file.write((char*)&gbcbgpallete[0], 0x40);
        file.write((char*)&gbcobjpallete[0], 0x40);
	file.write((char*)&doublespeed, sizeof(doublespeed));
        file.close();
        return true;
    }

    void MMU::reset()
    {
        memset(memorymap, 0, sizeof(memorymap));
        memset(cartmem, 0, sizeof(cartmem));
        memset(rambanks, 0, sizeof(rambanks));
        memset(bios, 0, sizeof(bios));
        memset(vram, 0, sizeof(vram));
        memset(wram, 0, sizeof(wram));
        memset(gbcbgpallete, 0, sizeof(gbcbgpallete));
        memset(gbcobjpallete, 0, sizeof(gbcobjpallete));

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

	if (isgbcenabled)
	{
	    memorymap[0xFF68] = 0xC0;
	    memorymap[0xFF69] = 0x88;
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

    uint8_t MMU::readVram(uint16_t address)
    {
	uint16_t offset = (vrambank1 * 0x2000);	
	return vram[(address - 0x8000) + offset];
    }

    void MMU::writeVram(uint16_t address, uint8_t value)
    {	
	uint16_t offset = (vrambank1 * 0x2000);	
	vram[(address - 0x8000) + offset] = value;
    }

    uint8_t MMU::readWram(uint16_t address)
    {
	uint16_t offset = (wrambank * 0x1000);	
	return wram[(address - 0xD000) + offset];
    }

    void MMU::writeWram(uint16_t address, uint8_t value)
    {
	uint16_t offset = (wrambank * 0x1000);	
	wram[(address - 0xD000) + offset] = value;
    }

    uint8_t MMU::readByte(uint16_t address)
    {
	if (biosload == true)
	{        
	    if ((biossize == 0x900) && (address > 0x100) && (address < 0x200))
	    {
		return memorymap[address];
	    }
	    else if (address == 0x100)
	    {
		biosload = false;
		cout << "MMU::Exiting BIOS..." << endl;
	    }
	    else if (address < biossize)
	    {
		return bios[address];
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
	else if ((address >= 0x8000) && (address < 0xA000))
	{
	    return readVram(address);
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
    else if ((address >= 0xC000) && (address < 0xD000))
    {
	return wram[address - 0xC000];
    }
	else if ((address >= 0xD000) && (address < 0xE000))
	{
	    return readWram(address);
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
	else if ((address >= 0xFE00) && (address < 0xFEA0))
	{
	    return memorymap[address];
	}
	else if ((address >= 0xFEA0) && (address <= 0xFEFF))
	{
	    return 0xFF;
	}
	else if (address == 0xFF4F)
	{
	    return vrambank1;
	}
	else if (address == 0xFF68)
	{
	    return gbcbgpalleteindex;
	}
	else if (address == 0xFF69)
	{
	    return gbcbgpallete[gbcbgpalleteindex];
	}
	else if (address == 0xFF6A)
	{
	    return gbcobjpalleteindex;
	}
	else if (address == 0xFF6B)
	{
	    return gbcobjpallete[gbcobjpalleteindex];
	}
	else if (address == 0xFF70)
	{
	    return (isgbcenabled) ? (uint8_t)(wrambank) : 0xFF;
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
	else if ((address >= 0x8000) && (address < 0xA000))
	{
	    writeVram(address, value);
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
    else if ((address >= 0xC000) && (address < 0xD000))
    {
	wram[address - 0xC000] = value;
    }
	else if ((address >= 0xD000) && (address < 0xE000))
	{
	    writeWram(address, value);
	}
	else if ((address >= 0xE000) && (address < 0xFE00))
	{
	    memorymap[address] = value;
	    writeByte((address - 0x2000), value);
	}
	else if ((address >= 0xFE00) && (address < 0xFEA0))
	{
	    memorymap[address] = value;
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
	else if (address == 0xFF41)
	{
	    uint8_t temp = memorymap[address];
	    memorymap[address] = ((temp & 0x7) | (value & 0xF8));
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
	else if ((address == 0xFF4D && isgbcenabled))
	{
	    memorymap[address] = value;
	}
	else if (address == 0xFF4F)
	{
	    vrambank1 = (isgbcenabled) ? BitGetVal(value, 0) : 0;
	    memorymap[address] = (!isgbcenabled) ? 0xFF : (value & 0x1);
	}
	else if (address == 0xFF51)
	{
	    if (!isgbcenabled)
	    {
		return;
	    }

	    hdmasource = (hdmasource & 0xFF) | (value << 8);
	}
	else if (address == 0xFF52)
	{
	    if (!isgbcenabled)
	    {
		return;
	    }

	    hdmasource = (hdmasource & 0xFF00) | (value & 0xF0);
	}
	else if (address == 0xFF53)
	{
	    if (!isgbcenabled)
	    {
		return;
	    }

	    hdmadest = (hdmadest & 0xFF) | (((value & 0x1F) | 0x80) << 8);
	}
	else if (address == 0xFF54)
	{
	    if (!isgbcenabled)
	    {
		return;
	    }

	    hdmadest = (hdmadest & 0xFF00) | (value & 0xF0);
	}
	else if (address == 0xFF55)
	{
	    if (!isgbcenabled)
	    {
		return;
	    }

	    hdmalength = (value & 0x7F);

	    if (!TestBit(value, 7) && !hdmaactive)
	    {
		for (int i = 0; i <= (hdmalength); i++)
		{
		    hdmatransfer();
		}

		hdmalength = 0xFF;
		hdmaactive = false;
	    }
	    else if (!TestBit(value, 7) && hdmaactive)
	    {
		hdmaactive = false;
	    }
	    else
	    {
		hdmaactive = true;
	    }
	}
	else if (address == 0xFF68)
	{
	    gbcbgpalleteindex = (value & 0x3F);
	    gbcbgpallinc = TestBit(value, 7);
	}
	else if (address == 0xFF69)
	{
	    gbcbgpallete[gbcbgpalleteindex] = value;

	    if (gbcbgpallinc)
	    {
		gbcbgpalleteindex = ((gbcbgpalleteindex + 1) & 0x3F);
	    }
	}
	else if (address == 0xFF6A)
	{
	    gbcobjpalleteindex = (value & 0x3F);
	    gbcobjpallinc = TestBit(value, 7);
	}
	else if (address == 0xFF6B)
	{
	    gbcobjpallete[gbcobjpalleteindex] = value;

	    if (gbcobjpallinc)
	    {
		gbcobjpalleteindex = ((gbcobjpalleteindex + 1) & 0x3F);
	    }
	}
	else if (address == 0xFF70)
	{
	    if (!isgbcenabled)
	    {
		return;
	    }

	    wrambank = ((value & 0x07) != 0) ? (value & 0x07) : 1;
	}
	else if (address == 0xFFFF)
	{
	    memorymap[address] = value;
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
            case 16: temp = 3; rtc = true; cout << "Type: MBC3 + TIMER + RAM + BATTERY" << endl; break;
	    case 17: temp = 3; cout << "Type: MBC3" << endl; break;
            case 18: temp = 3; cout << "Type: MBC3 + RAM" << endl; break;
            case 19: temp = 3; cout << "Type: MBC3 + RAM + BATTERY" << endl; break;
            case 25: temp = 5; cout << "Type: MBC5" << endl; break;
            case 26: temp = 5; cout << "Type: MBC5 + RAM" << endl; break;
            case 27: temp = 5; cout << "Type: MBC5 + RAM + BATTERY" << endl; break;
	    case 30: temp = 5; cout << "Type: MBC5 + RUMBLE + RAM + BATTERY" << endl; break;
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

    void MMU::hdmatransfer()
    {
	for (int i = 0; i < 0x10; i++)
	{
	    writeByte(hdmadest, readByte(hdmasource));

	    hdmadest += 1;
	    hdmasource += 1;
	}
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
	uint8_t cgbflag = cartmem[0x0143];


	if (((gbtype == 0) || (gbtype == 2)) && !ismanual)
	{
	    if (cgbflag == 0x00)
	    {
		gbtype = 1;
	    }
	    else if (cgbflag == 0x80)
	    {
		gbtype = 2;
	    }
	    else if (cgbflag == 0xC0)
	    {
		gbtype = 2;
	    }
	    else
	    {
		gbtype = 1;
	    }
	}

	if (gbtype == 1)
	{
	    isgbcenabled = false;
	}
	else
	{
	    isgbcenabled = true;
	}

	cout << "GBC Enabled? " << ((isgbcenabled) ? "Yes" : "No") << endl;

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

	    if (size == 0x100)
	    {
		gbtype = 1;
	    }
	    else if (size == 0x900)
	    {
		gbtype = 0;
		isgbcbios = true;
	    }
	    else
	    {
		cout << "MMU::BIOS has an incorrect size of " << size << " bytes." << endl;
		return false;
	    }

	    if (isgbcbios)
	    {
		isgbcenabled = true;
	    }

	    biossize = size;

	    file.seekg(0, ios::beg);
	    file.read((char*)&bios[0], size);
	    file.close();
	    cout << "BIOS succesfully loaded." << endl;
	    biosload = true;
	    return true;
	}
	else
	{
	    cout << "MMU::BIOS could not be opened. Check file path or permissions." << endl;
	    return false;
	}
    }
}
