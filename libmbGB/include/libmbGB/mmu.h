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

#ifndef LIBMBGB_MMU_H
#define LIBMBGB_MMU_H

#include "utils.h"
#include "gpu.h"
#include "input.h"
#include "timers.h"
using namespace gb;

namespace gb
{
    #include "mappers.inl"

    class LIBMBGB_API GBMMU
    {
	public:
	    GBMMU(GBGPU &gpu, GBInput &input, GBTimers &timer);
	    ~GBMMU();

	    void init();
	    void shutdown();

	    void tick(int cycles);
	    void haltedtick(int cycles);

	    uint8_t readByte(uint16_t addr);
	    void writeByte(uint16_t addr, uint8_t data);

	    bool loadBIOS(vector<uint8_t> data);
	    bool loadROM(vector<uint8_t> data);

	    bool isFrame();
	    void resetFrame();

	    uint8_t getIE()
	    {
		return irq_enable;
	    }

	    uint8_t getIF()
	    {
		return irq_flags;
	    }

	    void clearIRQ(int level)
	    {
		irq_flags = resetbit(irq_flags, level);
	    }

	private:
	    GBGPU &graphics;
	    GBInput &joypad;
	    GBTimers &timers;

	    int total_cycles = 0;

	    uint8_t readDirectly(uint16_t addr);
	    void writeDirectly(uint16_t addr, uint8_t data);

	    uint8_t readDMA(uint16_t addr);

	    uint8_t readIO(uint16_t addr);
	    void writeIO(uint16_t addr, uint8_t data);

	    void setIRQ(int level)
	    {
		irq_flags = setbit(irq_flags, level);
	    }

	    

	    vector<uint8_t> bios;

	    array<uint8_t, 0x7F> hram;

	    array<uint8_t, 0x2000> wram;

	    uint8_t irq_enable = 0;
	    uint8_t irq_flags = 0;

	    uint8_t readWRAM(uint16_t addr)
	    {
		addr &= 0x1FFF;
		return wram.at(addr);
	    }

	    void writeWRAM(uint16_t addr, uint8_t data)
	    {
		addr &= 0x1FFF;
		wram.at(addr) = data;
	    }

	    bool is_bios_load = false;

	    mbGBMapper *mapper = NULL;

	    GBMBCType getMBCType(vector<uint8_t> &rom, ostream &stream, int &flags)
	    {
		GBMBCType mbc_type;
		switch (rom[0x0147])
		{
		    case 0x00:
		    {
			mbc_type = ROMOnly;
			stream << "ROM ONLY";
			flags = 0x0;
		    }
		    break;
		    case 0x01:
		    {
			mbc_type = MBC1;
			stream << "MBC1";
			flags = 0x0;
		    }
		    break;
		    case 0x02:
		    {
			mbc_type = MBC1;
			stream << "MBC1 + RAM";
			flags = 0x1;
		    }
		    break;
		    case 0x03:
		    {
			mbc_type = MBC1;
			stream << "MBC1 + RAM + BATTERY";
			flags = 0x3;
		    }
		    break;
		    case 0x05:
		    {
			mbc_type = MBC2;
			stream << "MBC2";
			flags = 0x0;
		    }
		    break;
		    case 0x06:
		    {
			mbc_type = MBC2;
			stream << "MBC2 + BATTERY";
			flags = 0x2;
		    }
		    break;
		    case 0x08:
		    {
			mbc_type = ROMOnly;
			stream << "ROM + RAM";
			flags = 0x1;
		    }
		    break;
		    case 0x09:
		    {
			mbc_type = ROMOnly;
			stream << "ROM + RAM + BATTERY";
			flags = 0x3;
		    }
		    break;
		    case 0x11:
		    {
			mbc_type = MBC3;
			stream << "MBC3";
			flags = 0x0;
		    }
		    break;
		    case 0x12:
		    {
			mbc_type = MBC3;
			stream << "MBC3 + RAM";
			flags = 0x1;
		    }
		    break;
		    case 0x13:
		    {
			mbc_type = MBC3;
			stream << "MBC3 + RAM + BATTERY";
			flags = 0x3;
		    }
		    break;
		    case 0x19:
		    {
			mbc_type = MBC5;
			stream << "MBC5";
			flags = 0x0;
		    }
		    break;
		    case 0x1A:
		    {
			mbc_type = MBC5;
			stream << "MBC5 + RAM";
			flags = 0x1;
		    }
		    break;
		    case 0x1B:
		    {
			mbc_type = MBC5;
			stream << "MBC5 + RAM + BATTERY";
			flags = 0x3;
		    }
		    break;
		    case 0x1C:
		    {
			mbc_type = MBC5;
			stream << "MBC5 + RUMBLE";
			flags = 0x4;
		    }
		    break;
		    case 0x1D:
		    {
			mbc_type = MBC5;
			stream << "MBC5 + RUMBLE + RAM";
			flags = 0x5;
		    }
		    break;
		    case 0x1E:
		    {
			mbc_type = MBC5;
			stream << "MBC5 + RUMBLE + RAM + BATTERY";
			flags = 0x7;
		    }
		    break;
		    case 0xFC:
		    {
			mbc_type = PocketCamera;
			stream << "POCKET CAMERA";
			flags = 0x0;
		    }
		    break;
		    default:
		    {
			stringstream ss;
			ss << "Unrecognized MBC type of " << hex << int(rom[0x147]) << endl;
			throw runtime_error(ss.str());
		    }
		    break;
		}

		return mbc_type;
	    }

	    int getROMBanks(vector<uint8_t> &rom, ostream &stream)
	    {
		int num_rom_banks = 0;
		switch (rom[0x0148])
		{
		    case 0x0:
		    {
			num_rom_banks = 2;
			stream << "32 KB";
		    }
		    break;
		    case 0x1:
		    {
			num_rom_banks = 4;
			stream << "64 KB";
		    }
		    break;
		    case 0x2:
		    {
			num_rom_banks = 8;
			stream << "128 KB";
		    }
		    break;
		    case 0x3:
		    {
			num_rom_banks = 16;
			stream << "256 KB";
		    }
		    break;
		    case 0x4:
		    {
			num_rom_banks = 32;
			stream << "512 KB";
		    }
		    break;
		    case 0x5:
		    {
			num_rom_banks = 64;
			stream << "1 MB";
		    }
		    break;
		    case 0x6:
		    {
			num_rom_banks = 128;
			stream << "2 MB";
		    }
		    break;
		    case 0x7:
		    {
			num_rom_banks = 256;
			stream << "4 MB";
		    }
		    break;
		    case 0x8:
		    {
			num_rom_banks = 512;
			stream << "8 MB";
		    }
		    break;
		    default:
		    {
			stringstream ss;
			ss << "Unrecognized ROM bank count of " << hex << int(rom[0x148]) << endl;
			throw runtime_error(ss.str());
		    }
		    break;
		}

		return num_rom_banks;
	    }

	    int getRAMBanks(vector<uint8_t> &rom, ostream &stream)
	    {
		int num_ram_banks = 0;
		switch (rom[0x0149])
		{
		    case 0x0:
		    case 0x1:
		    {
			num_ram_banks = 0;
			stream << "None";
		    }
		    break;
		    case 0x2:
		    {
			num_ram_banks = 1;
			stream << "8 KB";
		    }
		    break;
		    case 0x3:
		    {
			num_ram_banks = 4;
			stream << "32 KB";
		    }
		    break;
		    case 0x4:
		    {
			num_ram_banks = 16;
			stream << "128 KB";
		    }
		    break;
		    case 0x5:
		    {
			num_ram_banks = 8;
			stream << "64 KB";
		    }
		    break;
		    default:
		    {
			stringstream ss;
			ss << "Unrecognized RAM bank count of " << hex << int(rom[0x149]) << endl;
			throw runtime_error(ss.str());
		    }
		    break;
		}

		return num_ram_banks;
	    }

	    bool isMulticart(vector<uint8_t> &rom)
	    {
		return ((rom.size() >= 0x44000) && (memcmp((char*)&rom[0x104], (char*)&rom[0x40104], 0x30) == 0));
	    }

	    bool isSonar(vector<uint8_t> &rom)
	    {
		return (strncmp((char*)&rom[0x134], "POCKETSONAR", 11) == 0);
	    }

	    uint8_t readMBC(uint16_t addr);
	    void writeMBC(uint16_t addr, uint8_t data);

	    uint8_t unmappedRead(uint16_t addr)
	    {
		throw mbGBUnmappedMemory(addr, false);
		return 0;
	    }

	    void unmappedWrite(uint16_t addr, uint8_t data)
	    {
		throw mbGBUnmappedMemory(addr, data, false);
		return;
	    }

	    void writeOAMDMA(uint8_t data);
	    void tickOAMDMA();
	    void runOAMDMA();

	    GBDMAState oam_dma_state = Inactive;
	    uint8_t oam_dma_byte = 0;
	    uint16_t oam_dma_addr = 0;
	    uint16_t oam_dma_dest = 0;
	    uint8_t oam_transfer_byte = 0;
	    int oam_dma_cycles = 0;
	    int oam_num_bytes = 160;
    };
};

#endif // LIBMBGB_MMU_H