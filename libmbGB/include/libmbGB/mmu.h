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

	    GBMBCType getMBCType(vector<uint8_t>&)
	    {
		return ROMOnly;
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