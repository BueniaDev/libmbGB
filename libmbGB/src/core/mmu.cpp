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
	graphics.setIRQCallback(irq_func);
	timers.setIRQCallback(irq_func);
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
		if (inRange(addr, 0, 0x100))
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
	else if (inRangeEx(reg, 0x40, 0x4B))
	{
	    data = graphics.readIO(addr);
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
	else if (reg == 0x50)
	{
	    if (is_bios_load)
	    {
		cout << "Exiting BIOS..." << endl;
		is_bios_load = false;
	    }
	}
	else if (inRange(reg, 0x70, 0x80))
	{
	    return;
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

	if (data.size() != 0x100)
	{
	    cout << "Invalid BIOS size" << endl;
	    return false;
	}

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

	GBMBCType mbc_type = getMBCType(data);

	switch (mbc_type)
	{
	    case ROMOnly: mapper = new RomOnly(); break;
	    default:
	    {
		throw runtime_error("Unsupported MBC type");
	    }
	    break;
	}

	mapper->init(data);
	return true;
    }

    void GBMMU::tick(int cycles)
    {
	total_cycles += cycles;
	for (int i = 0; i < cycles; i++)
	{
	    graphics.tickGPU();
	    timers.tickTimers();
	    tickOAMDMA();
	}
    }

    void GBMMU::haltedtick(int cycles)
    {
	total_cycles += cycles;
	for (int i = 0; i < cycles; i++)
	{
	    graphics.tickGPU();
	    timers.tickTimers();
	}
    }

    bool GBMMU::isFrame()
    {
	return (total_cycles < 70224);
    }

    void GBMMU::resetFrame()
    {
	total_cycles = 0;
    }
};