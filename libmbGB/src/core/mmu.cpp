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
using namespace std::placeholders;

namespace gb
{
    MMU::MMU()
    {
	addmemoryreadhandler(0xFF03, bind(&MMU::readempty, this, _1));
	addmemorywritehandler(0xFF03, bind(&MMU::writeempty, this, _1, _2));

	for (int i = 0x8; i < 0xF; i++)
	{
	    addmemoryreadhandler((0xFF00 + i), bind(&MMU::readempty, this, _1));
	    addmemorywritehandler((0xFF00 + i), bind(&MMU::writeempty, this, _1, _2));
	}
    }

    MMU::~MMU()
    {

    }

    void MMU::init()
    {
	vram.resize(0x4000, 0);
	sram.resize(0x2000, 0);
	wram.resize(0x8000, 0);
	oam.resize(0xA0, 0);
	hram.resize(0x7F, 0);

	if (gbmbc == MBCType::TPP1)
	{
	    mr0_reg = 1;
	    mr1_reg = 0;
	    mr2_reg = 0;
	    map_control = TPP1Mapped::ControlReg;
	}

	gbcbgpalette.resize(0x40, 0xFF);
	gbcobjpalette.resize(0x40, 0xFF);

	currentrombank = 1;
	currentrambank = 0;
	wisdomrombank = 0;

	// Initialize IO registers
	interruptflags = 0x00;
	dma = 0x00;
	key1 = 0x00;
	interruptenabled = 0x00;
	dmaactive = false;

	if (!biosload)
	{
	    interruptenabled = 0x00;
	    interruptflags = 0xE0;
	    vrambank = 0;
	    wrambank = 1;
	    writeByte(0xFF4D, 0x7E);
	    doublespeed = false;
	}

	sensorx = 0;
	sensory = 0;
	mbc7_sensorx = 0x8000;
	mbc7_sensory = 0x8000;
	mbc7_state = MBC7State::StartBit;
	mbc7_gyro_val = 0;

	cout << "MMU::Initialized" << endl;
    }

    void MMU::initvram()
    {
	if (isdmgmode()) 
	{
	    int inittilemap = 0x19;
	    vram[0x1910] = inittilemap--;
	    for (int addr = 0x192F; addr >= 0x1924; --addr) 
	    {
		vram[addr] = inittilemap--;
	    }

	    for (int addr = 0x190F; addr >= 0x1904; --addr) 
	    {
		vram[addr] = inittilemap--;
	    }
	}

        array<uint8_t, 200> inittiledata{{
  	    0xF0, 0xF0, 0xFC, 0xFC, 0xFC, 0xFC, 0xF3, 0xF3,
            0x3C, 0x3C, 0x3C, 0x3C, 0x3C, 0x3C, 0x3C, 0x3C,
            0xF0, 0xF0, 0xF0, 0xF0, 0x00, 0x00, 0xF3, 0xF3,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xCF, 0xCF,
            0x00, 0x00, 0x0F, 0x0F, 0x3F, 0x3F, 0x0F, 0x0F,
            0x00, 0x00, 0x00, 0x00, 0xC0, 0xC0, 0x0F, 0x0F,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xF0, 0xF0,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xF3, 0xF3,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xC0, 0xC0,
            0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0xFF, 0xFF,
            0xC0, 0xC0, 0xC0, 0xC0, 0xC0, 0xC0, 0xC3, 0xC3,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFC, 0xFC,
            0xF3, 0xF3, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0,
            0x3C, 0x3C, 0xFC, 0xFC, 0xFC, 0xFC, 0x3C, 0x3C,
            0xF3, 0xF3, 0xF3, 0xF3, 0xF3, 0xF3, 0xF3, 0xF3,
            0xF3, 0xF3, 0xC3, 0xC3, 0xC3, 0xC3, 0xC3, 0xC3,
            0xCF, 0xCF, 0xCF, 0xCF, 0xCF, 0xCF, 0xCF, 0xCF,
            0x3C, 0x3C, 0x3F, 0x3F, 0x3C, 0x3C, 0x0F, 0x0F,
            0x3C, 0x3C, 0xFC, 0xFC, 0x00, 0x00, 0xFC, 0xFC,
            0xFC, 0xFC, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0,
            0xF3, 0xF3, 0xF3, 0xF3, 0xF3, 0xF3, 0xF0, 0xF0,
            0xC3, 0xC3, 0xC3, 0xC3, 0xC3, 0xC3, 0xFF, 0xFF,
            0xCF, 0xCF, 0xCF, 0xCF, 0xCF, 0xCF, 0xC3, 0xC3,
            0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0xFC, 0xFC,
            0x3C, 0x42, 0xB9, 0xA5, 0xB9, 0xA5, 0x42, 0x3C
        }};

        auto tiledataiter = inittiledata.begin();
        for (int addr = 0x0010; addr < 0x01A0; addr += 2) 
	{
            vram[addr] = *tiledataiter++;
	}
    }

    void MMU::shutdown()
    {
	rom.clear();
	vram.clear();
	sram.clear();
	wram.clear();
	oam.clear();
	hram.clear();
	cartmem.clear();
	rambanks.clear();
	gbcbgpalette.clear();
	gbcobjpalette.clear();

	gameboy = Console::Default;
	gbmode = Mode::Default;
	
	cout << "MMU::Shutting down..." << endl;
    }

    void MMU::updatecomponents()
    {
	updatecamera();
	updatetimer();
	updaterumble();
    }

    void MMU::updaterumble()
    {
	if (!isrumblepres)
	{
	    return;
	}

	rumble_on_cycles += (rumble_strength & 3);
	rumble_off_cycles += ((rumble_strength & 3) ^ 3);
    }

    void MMU::handlerumble()
    {
	if (setrumble)
	{
	    if (isrumblepres)
	    {
		if ((rumble_on_cycles + rumble_off_cycles) > 0)
		{
		    double rumble_val = (rumble_on_cycles / double(rumble_on_cycles + rumble_off_cycles));
		    setrumble(rumble_val);
		    rumble_on_cycles = rumble_off_cycles = 0;
		}
	    }
	}
    }

    void MMU::handlevblank()
    {
	handlerumble();
	apply_gs_cheats();
    }

    void MMU::dosavestate(mbGBSavestate &file)
    {
	file.section("MMU ");

	file.vararray(rambanks.data(), rambanks.size());
	file.vararray(vram.data(), 0x4000);
	file.vararray(wram.data(), 0x8000);
	file.vararray(oam.data(), 0xA0);
	file.vararray(hram.data(), 0x7F);
	file.vararray(gbcbgpalette.data(), 0x40);
	file.vararray(gbcobjpalette.data(), 0x40);
	file.varint(&gbcbgpaletteindex);
	file.varint(&gbcobjpaletteindex);
	file.bool32(&doublespeed);
	file.varint(&currentrombank);
	file.varint(&currentrambank);
	file.varint(&wisdomrombank);
	file.bool32(&huc1_bank_mode);
	file.bool32(&huc1_ir_trigger);
	file.var8(&latchsecs);
	file.var8(&latchmins);
	file.var8(&latchhours);
	file.var16(&latchdays);
	file.bool32(&latchhalt);
	file.bool32(&latchdayscarry);
	file.var8(&realsecs);
	file.var8(&realmins);
	file.var8(&realhours);
	file.var16(&realdays);
	file.bool32(&rtchalt);
	file.bool32(&realdayscarry);
	file.varint(&timercounter);
	file.bool32(&rtclatch);
	file.var8(&mr0_reg);
	file.var8(&mr1_reg);
	file.var8(&mr2_reg);
	file.var8(&mr4_reg);
	file.varint(&tpp1_latched_week);
	file.varint(&tpp1_latched_day);
	file.varint(&tpp1_latched_hours);
	file.varint(&tpp1_latched_minutes);
	file.varint(&tpp1_latched_seconds);
	file.varint(&tpp1_rtc_week);
	file.varint(&tpp1_rtc_day);
	file.varint(&tpp1_rtc_hours);
	file.varint(&tpp1_rtc_minutes);
	file.varint(&tpp1_rtc_seconds);
	file.varint(&rumble_strength);
	file.varenum(&map_control);
	// file.varint(reinterpret_cast<int*>(&map_control));
	file.varint(&rumble_on_cycles);
	file.varint(&rumble_off_cycles);
	file.varint(&mbc6rombanka);
	file.varint(&mbc6rombankb);
	file.varint(&mbc6rambanka);
	file.varint(&mbc6rambankb);
	file.varint(&sensorx);
	file.varint(&sensory);
	file.var16(&mbc7_sensorx);
	file.var16(&mbc7_sensorx);
	file.bool32(&mbc7_latch);
	file.bool32(&mbc7_cs);
	file.bool32(&mbc7_clk);
	file.bool32(&mbc7_di);
	file.bool32(&mbc7_do);
	file.varint(&mbc7_buffer);
	file.varint(&mbc7_length);
	file.var16(&mbc7_write_addr);
	file.var16(&mbc7_read_value);
	file.var16(&mbc7_write_value);
	file.bool32(&mbc7_erase_enabled);
	file.varint(&mbc7_gyro_val);
	file.varenum(&mbc7_state);
	// file.varint(reinterpret_cast<int*>(&mbc7_state));
	file.varint(&higherrombankbits);
	file.bool32(&rommode);
	file.bool32(&ramenabled);
	file.bool32(&externalrampres);
	file.bool32(&mbc6flashenable);
	file.bool32(&mbc6flashwriteenable);
	file.bool32(&cameramode);
	file.varint(&camera_trigger);
	file.var8(&camera_outputedge);
	file.var16(&camera_exposure);
	file.var8(&camera_edge);
	file.var8(&camera_voltage);
	file.bool32(&camera_capture);
	file.varint(&camera_clock);
	file.vararray(camera_matrix.data(), camera_matrix.size());
	file.varint(&camera_bank);
	file.varint(&vrambank);
	file.varint(&wrambank);
    }

    size_t MMU::rtc_data_size()
    {
	if (!isrtcpres)
	{
	    return 0;
	}

	if (gbmbc == MBCType::MBC3)
	{
	    return 48;
	}

	return 0;
    }

    bool MMU::loadbackup(vector<uint8_t> data)
    {
	bool success = true;	

	if (batteryenabled)
	{
	    if (data.empty())
	    {
		success = true;
	    }
	    else
	    {
		if (gbmbc != MBCType::None)
		{
		    rambanks = vector<uint8_t>(data.begin(), (data.end() - rtc_data_size()));
		    loadbackuprtc(data, rambanks.size());
		}
	    }
	}

	return success;
    }

    vector<uint8_t> MMU::savebackup()
    {
	vector<uint8_t> save;
	if (batteryenabled)
	{
	    if (gbmbc != MBCType::None)
	    {
		save = vector<uint8_t>(rambanks.begin(), rambanks.end());
		savebackuprtc(save);
	    }
	}

	return save;
    }

    void MMU::loadbackuprtc(vector<uint8_t> saveram, size_t ramsize)
    {
	if (!isrtcpres)
	{
	    return;
	}

	if (gbmbc == MBCType::MBC3)
	{
	    loadmbc3rtc(saveram, ramsize);
	}
    }

    void MMU::savebackuprtc(vector<uint8_t> &saveram)
    {
	if (!isrtcpres)
	{
	    return;
	}

	if (gbmbc == MBCType::MBC3)
	{
	    savembc3rtc(saveram);
	}
    }

    void MMU::add_cheats(vector<string> cheats)
    {
	for (auto &code : cheats)
	{
	    size_t seperator_loc = code.find(':');

	    if (seperator_loc == string::npos)
	    {
		cout << "Invalid cheat code" << endl;
		continue;
	    }

	    string code_id = code.substr(0, seperator_loc);
	    string code_data = code.substr(seperator_loc + 1);

	    if (code_id == "GG")
	    {
		add_gg_cheat(code_data);
	    }
	    else if (code_id == "GS")
	    {
		add_gs_cheat(code_data);
	    }
	    else
	    {
		cout << "Invalid code ID of " << code_id << endl;
		continue;
	    }
	}
    }

    void MMU::add_gg_cheat(string cheat_code)
    {
	if (cheat_code.length() != 9)
	{
	    cout << "Could not parse Game Genie code" << endl;
	    return;
	}

	gamegenie_codes.push_back(cheat_code);
    }

    void MMU::add_gs_cheat(string cheat_code)
    {
	if (cheat_code.length() != 8)
	{
	    cout << "Could not parse Game Shark code" << endl;
	    return;
	}

	gameshark_codes.push_back(cheat_code);
    }

    void MMU::apply_gg_cheats()
    {
	for (auto &code : gamegenie_codes)
	{
	    string current_code = code;
	    current_code.erase(remove(current_code.begin(), current_code.end(), '-'), current_code.end());
	    uint8_t new_data = from_hex_str(current_code.substr(0, 2));

	    // Format of address string is FCDE
	    string addr_str = "";
	    addr_str.push_back(current_code.at(5));
	    addr_str.push_back(current_code.at(2));
	    addr_str.push_back(current_code.at(3));
	    addr_str.push_back(current_code.at(4));

	    uint16_t mem_addr = from_hex_str(addr_str);
	    mem_addr ^= 0xF000;

	    // Format of old data string is GI
	    string data_str = "";
	    data_str.push_back(current_code.at(6));
	    data_str.push_back(current_code.at(8));
	    uint8_t old_data = from_hex_str(data_str);
	    old_data = ((old_data ^ 0xBA) << 2);

	    if (mem_addr < 0x4000)
	    {
		cartmem.at(mem_addr) = new_data;
	    }
	    else
	    {
		for (int bank = 0; bank < numrombanks; bank++)
		{
		    uint32_t bank_addr = ((mem_addr - 0x4000) + (bank * 0x4000));

		    if (cartmem.at(bank_addr) == old_data)
		    {
			cartmem.at(bank_addr) = new_data;
		    }
		}
	    }
	}
    }

    void MMU::apply_gs_cheats()
    {
	for (auto &code : gameshark_codes)
	{
	    string current_code = code;

	    uint8_t ram_bank_num = from_hex_str(current_code.substr(0, 2));
	    uint8_t new_data = from_hex_str(current_code.substr(2, 2));
	    string addr_str = "";
	    addr_str.append(current_code.substr(6, 2));
	    addr_str.append(current_code.substr(4, 2));
	    uint16_t ram_addr = from_hex_str(addr_str);

	    if (inRange(ram_addr, 0xA000, 0xC000))
	    {
		ram_bank_num %= numrambanks;

		uint16_t mem_addr = ((ram_addr - 0xA000) + (ram_bank_num * 0x2000));
		rambanks.at(mem_addr) = new_data;
	    }
	    else if (inRange(ram_addr, 0xC000, 0xD000))
	    {
		wram.at((ram_addr - 0xC000)) = new_data;
	    }
	    else if (inRange(ram_addr, 0xD000, 0xE000))
	    {
		ram_bank_num %= 8;
		int wram_bank_num = 0;

		if (isgbcmode())
		{
		    wram_bank_num = ((ram_bank_num == 0) ? 0 : (ram_bank_num - 1));
		}

		uint16_t wram_addr = ((ram_addr - 0xC000) + (wram_bank_num * 0x1000));
		wram.at(wram_addr) = new_data;
	    }
	}
    }

    uint8_t MMU::readDirectly(uint16_t addr)
    {
	if (addr < 0x8000)
	{
	    uint8_t temp = 0;

	    bool isromread = false;

	    if (biosload == true)
	    {
		if ((biossize == 0x900) && (addr > 0x100) && (addr < 0x200))
		{
		    isromread = true;
		}
		else if (addr == 0x100)
		{
		    exitbios();
		    isromread = true;
		}
		else if (addr < biossize)
		{
		    isromread = false;
		}
		else
		{
		    isromread = true;
		}
	    }
	    else
	    {
		isromread = true;
	    }

	    if (isromread == true)
	    {
		switch (gbmbc)
		{
		    case MBCType::None: temp = rom[addr]; break;
		    case MBCType::MBC1: temp = mbc1read(addr); break;
		    case MBCType::MBC2: temp = mbc2read(addr); break;
		    case MBCType::MBC3: temp = mbc3read(addr); break;
		    case MBCType::MBC5: temp = mbc5read(addr); break;
		    case MBCType::MBC6: temp = mbc6read(addr); break;
		    case MBCType::MBC7: temp = mbc7read(addr); break;
		    case MBCType::Camera: temp = gbcameraread(addr); break;
		    case MBCType::HuC3: temp = huc3read(addr); break;
		    case MBCType::HuC1: temp = huc1read(addr); break;
		    case MBCType::WisdomTree: temp = wisdomtreeread(addr); break;
		    case MBCType::TPP1: temp = tpp1read(addr); break;
		    default:
		    {
			cout << "Unrecognized MBC read" << endl;
			exit(0);
			temp = 0;
		    }
		    break;
		}
	    }
	    else
	    {
		return bios[addr];
	    }

	    return temp;
	}
	else if (addr < 0xA000)
	{
	    return vram[(addr - 0x8000) + (vrambank * 0x2000)];
	}
	else if (addr < 0xC000)
	{
	    uint8_t temp = 0;

	    switch (gbmbc)
	    {
		case MBCType::None: temp = 0xFF; break;
		case MBCType::MBC1: temp = mbc1read(addr); break;
		case MBCType::MBC2: temp = mbc2read(addr); break;
		case MBCType::MBC3: temp = mbc3read(addr); break;
		case MBCType::MBC5: temp = mbc5read(addr); break;
		case MBCType::MBC6: temp = mbc6read(addr); break;
		case MBCType::MBC7: temp = mbc7read(addr); break;
		case MBCType::Camera: temp = gbcameraread(addr); break;
		case MBCType::HuC3: temp = huc3read(addr); break;
		case MBCType::HuC1: temp = huc1read(addr); break;
		case MBCType::WisdomTree: temp = wisdomtreeread(addr); break;
		case MBCType::TPP1: temp = tpp1read(addr); break;
		default:
		{
		    cout << "Unrecognized MBC read" << endl;
		    exit(0);
		    temp = 0;
		}
		break;
	    }

	    return temp;
	}
	else if (addr < 0xD000)
	{
	    return wram[addr - 0xC000];
	}
	else if (addr < 0xE000)
	{
	    uint16_t offset = (0x1000 * ((wrambank == 0) ? 0 : (wrambank - 1)));
	    return wram[addr - 0xC000 + offset];
	}
	else if (addr < 0xF000)
	{
	    return wram[addr - 0xE000];
	}
	else if (addr < 0xFE00)
	{
	    uint16_t offset = (0x1000 * ((wrambank == 0) ? 0 : (wrambank - 1)));
	    return wram[addr - 0xE000 + offset];
	}
	else if (addr < 0xFEA0)
	{
	    return oam[addr - 0xFE00];
	}
	else if (addr < 0xFF00)
	{
	    return 0x00;
	}
	else if (addr < 0xFF80)
	{
	    return readIO(addr);
	}
	else if (addr < 0xFFFF)
	{
	    return hram[addr - 0xFF80];
	}
	else
	{
	    return interruptenabled;
	}
    }

    uint8_t MMU::readByte(uint16_t addr)
    {
    	uint8_t temp = 0;
    
	if (addr < 0x4000)
	{
	    temp = readDirectly(addr);
	}
	else if (addr < 0x8000)
	{
	    if (dmabusblock != Bus::External)
	    {
		temp = readDirectly(addr);
	    }
	    else
	    {
		temp = oamtransferbyte;
	    }
	}
	else if (addr < 0xA000)
	{
	    if (dmabusblock != Bus::Vram)
	    {
		temp = readDirectly(addr);
	    }
	    else
	    {
		temp = oamtransferbyte;
	    }
	}
	else if (addr < 0xFE00)
	{
	    if (dmabusblock != Bus::External)
	    {
		temp = readDirectly(addr);
	    }
	}
	else if (addr < 0xFEA0)
	{
	    if (dmabusblock == Bus::None)
	    {
		temp = readDirectly(addr);
	    }
	    else
	    {
		temp = 0xFF;
	    }
	}
	else if (addr < 0xFF00)
	{
	    temp = 0x00;
	}
	else
	{
	    temp = readDirectly(addr);
	}
	
	return temp;
    }

    void MMU::writeDirectly(uint16_t addr, uint8_t value)
    {
	if (addr < 0x8000)
	{
	    switch (gbmbc)
	    {
		case MBCType::None: return; break;
		case MBCType::MBC1: mbc1write(addr, value); break;
		case MBCType::MBC2: mbc2write(addr, value); break;
		case MBCType::MBC3: mbc3write(addr, value); break;
		case MBCType::MBC5: mbc5write(addr, value); break;
		case MBCType::MBC6: mbc6write(addr, value); break;
		case MBCType::MBC7: mbc7write(addr, value); break;
		case MBCType::Camera: gbcamerawrite(addr, value); break;
		case MBCType::HuC3: huc3write(addr, value); break;
		case MBCType::HuC1: huc1write(addr, value); break;
		case MBCType::WisdomTree: wisdomtreewrite(addr, value); break;
		case MBCType::TPP1: tpp1write(addr, value); break;
		default:
		{
		    cout << "Unrecognized MBC write" << endl;
		    exit(0);
		}
		break;
	    }
	}
	else if (addr < 0xA000)
	{
	    vram[(addr - 0x8000) + (vrambank * 0x2000)] = value;
	}
	else if (addr < 0xC000)
	{
	    switch (gbmbc)
	    {
		case MBCType::None: return; break;
		case MBCType::MBC1: mbc1write(addr, value); break;
		case MBCType::MBC2: mbc2write(addr, value); break;
		case MBCType::MBC3: mbc3write(addr, value); break;
		case MBCType::MBC5: mbc5write(addr, value); break;
		case MBCType::MBC6: mbc6write(addr, value); break;
		case MBCType::MBC7: mbc7write(addr, value); break;
		case MBCType::Camera: gbcamerawrite(addr, value); break;
		case MBCType::HuC3: huc3write(addr, value); break;
		case MBCType::HuC1: huc1write(addr, value); break;
		case MBCType::WisdomTree: wisdomtreewrite(addr, value); break;
		case MBCType::TPP1: tpp1write(addr, value); break;
		default:
		{
		    cout << "Unrecognized MBC write" << endl;
		    exit(0);
		}
		break;
	    }
	}
	else if (addr < 0xD000)
	{
	    wram[addr - 0xC000] = value;
	}
	else if (addr < 0xE000)
	{
	    uint16_t offset = (0x1000 * ((wrambank == 0) ? 0 : (wrambank - 1)));
	    wram[addr - 0xC000 + offset] = value;
	}
	else if (addr < 0xF000)
	{
	    wram[addr - 0xE000] = value;
	}
	else if (addr < 0xFE00)
	{
	    uint16_t offset = (0x1000 * ((wrambank == 0) ? 0 : (wrambank - 1)));
	    wram[addr - 0xE000 + offset] = value;
	}
	else if (addr < 0xFEA0)
	{
	    oam[addr - 0xFE00] = value;
	}
	else if (addr < 0xFF00)
	{
	    return;
	}
	else if (addr < 0xFF80)
	{
	    writeIO(addr, value);
	}
	else if (addr < 0xFFFF)
	{
	    hram[addr - 0xFF80] = value;
	}
	else
	{
	    interruptenabled = value;
	}
    }

    void MMU::writeByte(uint16_t addr, uint8_t value)
    {
	if (addr < 0x8000)
	{
	    if (dmabusblock != Bus::External)
	    {
		writeDirectly(addr, value);
	    }
	}
	else if (addr < 0xA000)
	{
	    if (dmabusblock != Bus::Vram)
	    {
		writeDirectly(addr, value);
	    }
	}
	else if (addr < 0xFE00)
	{
	    if (dmabusblock != Bus::External)
	    {
		writeDirectly(addr, value);
	    }
	}
	else if (addr < 0xFEA0)
	{
	    if (dmabusblock == Bus::None)
	    {
		writeDirectly(addr, value);
	    }
	}
	else if (addr < 0xFF00)
	{
	    return;
	}
	else
	{
	    writeDirectly(addr, value);
	}
    }

    uint16_t MMU::readWord(uint16_t addr)
    {
	return ((readByte(addr + 1) << 8) | (readByte(addr)));
    }

    void MMU::writeWord(uint16_t addr, uint16_t val)
    {
	writeByte(addr , (val & 0xFF));
	writeByte((addr + 1), (val >> 8));
    }

    uint8_t MMU::readIO(uint16_t addr)
    {
	uint8_t temp = 0;

	if (inRangeEx(addr, 0xFF00, 0xFF07))
	{
	    temp = memoryreadhandlers.at((addr - 0xFF00))(addr);
	}
	else if (inRangeEx(addr, 0xFF10, 0xFF26))
	{
	    temp = memoryreadhandlers.at((addr - 0xFF00))(addr);
	}
	else if (inRange(addr, 0xFF30, 0xFF46))
	{
	    temp = memoryreadhandlers.at((addr - 0xFF00))(addr);
	}
	else if (inRangeEx(addr, 0xFF47, 0xFF4B))
	{
	    temp = memoryreadhandlers.at((addr - 0xFF00))(addr);
	}
	else if (addr == 0xFF56)
	{
	    temp = memoryreadhandlers.at((addr - 0xFF00))(addr);
	}
	else
	{
	    switch ((addr & 0xFF))
	    {
	    	case 0x0F: temp = (interruptflags | 0xE0); break;
	    	case 0x46: temp = oamdmastart; break;
	    	case 0x4D: temp = (key1 | ((isgbcmode()) ? 0x7E : 0xFF)); break;
	    	case 0x4F:
		{
		    if (isgbcconsole())
		    {
			temp = ((isgbcmode()) ? (vrambank | 0xFE) : 0xFE);
		    }
		    else
		    {
			temp = 0xFF;
		    }
		}
		break;
	    	case 0x55: temp = ((!ishdmaactive) | 0x7F); break;
	    	case 0x68: temp = (isgbcconsole()) ? gbcbgpaletteindex : 0xFF; break;
	    	case 0x69: temp = (isgbcconsole()) ? gbcbgpalette[gbcbgpaletteindex] : 0xFF; break;
	    	case 0x6A: temp = (isgbcconsole()) ? gbcobjpaletteindex : 0xFF; break;
	    	case 0x6B: temp = (isgbcconsole()) ? gbcobjpalette[gbcobjpaletteindex] : 0xFF; break;
	    	case 0x70: temp = (wrambank | 0xF8); break;
	    	default: temp = 0xFF; break;
	    }
	}
	
	return temp;
    }

    void MMU::writeIO(uint16_t addr, uint8_t value)
    {
	if (inRangeEx(addr, 0xFF00, 0xFF07))
	{
	    memorywritehandlers.at((addr - 0xFF00))(addr, value);
	}
	else if (inRangeEx(addr, 0xFF10, 0xFF26))
	{
	    memorywritehandlers.at((addr - 0xFF00))(addr, value);
	}
	else if (inRange(addr, 0xFF30, 0xFF46))
	{
	    memorywritehandlers.at((addr - 0xFF00))(addr, value);
	}
	else if (inRangeEx(addr, 0xFF47, 0xFF4B))
	{
	    memorywritehandlers.at((addr - 0xFF00))(addr, value);
	}
	else if (addr == 0xFF56)
	{
	    memorywritehandlers.at((addr - 0xFF00))(addr, value);
	}
	else
	{
	    switch ((addr & 0xFF))
	    {
		case 0x0F: writeif(value); break;
		case 0x46: writedma(value); break;
		case 0x4D: 
		{
		    key1 = value;
		}
		break;
		case 0x4F: 
		{
		    vrambank = (isgbcmode()) ? getbitval(value, 0) : 0;
		}
		break;
		case 0x51:
		{
		    hdmasource = ((value << 8) | (hdmasource & 0xFF));
	 	}
		break;
		case 0x52:
		{
		    hdmasource = ((hdmasource & 0xFF00) | (value & 0xF0));
		}
		break;
		case 0x53:
		{
		    hdmadest = ((((value & 0x1F) | 0x80) << 8) | (hdmadest & 0xFF));
		}
	 	break;
		case 0x54:
		{
		    hdmadest = ((hdmadest & 0xFF00) | (value & 0xF0));
		}
		break;
		case 0x55:
		{
		    if (hdmastate == DmaState::Inactive)
		    {
			initgbcdma(value);
		    }
		    else
		    {
		    	if (testbit(value, 7))
		   	{
			    initgbcdma(value);
		    	}
		    	else
		   	{
			    ishdmaactive = false;
			    hdmabytestocopy = 0;
			    hdmabytes = 0;
			    hdmastate = DmaState::Inactive;
		        }
		    }
		}
		break;
		case 0x68:
		{
		    if (!isgbcmode())
		    {
		    	return;
		    }	

		    gbcbgpaletteindex = (value & 0x3F);
		    gbcbgpalinc = testbit(value, 7);
	        }
	        break;
	        case 0x69:
	        {
		    if (!isgbcmode())
		    {
		    	return;
		    }		

		    gbcbgpalette[gbcbgpaletteindex] = value;

		    if (gbcbgpalinc)
		    {
		    	gbcbgpaletteindex = ((gbcbgpaletteindex + 1) & 0x3F);
		    }
	    	}
	    	break;
	    	case 0x6A:
	    	{
		    if (!isgbcmode())
		    {
		    	return;
		    }	

		    gbcobjpaletteindex = (value & 0x3F);
		    gbcobjpalinc = testbit(value, 7);
	    	}
	    	break;
	    	case 0x6B:
	    	{
		    if (!isgbcmode())
		    {
		    	return;
		    }		

		    gbcobjpalette[gbcobjpaletteindex] = value;

		    if (gbcobjpalinc)
		    {
		    	gbcobjpaletteindex = ((gbcobjpaletteindex + 1) & 0x3F);
		    }
	    	}
	    	break;
	    	case 0x70: 
	    	{
		    if (!isgbcmode())
		    {
		    	return;
		    }		

		    wrambank = (value & 0x07);
	    	}
	    	break;
	    	default: break;
	    }
	}
    }

    bool MMU::loadROM(vector<uint8_t> data)
    {
	if (!data.empty())
	{
	    cartmem = data;

	    bool cgbflag = ((cartmem[0x0143] == 0xC0) || (cartmem[0x0143] == 0x80));

	    if (gameboy == Console::Default)
	    {
		if (cgbflag)
		{
		    gameboy = Console::CGB;
		}
		else
		{
		    gameboy = Console::DMG;
		}
	    }
	    
	    if (isgbcconsole() && cgbflag)
	    {
		gbmode = Mode::CGB;
	    }
	    else
	    {
		gbmode = Mode::DMG;
	    }

	    cout << "Title: " << determinegametitle(cartmem) << endl;
	    determinembctype(cartmem);
	    cout << "MBC type: " << mbctype << endl;
	    numrombanks = getrombanks(cartmem);
	    cout << "ROM size: " << romsize << endl;
	    numrambanks = getrambanks(cartmem);
	    cout << "RAM size: " << ramsize << endl;
	    
	    if (mbcramsize != 0)
	    {
		rambanks.resize((mbcramsize << 10), 0);
	    }

	    if (gbmbc != MBCType::None && static_cast<int>(data.size()) != (numrombanks * 0x4000))
	    {
		cout << "MMU::Warning - Size of ROM does not match size in cartridge header." << endl;
	    }

	    if (gbmbc == MBCType::MBC2)
	    {
		rambanks.clear();
		rambanks.resize(512, 0);
	    }

	    if (gbmbc == MBCType::MBC7)
	    {
		rambanks.clear();
		int numbanks = (numrombanks == 128) ? 512 : 256;
		rambanks.resize(numbanks, 0);
	    }

	    apply_gg_cheats();

	    rom = vector<uint8_t>(cartmem.begin(), (cartmem.begin() + 0x8000));
	    cout << "MMU::ROM succesfully loaded." << endl;
	    return true;
	}
	else
	{
	    cout << "MMU::Error - ROM could not be opened." << endl;
	    return false;
	}
    }

    bool MMU::loadBIOS(vector<uint8_t> data)
    {
	if (!data.empty())
	{
	    size_t size = data.size();

	    if (size == 0x100)
	    {
		if (gameboy == Console::Default)
		{
		    gameboy = Console::DMG;
		}

		bios.resize(0x100, 0);
	    }
	    else if (size == 0x900)
	    {
		if (gameboy == Console::Default)
		{
		    gameboy = Console::CGB;
		}

		bios.resize(0x900, 0);
	    }
	    else
	    {
		cout << "MMU::Error - BIOS size does not match sizes of the official BIOS." << endl;
		return false;
	    }

	    biossize = size;
	    bios = data;

	    cout << "MMU::BIOS succesfully loaded." << endl;
	    return true;
	}
	else
	{
	    cout << "MMU::Error - BIOS could not be opened." << endl;
	    return false;
	}
    }
};
