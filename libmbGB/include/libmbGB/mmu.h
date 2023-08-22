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
#include "apu.h"
#include "input.h"
#include "timers.h"
#include "serial.h"
#include "modules/mbc7eeprom.h"
using namespace eeprom;
using namespace gb;

namespace gb
{
    #include "mappers.inl"

    class LIBMBGB_API GBMMU
    {
	public:
	    GBMMU(GBGPU &gpu, GBInput &input, GBTimers &timer, GBSerial &link, GBAPU &apu);
	    ~GBMMU();

	    void init();
	    void shutdown();

	    void doSavestate(mbGBSavestate &file);

	    void setModel(GBModel model)
	    {
		model_type = model;
	    }

	    vector<uint8_t> saveBackup();
	    void loadBackup(vector<uint8_t> data);

	    void tick(int cycles);
	    void haltedtick(int cycles);
	    void tickInternal(bool is_running);

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

	    void debugOutput()
	    {
		graphics.debugOutput();
	    }

	    bool isBIOSLoad()
	    {
		return is_bios_load;
	    }

	    GBModel getModel()
	    {
		return model_type;
	    }

	    bool isCGB()
	    {
		return (model_type == ModelCgbX);
	    }

	    bool prepareSpeedSwitch()
	    {
		return is_prepare_switch;
	    }

	    void commitSpeedSwitch()
	    {
		if (is_prepare_switch)
		{
		    is_double_speed = !is_double_speed;
		    is_prepare_switch = false;
		}
	    }

	    bool isHDMAInProgress()
	    {
		return ((hdma_state == Starting) || (hdma_state == Active));
	    }

	    void updateHDMA()
	    {
		runHDMA();
		haltedtick(4);
	    }

	    void contextKeyChanged(GBContextButton button, bool is_pressed)
	    {
		mapper->contextKeyChanged(button, is_pressed);
	    }

	    void updateAccel(float xpos, float ypos)
	    {
		xpos = clamp<float>(xpos, -1.0f, 1.0f);
		ypos = clamp<float>(ypos, -1.0f, 1.0f);
		mapper->updateAccel(xpos, ypos);
	    }

	    void setRumbleCallback(rumblefunc cb)
	    {
		rumble_func = cb;
	    }

	private:
	    GBGPU &graphics;
	    GBInput &joypad;
	    GBTimers &timers;
	    GBSerial &serial;
	    GBAPU &audio;

	    bool is_odd_tick = false;

	    rumblefunc rumble_func;

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

	    array<uint8_t, 0x8000> wram;

	    uint8_t irq_enable = 0;
	    uint8_t irq_flags = 0;

	    uint8_t readWRAM(uint16_t addr)
	    {
		addr &= 0x1FFF;

		uint32_t wram_addr = addr;

		if (inRange(wram_addr, 0x1000, 0x2000))
		{
		    wram_addr = ((addr - 0x1000) + (wram_bank * 0x1000));
		}

		return wram.at(wram_addr);
	    }

	    void writeWRAM(uint16_t addr, uint8_t data)
	    {
		addr &= 0x1FFF;

		uint32_t wram_addr = addr;

		if (inRange(wram_addr, 0x1000, 0x2000))
		{
		    wram_addr = ((addr - 0x1000) + (wram_bank * 0x1000));
		}

		wram.at(wram_addr) = data;
	    }

	    void writeWRAMBank(uint8_t data)
	    {
		wram_bank = (data & 0x7);

		if (wram_bank == 0)
		{
		   wram_bank = 1;
		}
	    }

	    bool is_bios_load = false;

	    bool is_header_at_end = false;

	    mbGBMapper *mapper = NULL;
	    GBMBCType mbc_type;

	    uint32_t getHeaderOffs(vector<uint8_t> &rom)
	    {
		if (is_header_at_end)
		{
		    return (rom.size() < 0x8000) ? rom.size() : (rom.size() - 0x8000);
		}

		return 0;
	    }

	    uint8_t getROMByte(vector<uint8_t> &rom, uint32_t addr)
	    {
		uint32_t header_addr = getHeaderOffs(rom);
		return rom.at(header_addr + addr);
	    }

	    void detectHeaderEnd(vector<uint8_t> &rom)
	    {
		is_header_at_end = true;

		int logo_checksum = 0;

		for (int i = 0; i < 0x30; i++)
		{
		    logo_checksum += getROMByte(rom, (0x104 + i));
		}

		if (logo_checksum != 5446)
		{
		    is_header_at_end = false;
		}
	    }

	    GBMBCType getMBCType(vector<uint8_t> &rom, ostream &stream, int &flags)
	    {
		detectHeaderEnd(rom);
		GBMBCType mbc_type;

		uint8_t mapper_byte = getROMByte(rom, 0x0147);

		switch (mapper_byte)
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
		    case 0x0B:
		    {
			mbc_type = MMM01;
			stream << "MMM01";
			flags = 0x0;
		    }
		    break;
		    case 0x0C:
		    {
			mbc_type = MMM01;
			stream << "MMM01 + RAM";
			flags = 0x1;
		    }
		    break;
		    case 0x0D:
		    {
			mbc_type = MMM01;
			stream << "MMM01 + RAM + BATTERY";
			flags = 0x3;
		    }
		    break;
		    case 0x0F:
		    {
			mbc_type = MBC3;
			stream << "MBC3 + TIMER + BATTERY";
			flags = 0x6;
		    }
		    break;
		    case 0x10:
		    {
			if (compareTitle(rom, "TETRIS SET"))
			{
			    mbc_type = M161;
			    stream << "M161";
			    flags = 0x0;
			}
			else
			{
			    mbc_type = MBC3;
			    stream << "MBC3 + TIMER + RAM + BATTERY";
			    flags = 0x7;
			}
		    }
		    break;
		    case 0x11:
		    {
			if (is_header_at_end)
			{
			    mbc_type = MMM01;
			    stream << "MMM01";
			    flags = 0x00;
			}
			else
			{
			    mbc_type = MBC3;
			    stream << "MBC3";
			    flags = 0x0;
			}
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
		    case 0x22:
		    {
			mbc_type = MBC7;
			stream << "MBC7 + SENSOR + RAM + BATTERY";
			flags = 0x0;
		    }
		    break;
		    case 0xFC:
		    {
			mbc_type = PocketCamera;
			stream << "POCKET CAMERA";
			flags = 0x0;
		    }
		    break;
		    case 0xFE:
		    {
			mbc_type = HuC3;
			stream << "HuC3";
			flags = 0x0;
		    }
		    break;
		    case 0xFF:
		    {
			mbc_type = HuC1;
			stream << "HuC1 + RAM + BATTERY";
			flags = 0x0;
		    }
		    break;
		    default:
		    {
			stringstream ss;
			ss << "Unrecognized MBC type of " << hex << int(mapper_byte);
			throw runtime_error(ss.str());
		    }
		    break;
		}

		return mbc_type;
	    }

	    int getROMBanks(vector<uint8_t> &rom, ostream &stream)
	    {
		int num_rom_banks = (rom.size() / 0x4000);

		if (rom.size() >= 0x100000)
		{
		    int num_megs = (rom.size() / 0x100000);
		    stream << dec << num_megs << " MB";
		}
		else
		{
		    int num_kbs = (rom.size() / 0x400);
		    stream << dec << num_kbs << " KB";
		}

		return num_rom_banks;
	    }

	    int getRAMBanks(vector<uint8_t> &rom, ostream &stream)
	    {
		int num_ram_banks = 0;

		uint8_t ram_byte = getROMByte(rom, 0x0149);

		switch (ram_byte)
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
			ss << "Unrecognized RAM bank count of " << hex << int(ram_byte);
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

	    bool compareTitle(vector<uint8_t> &rom, string str)
	    {
		uint32_t title_offs = (getHeaderOffs(rom) + 0x134);
		return (strncmp((char*)&rom[title_offs], str.c_str(), 11) == 0);
	    }

	    bool isSonar(vector<uint8_t> &rom)
	    {
		return compareTitle(rom, "POCKETSONAR");
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

	    GBModel model_type = ModelAuto;

	    int bios_size = 0;

	    int wram_bank = 0;

	    bool is_double_speed = false;
	    bool is_prepare_switch = false;

	    uint16_t hdma_source = 0;
	    uint16_t hdma_dest = 0;
	    uint8_t hdma_control = 0;
	    int hdma_bytes_to_copy = 0;
	    int hdma_cycles = 0;
	    int hblank_bytes = 0;

	    void startHDMA();
	    void tickHDMA();
	    void runHDMA();
	    void signalHDMA();

	    GBDMAType hdma_type;
	    GBDMAState hdma_state = Inactive;
    };
};

#endif // LIBMBGB_MMU_H