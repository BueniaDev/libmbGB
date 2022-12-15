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

#include "mmu.h"
using namespace gb;

namespace gb
{
    GBMMU::GBMMU(GBGPU &gpu, GBInput &input, GBTimers &timer) : graphics(gpu), joypad(input), timers(timer)
    {

    }

    GBMMU::~GBMMU()
    {

    }

    void GBMMU::init()
    {
	auto irq_func = bind(&GBMMU::setIRQ, this, _1);
	auto hdma_func = bind(&GBMMU::signalHDMA, this);
	graphics.setIRQCallback(irq_func);
	graphics.setHDMACallback(hdma_func);
	timers.setIRQCallback(irq_func);

	wram_bank = 1;
    }

    void GBMMU::shutdown()
    {
	mapper->shutdown();
    }

    uint8_t GBMMU::readByte(uint16_t addr)
    {
	return readDirectly(addr);
    }

    void GBMMU::writeByte(uint16_t addr, uint8_t data)
    {
	writeDirectly(addr, data);
    }

    uint8_t GBMMU::readDirectly(uint16_t addr)
    {
	uint8_t data = 0;
	if (inRange(addr, 0, 0x4000))
	{
	    if (is_bios_load)
	    {
		if (isCGB() && inRange(addr, 0x100, 0x200))
		{
		    data = readMBC(addr);
		}
		else if (inRange(addr, 0, bios_size))
		{
		    data = bios.at(addr);
		}
		else
		{
		    data = readMBC(addr);
		}
	    }
	    else
	    {
		data = readMBC(addr);
	    }
	}
	else if (inRange(addr, 0x4000, 0x8000))
	{
	    data = readMBC(addr);
	}
	else if (inRange(addr, 0x8000, 0xA000))
	{
	    data = graphics.readVRAM(addr);
	}
	else if (inRange(addr, 0xA000, 0xC000))
	{
	    data = readMBC(addr);
	}
	else if (inRange(addr, 0xC000, 0xFE00))
	{
	    data = readWRAM(addr);
	}
	else if (inRange(addr, 0xFE00, 0xFEA0))
	{
	    data = graphics.readOAM(addr);
	}
	else if (inRange(addr, 0xFF00, 0xFF80))
	{
	    data = readIO(addr);
	}
	else if (inRange(addr, 0xFF80, 0xFFFF))
	{
	    data = hram.at(addr & 0x7F);
	}
	else if (addr == 0xFFFF)
	{
	    data = irq_enable;
	}
	else
	{
	    data = unmappedRead(addr);
	}

	return data;
    }

    void GBMMU::writeDirectly(uint16_t addr, uint8_t data)
    {
	if (inRange(addr, 0, 0x8000))
	{
	    writeMBC(addr, data);
	}
	else if (inRange(addr, 0x8000, 0xA000))
	{
	    graphics.writeVRAM(addr, data);
	}
	else if (inRange(addr, 0xA000, 0xC000))
	{
	    writeMBC(addr, data);
	}
	else if (inRange(addr, 0xC000, 0xFE00))
	{
	    writeWRAM(addr, data);
	}
	else if (inRange(addr, 0xFE00, 0xFEA0))
	{
	    graphics.writeOAM(addr, data);
	}
	else if (inRange(addr, 0xFEA0, 0xFF00))
	{
	    return;
	}
	else if (inRange(addr, 0xFF00, 0xFF80))
	{
	    writeIO(addr, data);
	}
	else if (inRange(addr, 0xFF80, 0xFFFF))
	{
	    hram.at(addr & 0x7F) = data;
	}
	else if (addr == 0xFFFF)
	{
	    irq_enable = (data & 0x1F);
	}
	else
	{
	    unmappedWrite(addr, data);
	}
    }

    uint8_t GBMMU::readIO(uint16_t addr)
    {
	uint8_t data = 0;
	uint8_t reg = (addr & 0xFF);

	if (reg == 0x00)
	{
	    data = joypad.readIO();
	}
	else if (reg == 0x02)
	{
	    data = 0xFF;
	}
	else if (inRangeEx(reg, 0x04, 0x07))
	{
	    data = timers.readIO(reg);
	}
	else if (reg == 0x0F)
	{
	    data = irq_flags;
	}
	else if (inRange(reg, 0x10, 0x40))
	{
	    // TODO: Audio
	    data = 0xFF;
	}
	else if (inRangeEx(reg, 0x40, 0x4B))
	{
	    data = graphics.readIO(addr);
	}
	else if (reg == 0x4D)
	{
	    if (isCGB())
	    {
		data = ((is_double_speed << 7) | is_prepare_switch);
	    }
	    else
	    {
		data = 0xFF;
	    }
	}
	else if (reg == 0x4F)
	{
	    if (isCGB())
	    {
		data = graphics.readCGBIO(addr);
	    }
	    else
	    {
		data = 0xFF;
	    }
	}
	else if (reg == 0x55)
	{
	    if (isCGB())
	    {
		data = hdma_control;
	    }
	    else
	    {
		data = 0xFF;
	    }
	}
	else if (reg == 0x70)
	{
	    if (isCGB())
	    {
		data = wram_bank;
	    }
	    else
	    {
		data = 0xFF;
	    }
	}
	else
	{
	    data = unmappedRead(addr);
	}

	return data;
    }

    void GBMMU::writeIO(uint16_t addr, uint8_t data)
    {
	uint8_t reg = (addr & 0xFF);

	if (reg == 0x00)
	{
	   joypad.writeIO(data);
	}
	else if (inRangeEx(reg, 0x01, 0x02))
	{
	    // TODO: Serial
	    return;
	}
	else if (inRangeEx(reg, 0x04, 0x07))
	{
	    timers.writeIO(reg, data);
	}
	else if (reg == 0xF)
	{
	    irq_flags = (data & 0x1F);
	}
	else if (inRange(reg, 0x10, 0x40))
	{
	    // TODO: Audio
	    return;
	}
	else if (reg == 0x46)
	{
	    writeOAMDMA(data);
	}
	else if (inRangeEx(reg, 0x40, 0x4B))
	{
	    graphics.writeIO(addr, data);
	}
	else if (reg == 0x4D)
	{
	    if (isCGB())
	    {
		is_prepare_switch = testbit(data, 0);
	    }
	}
	else if (reg == 0x4F)
	{
	    if (isCGB())
	    {
		graphics.writeCGBIO(addr, data);
	    }
	}
	else if (reg == 0x50)
	{
	    if (is_bios_load)
	    {
		cout << "Exiting BIOS..." << endl;
		is_bios_load = false;
	    }
	}
	else if (inRangeEx(reg, 0x51, 0x55))
	{
	    if (isCGB())
	    {
		switch (reg)
		{
		    case 0x51:
		    {
			hdma_source = ((data << 8) | (hdma_source & 0xFF));
		    }
		    break;
		    case 0x52:
		    {
			hdma_source = ((hdma_source & 0xFF00) | (data & 0xF0));
		    }
		    break;
		    case 0x53:
		    {
			hdma_dest = (((data & 0x1F) << 8) | (hdma_dest & 0xFF));
		    }
		    break;
		    case 0x54:
		    {
			hdma_dest = ((hdma_dest & 0xFF00) | (data & 0xF0));
		    }
		    break;
		    case 0x55:
		    {
			hdma_control = data;

			if (hdma_state == Inactive)
			{
			    startHDMA();
			}
			else
			{
			    if (testbit(hdma_control, 7))
			    {
				startHDMA();
			    }
			    else
			    {
				hdma_control = setbit(hdma_control, 7);
				hdma_bytes_to_copy = 0;
				hblank_bytes = 0;
				hdma_state = Inactive;
			    }
			}
		    }
		    break;
		    default: unmappedWrite(addr, data); break;
		}
	    }
	}
	else if (reg == 0x56)
	{
	    // TODO: Infrared
	    return;
	}
	else if (inRange(reg, 0x68, 0x6C))
	{
	    if (isCGB())
	    {
		graphics.writeCGBIO(addr, data);
	    }
	}
	else if (reg == 0x70)
	{
	    if (isCGB())
	    {
		writeWRAMBank(data);
	    }
	}
	else
	{
	    unmappedWrite(addr, data);
	}
    }

    void GBMMU::writeOAMDMA(uint8_t data)
    {
	oam_dma_byte = data;
	oam_dma_state = Starting;
	oam_dma_cycles = 4;
    }

    void GBMMU::tickOAMDMA()
    {
	if (oam_dma_state != Inactive)
	{
	    oam_dma_cycles -= 1;

	    if (oam_dma_cycles == 0)
	    {
		runOAMDMA();
		oam_dma_cycles = 4;
	    }
	}
	else
	{
	    oam_dma_cycles = 0;
	}
    }

    void GBMMU::runOAMDMA()
    {
	if (oam_dma_state == Starting)
	{
	    if (oam_num_bytes != 0)
	    {
		oam_dma_addr = (oam_dma_byte << 8);
		oam_num_bytes = 0;
	    }
	    else
	    {
		oam_transfer_byte = readDMA(oam_dma_addr);
		oam_num_bytes += 1;

		oam_dma_state = Active;
	    }
	}
	else if (oam_dma_state == Active)
	{
	    graphics.writeOAM((oam_num_bytes - 1), oam_transfer_byte);

	    if (oam_num_bytes == 160)
	    {
		oam_dma_state = Inactive;
		return;
	    }

	    oam_transfer_byte = readDMA(oam_dma_addr + oam_num_bytes);
	    oam_num_bytes += 1;
	}
    }

    void GBMMU::startHDMA()
    {
	hdma_type = testbit(hdma_control, 7) ? Hdma : Gdma;
	hdma_bytes_to_copy = (((hdma_control & 0x7F) + 1) * 16);
	hblank_bytes = 16;

	hdma_control &= 0x7F;

	if ((hdma_type == Hdma) && ((graphics.readIO(0xFF41) & 0x3) != 0))
	{
	    hdma_state = Paused;
	}
	else
	{
	    hdma_state = Starting;
	}

	hdma_cycles = 4;
    }

    void GBMMU::tickHDMA()
    {
	if (isHDMAInProgress())
	{
	    hdma_cycles -= 1;

	    if (hdma_cycles == 0)
	    {
		runHDMA();
		hdma_cycles = 4;
	    }
	}
	else
	{
	    hdma_cycles = 0;
	}
    }

    void GBMMU::runHDMA()
    {
	if (hdma_state == Starting)
	{
	    hdma_state = Active;
	}
	else if (hdma_state == Active)
	{
	    int num_bytes = min(2, hdma_bytes_to_copy);

	    if (hdma_type == Hdma)
	    {
		num_bytes = min(num_bytes, hblank_bytes);
		hblank_bytes -= num_bytes;
	    }

	    hdma_bytes_to_copy -= num_bytes;

	    for (int i = 0; i < num_bytes; i++)
	    {
		graphics.writeVRAM(hdma_dest, readDMA(hdma_source));

		hdma_dest = ((hdma_dest + 1) & 0x1FFF);
		hdma_source += 1;
	    }

	    hdma_control = (((hdma_bytes_to_copy / 16) - 1) & 0x7F);

	    if (hdma_bytes_to_copy == 0)
	    {
		hdma_control = 0xFF;
		hdma_state = Inactive;
	    }
	    else if ((hdma_type == Hdma) && (hblank_bytes == 0))
	    {
		hdma_state = Paused;
	    }
	}
    }

    void GBMMU::signalHDMA()
    {
	if (hdma_state == Paused)
	{
	    hblank_bytes = 16;
	    hdma_state = Starting;
	}
    }

    uint8_t GBMMU::readDMA(uint16_t addr)
    {
	uint8_t data = 0;
	if (addr < 0x8000)
	{
	    data = readDirectly(addr);
	}
	else if (inRange(addr, 0x8000, 0xA000))
	{
	    if (true)
	    {
		data = readDirectly(addr);
	    }
	    else
	    {
		data = 0xFF;
	    }
	}
	else if (inRange(addr, 0xA000, 0xE000))
	{
	    data = readDirectly(addr);
	}
	else
	{
	    if (false)
	    {
		data = readDirectly(addr - 0x4000);
	    }
	    else
	    {
		data = readDirectly(addr - 0x2000);
	    }
	}

	return data;
    }

    uint8_t GBMMU::readMBC(uint16_t addr)
    {
	return mapper->readByte(addr);
    }

    void GBMMU::writeMBC(uint16_t addr, uint8_t data)
    {
	mapper->writeByte(addr, data);
    }

    bool GBMMU::loadBIOS(vector<uint8_t> data)
    {
	if (data.empty())
	{
	    return false;
	}

	bios_size = data.size();

	if ((bios_size != 0x100) && (bios_size != 0x900))
	{
	    cout << "Invalid BIOS size" << endl;
	    return false;
	}

	model_type = (bios_size == 0x100) ? ModelDmgX : ModelCgbX;

	bios = vector<uint8_t>(data.begin(), data.end());
	is_bios_load = true;
	return true;
    }

    bool GBMMU::loadROM(vector<uint8_t> data)
    {
	if (data.empty())
	{
	    return false;
	}

	stringstream mbc_str;
	stringstream rom_str;
	stringstream ram_str;
	int flags = 0;

	if (model_type == ModelAuto)
	{
	    model_type = testbit(data[0x0143], 7) ? ModelCgbX : ModelDmgX;
	}

	GBMBCType mbc_type = getMBCType(data, mbc_str, flags);
	int num_rom_banks = getROMBanks(data, rom_str);
	int num_ram_banks = getRAMBanks(data, ram_str);

	cout << "MBC type: " << mbc_str.str() << endl;
	cout << "ROM amount: " << rom_str.str() << endl;
	cout << "RAM amount: " << ram_str.str() << endl;
	cout << endl;

	switch (mbc_type)
	{
	    case ROMOnly: mapper = new RomOnly(); break;
	    case MBC1:
	    {
		mapper = new GBMBC1();

		if (isMulticart(data))
		{
		    flags = setbit(flags, 2);
		}
		else if (isSonar(data))
		{
		    flags = setbit(flags, 3);
		}
	    }
	    break;
	    case MBC2: mapper = new GBMBC2(); break;
	    case MBC3: mapper = new GBMBC3(); break;
	    case MBC5: mapper = new GBMBC5(); break;
	    case PocketCamera: mapper = new GBCamera(); break;
	    case M161: mapper = new GBM161(); break;
	    default:
	    {
		throw runtime_error("Unsupported MBC type");
	    }
	    break;
	}

	mapper->configure(flags, num_rom_banks, num_ram_banks);
	mapper->init(data);
	return true;
    }

    void GBMMU::tick(int cycles)
    {
	for (int i = 0; i < cycles; i++)
	{
	    total_cycles += 1;

	    if (!is_double_speed || ((total_cycles % 2) != 0))
	    {
		graphics.tickGPU();
		tickHDMA();
	    }

	    timers.tickTimers();
	    tickOAMDMA();
	}
    }

    void GBMMU::haltedtick(int cycles)
    {
	for (int i = 0; i < cycles; i++)
	{
	    total_cycles += 1;
	    if (!is_double_speed || ((total_cycles % 2) != 0))
	    {
		graphics.tickGPU();
	    }

	    timers.tickTimers();
	}
    }

    bool GBMMU::isFrame()
    {
	return (total_cycles < (70224 << is_double_speed));
    }

    void GBMMU::resetFrame()
    {
	total_cycles = 0;
    }
};