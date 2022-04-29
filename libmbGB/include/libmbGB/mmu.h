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

#ifndef LIBMBGB_MMU
#define LIBMBGB_MMU

#include <cstdint>
#include <vector>
#include <cstring>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <functional>
#include <algorithm>
#include <array>
#include <ctime>
#include "enums.h"
#include "utils.h"
#include "libmbgb_api.h"
using namespace std;

namespace gb
{
    using boolfunc = function<bool()>;
    using voidfunc = function<void()>;

    using rumblefunc = function<void(double)>;
    using caminitfunc = boolfunc;
    using camstopfunc = voidfunc;
    using camframefunc = function<bool(array<int, (128 * 120)>&)>;

    using memoryreadfunc = function<uint8_t(uint16_t)>;
    using memorywritefunc = function<void(uint16_t, uint8_t)>;

    class LIBMBGB_API MMU
    {
	public:
	    MMU();
	    ~MMU();

	    Console gameboy = Console::Default;
	    Mode gbmode = Mode::Default;
	    MBCType gbmbc;

	    void init();
	    void initvram();
	    void initnot();
	    void shutdown();

	    void handlevblank();

	    void updatecomponents();

	    void set_accel_values(float x, float y);
		
	    array<memoryreadfunc, 0x80> memoryreadhandlers;
	    array<memorywritefunc, 0x80> memorywritehandlers;
		
	    void addmemoryreadhandler(uint16_t addr, memoryreadfunc cb)
	    {
		memoryreadhandlers.at((addr - 0xFF00)) = cb;
	    }
		
	    void addmemorywritehandler(uint16_t addr, memorywritefunc cb)
	    {
		memorywritehandlers.at((addr - 0xFF00)) = cb;
	    }

	    uint8_t readempty(uint16_t addr)
	    {
		return 0xFF;
	    }

	    void writeempty(uint16_t addr, uint8_t val)
	    {
		return;
	    }

	    void dosavestate(mbGBSavestate &file);

	    bool loadmmu(int offs, string filename);
	    bool savemmu(string filename);

	    bool loadbackup(vector<uint8_t> data);
	    vector<uint8_t> savebackup();

	    void add_cheats(vector<string> cheats);

	    vector<uint8_t> rom;
	    vector<uint8_t> vram;
	    vector<uint8_t> sram;
	    vector<uint8_t> wram;
	    vector<uint8_t> oam;
	    vector<uint8_t> hram;
	    vector<uint8_t> cartmem;
	    vector<uint8_t> rambanks;
	    vector<uint8_t> bios;
	    array<uint8_t, 4> specialrombanks = {0x00, 0x20, 0x40, 0x60};

	    bool externalrampres = false;
	    bool isrumblepres = false;
	    bool isrtcpres = false;
	    bool biosload = false;
	    int currentrombank = 1;
	    int currentrambank = 0;
	    int wisdomrombank = 0;
	    int higherrombankbits = 0;
	    bool ramenabled = false;
	    bool batteryenabled = false;
	    bool rtcenabled = false;
	    bool ramsecenabled = false;
	    bool rommode = false;
	    int numrombanks;
	    int numrambanks;
	    int mbcramsize;
	    int biossize = 0;
	    bool ismanual = false;
	    bool hybrid = false;
	    bool doublespeed = false;

	    bool rtclatch = false;
	    time_t current_time = 0;

	    int timercounter = 0;

	    uint8_t realsecs = 0;
	    uint8_t realmins = 0;
	    uint8_t realhours = 0;
	    uint16_t realdays = 0;
	    bool rtchalt = false;
	    bool realdayscarry = false;

	    void inittimer();
	    void updatetimer();
	    void latchtimer();

	    void initmbc3rtc();
	    void updatembc3rtc();

	    uint8_t latchsecs = 0;
	    uint8_t latchmins = 0;
	    uint8_t latchhours = 0;
	    uint16_t latchdays = 0;
	    bool latchhalt = false;
	    bool latchdayscarry = false;

	    void loadbackuprtc(vector<uint8_t> saveram, size_t ramsize);
	    void savebackuprtc(vector<uint8_t> &saveram);
	    size_t rtc_data_size();

	    void loadmbc3rtc(vector<uint8_t> saveram, size_t ramsize);
	    void savembc3rtc(vector<uint8_t> &saveram);

	    int mbc6rombanka = 0;
	    int mbc6rombankb = 0;
	    int mbc6rambanka = 0;
	    int mbc6rambankb = 0;
	    bool mbc6bankaflash = false;
	    bool mbc6bankbflash = false;

	    bool mbc6flashenable = false;
	    bool mbc6flashwriteenable = false;

	    // MBC7 variables
	    uint8_t mbc7_reg_read(uint16_t addr);
	    void mbc7_reg_write(uint16_t addr, uint8_t value);

	    int sensorx = 0;
	    int sensory = 0;

	    uint16_t mbc7_sensorx = 0;
	    uint16_t mbc7_sensory = 0;

	    bool mbc7_latch = false;

	    uint8_t read_mbc7_eeprom();
	    void write_mbc7_eeprom(uint8_t value);
	    void process_mbc7_command();

	    void latch_mbc7_gyro();

	    bool mbc7_cs = false;
	    bool mbc7_clk = false;
	    bool mbc7_di = false;
	    bool mbc7_do = false;

	    int mbc7_buffer = 0;
	    int mbc7_length = 0;

	    uint16_t mbc7_write_addr = 0;
	    uint16_t mbc7_read_value = 0;
	    uint16_t mbc7_write_value = 0;
	    bool mbc7_erase_enabled = false;

	    int mbc7_gyro_val = 0;

	    enum MBC7State : int
	    {
		StartBit = 0,
		Command = 1,
		Read = 2,
		Write = 3,
		WriteAll = 4,
	    };

	    MBC7State mbc7_state = MBC7State::StartBit;

	    // Rumble related variables
	    void updaterumble();
	    void handlerumble();

	    int rumble_on_cycles = 0;
	    int rumble_off_cycles = 0;
	    int rumble_strength = 0;

	    rumblefunc setrumble;

	    inline void setrumblecallback(rumblefunc cb)
	    {
		setrumble = cb;
	    }

	    // GB Camera variables
	    bool cameramode = false;
	    int camera_trigger = 0;
	    uint8_t camera_outputedge = 0;
	    uint16_t camera_exposure = 0;
	    uint8_t camera_edge = 0;
	    uint8_t camera_voltage = 0;
	    bool camera_capture = false;
	    int camera_clock = 0;
	    array<uint8_t, 47> camera_matrix;
	    int camera_bank = 0;

	    uint8_t readgbcamreg(uint16_t addr);
	    void writegbcamreg(uint16_t addr, uint8_t val);
	    void updatecamera();

	    caminitfunc caminit;
	    camstopfunc camstop;
	    camframefunc camframe;	

	    array<int, (128 * 120)> cam_web_output;
	    array<int, (128 * 120)> cam_ret_output;

	    inline void setcamcallbacks(caminitfunc icb, camstopfunc scb, camframefunc fcb)
	    {
		caminit = icb;
		camstop = scb;
		camframe = fcb;
	    }

	    void take_camera_pic();
	    int camera_matrix_process(int val, int x, int y);

	    // HuC1 variables
	    bool huc1_bank_mode = false;
	    bool huc1_ir_trigger = false;

	    // HuC3 variables
	    int huc3_mode = 0;
	    uint8_t huc3_value = 0;
	    int huc3_index = 0;

	    void huc3_commit();

	    // TPP1 variables
	    uint8_t mr0_reg = 0;
	    uint8_t mr1_reg = 0;
	    uint8_t mr2_reg = 0;
	    uint8_t mr4_reg = 0;

	    enum TPP1Mapped : int
	    {
		ControlReg = 0,
		SRAMRead = 1,
		SRAMReadWrite = 2,
		RTCLatched = 3,
	    };

	    TPP1Mapped map_control;

	    uint8_t read_tpp1_control_reg(uint16_t addr);
	    void write_tpp1_control_reg(uint16_t addr, uint8_t value);
	    void write_tpp1_mr3(uint8_t value);
	    uint8_t read_tpp1_rtc(uint16_t addr);
	    void write_tpp1_rtc(uint16_t addr, uint8_t value);

	    bool is_multi_rumble = false;

	    int tpp1_latched_week = 0;
	    int tpp1_latched_day = 0;
	    int tpp1_latched_hours = 0;
	    int tpp1_latched_minutes = 0;
	    int tpp1_latched_seconds = 0;

	    int tpp1_rtc_week = 0;
	    int tpp1_rtc_day = 0;
	    int tpp1_rtc_hours = 0;
	    int tpp1_rtc_minutes = 0;
	    int tpp1_rtc_seconds = 0;

	    void updatetpp1rtc();

	    // Other variables

	    bool dump = false;

	    int vrambank = 0;
	    int wrambank = 1;

	    int gbcbgpaletteindex = 0;
	    vector<uint8_t> gbcbgpalette;
	    int gbcobjpaletteindex = 0;
	    vector<uint8_t> gbcobjpalette;
	    bool gbcbgpalinc = false;
	    bool gbcobjpalinc = false;

	    bool isgbcamera()
	    {
		return (gbmbc == MBCType::Camera);
	    }

	    bool istiltsensor()
	    {
		return (gbmbc == MBCType::MBC7);
	    }

	    bool isdmgconsole()
	    {
		return (gameboy == Console::DMG);
	    }

	    bool iscgbconsole()
	    {
		return (gameboy == Console::CGB);
	    }

	    bool isagbconsole()
	    {
		return (gameboy == Console::AGB);
	    }

	    bool isdmgmode()
	    {
		return (gbmode == Mode::DMG);
	    }

	    bool isgbcmode()
	    {
		return (gbmode == Mode::CGB);
	    }

	    bool isgbcconsole()
	    {
		return (iscgbconsole() || isagbconsole());
	    }

	    bool isgbcdmgmode()
	    {
		return (isgbcconsole() && isdmgmode());
	    }

	    bool isgbcgame()
	    {
		return (isgbcconsole() && isgbcmode());
	    }

	    bool ishybridconsole()
	    {
		return hybrid;
	    }

	    bool isdoublespeed()
	    {
		return doublespeed;
	    }

	    bool detectwisdomtree = false;

	    inline bool iswisdomtree(vector<uint8_t>& rom)
	    {
		for (int i = 0x134; i < 0x14C; i += 4)
		{
		    if (*(uint32_t*)&rom[i] != 0)
		    {
			return false;
		    }
		}

		for (int i = 0xF0; i < 0x100; i += 4)
		{
		    if (*(uint32_t*)&rom[i] != 0)
		    {
			return false;
		    }
		}

		if (rom[0x14D] != 0xE7)
		{
		    return false;
		}


		for (int i = 0x300; i < (int)(rom.size() - 11); i++)
		{
		    if ((strncmp((const char*)&rom[i], "WISDOM", 6) == 0) && (strncmp((const char*)&rom[i + 7], "TREE", 4) == 0))
		    {
			return true;
		    }
		}

		return false;
	    }

	    inline void isromonly(vector<uint8_t>& rom)
	    {
		externalrampres = false;
		batteryenabled = false;

		if (iswisdomtree(rom))
		{
		    gbmbc = MBCType::WisdomTree;
		    numrombanks = 64;
		    romsize = "1 MB";
		    mbctype = "WISDOM TREE";
		}
		else if (rom.size() > 0x8000)
		{
		    cout << "ROM header reports no MBC, but file size is over 32 KB. Assuming cartridge uses MBC3..." << endl;
		    gbmbc = MBCType::MBC3;
		    externalrampres = false;
		    mbctype = "MBC3";
		    batteryenabled = false;
		}
		else
		{
		    gbmbc = MBCType::None;
		    mbctype = "ROM ONLY";
		}
	    }

	    inline void determinembctype(vector<uint8_t>& rom)
	    {
		if ((rom[0x0147] == 0xBC) && (rom[0x0149] == 0xC1) && (rom[0x14A] == 0x65))
		{
		    gbmbc = MBCType::TPP1;
		    mbctype = "TPP1";
		    externalrampres = (rom[0x0152] != 0);
		    isrumblepres = testbit(rom[0x0153], 0);

		    if (isrumblepres)
		    {
			is_multi_rumble = testbit(rom[0x0153], 1);
		    }

		    isrtcpres = testbit(rom[0x0153], 2);
		    batteryenabled = testbit(rom[0x0153], 3);
		    return;
		}

		switch (rom[0x0147])
		{
		    case 0x00: isromonly(rom); break;
		    case 0x01: gbmbc = MBCType::MBC1; externalrampres = false; mbctype = "MBC1"; batteryenabled = false; break;
		    case 0x02: gbmbc = MBCType::MBC1; externalrampres = true; mbctype = "MBC1 + RAM"; batteryenabled = false; break;
		    case 0x03: gbmbc = MBCType::MBC1; externalrampres = true; mbctype = "MBC1 + RAM + BATTERY"; batteryenabled = true; break;
		    case 0x05: gbmbc = MBCType::MBC2; externalrampres = false; mbctype = "MBC2"; batteryenabled = false; break;
		    case 0x06: gbmbc = MBCType::MBC2; externalrampres = false; mbctype = "MBC2 + BATTERY"; batteryenabled = true; break;
		    case 0x08: gbmbc = MBCType::None; externalrampres = true; mbctype = "ROM + RAM"; batteryenabled = false; break;
		    case 0x09: gbmbc = MBCType::None; externalrampres = true; mbctype = "ROM + RAM + BATTERY"; batteryenabled = true; break;
		    case 0x0F: gbmbc = MBCType::MBC3; externalrampres = false; mbctype = "MBC3 + TIMER + BATTERY"; batteryenabled = true; isrtcpres = true; break;
		    case 0x10: gbmbc = MBCType::MBC3; externalrampres = true; mbctype = "MBC3 + TIMER + RAM + BATTERY"; batteryenabled = true; isrtcpres = true; break;
		    case 0x11: gbmbc = MBCType::MBC3; externalrampres = false; mbctype = "MBC3"; batteryenabled = false; break;
		    case 0x12: gbmbc = MBCType::MBC3; externalrampres = true; mbctype = "MBC3 + RAM"; batteryenabled = false; break;
		    case 0x13: gbmbc = MBCType::MBC3; externalrampres = true; mbctype = "MBC3 + RAM + BATTERY"; batteryenabled = true; break;
		    case 0x19: gbmbc = MBCType::MBC5; externalrampres = false; mbctype = "MBC5"; batteryenabled = false; break;
		    case 0x1A: gbmbc = MBCType::MBC5; externalrampres = true; mbctype = "MBC5 + RAM"; batteryenabled = false; break;
		    case 0x1B: gbmbc = MBCType::MBC5; externalrampres = true; mbctype = "MBC5 + RAM + BATTERY"; batteryenabled = true; isrtcpres = false; break;
		    case 0x1C: gbmbc = MBCType::MBC5; externalrampres = false; isrumblepres = true; mbctype = "MBC5 + RUMBLE"; batteryenabled = false; break;
		    case 0x1D: gbmbc = MBCType::MBC5; externalrampres = true; isrumblepres = true; mbctype = "MBC5 + RUMBLE + RAM"; batteryenabled = false; break;
		    case 0x1E: gbmbc = MBCType::MBC5; externalrampres = true; isrumblepres = true; mbctype = "MBC5 + RUMBLE + RAM + BATTERY"; batteryenabled = true; break;
		    case 0x20: gbmbc = MBCType::MBC6; externalrampres = true; isrumblepres = false; mbctype = "MBC6"; batteryenabled = true; break;
		    case 0x22: gbmbc = MBCType::MBC7; externalrampres = true; isrumblepres = true; mbctype = "MBC7 + SENSOR + RUMBLE + RAM + BATTERY"; batteryenabled = true; break;
		    case 0xFC: gbmbc = MBCType::Camera; externalrampres = true; mbctype = "POCKET CAMERA"; batteryenabled = true; break;
		    case 0xFE: gbmbc = MBCType::HuC3; mbctype = "HuC3"; break;
		    case 0xFF: gbmbc = MBCType::HuC1; externalrampres = true; mbctype = "HuC1 + RAM + BATTERY"; batteryenabled = true; break;
		    default: cout << "MMU::Error - Unrecognized MBC type of " << hex << (int)(rom[0x0147]) << endl; exit(1); break;
		}
	    }

	    inline int getrombanks(vector<uint8_t>& rom)
	    {
		int banks = 0;

		switch (rom[0x0148])
		{
		    case 0: banks = 2; romsize = "32 KB"; break;
		    case 1: banks = 4; romsize = "64 KB"; break;
		    case 2: banks = 8; romsize = "128 KB"; break;
		    case 3: banks = 16; romsize = "256 KB"; break;
		    case 4: banks = 32; romsize = "512 KB"; break;
		    case 5: banks = 64; romsize = "1 MB"; break;
		    case 6: banks = 128; romsize = "2 MB"; break;
		    case 7: banks = 256; romsize = "4 MB"; break;
		    case 8: banks = 512; romsize = "8 MB"; break;
		    case 9: banks = 1024; romsize = "16 MB"; break;
		    case 10: banks = 2048; romsize = "32 MB"; break;
		    case 11: banks = 4096; romsize = "64 MB"; break;
		    case 12: banks = 8192; romsize = "128 MB"; break;
		    case 13: banks = 16384; romsize = "256 MB"; break;
		    case 14: banks = 32768; romsize = "512 MB"; break;
		    case 15: banks = 65536; romsize = "1 GB"; break;
		    case 82: banks = 72; romsize = "1.1 MB"; break;
		    case 83: banks = 80; romsize = "1.2 MB"; break;
		    case 84: banks = 96; romsize = "1.5 MB"; break;
		    default: cout << "MMU::Error - Unrecognzied ROM quantity given in cartridge" << endl; exit(1); break;
		}

		return banks;
	    }

	    inline int getrambanks(vector<uint8_t>& rom)
	    {
		int banks = 0;

		if (gbmbc == MBCType::TPP1)
		{
		    switch (rom[0x0152])
		    {
			case 0: banks = 0; mbcramsize = 0; ramsize = "None"; break;
			case 1: banks = 1; mbcramsize = 8; ramsize = "8 KB"; break;
			case 2: banks = 2; mbcramsize = 16; ramsize = "16 KB"; break;
			case 3: banks = 4; mbcramsize = 32; ramsize = "32 KB"; break;
			case 4: banks = 8; mbcramsize = 64; ramsize = "64 KB"; break;
			case 5: banks = 16; mbcramsize = 128; ramsize = "128 KB"; break;
			case 6: banks = 32; mbcramsize = 256; ramsize = "256 KB"; break;
			case 7: banks = 64; mbcramsize = 512; ramsize = "512 KB"; break;
			case 8: banks = 128; mbcramsize = 1024; ramsize = "1 MB"; break;
			case 9: banks = 256; mbcramsize = 2048; ramsize = "2 MB"; break;
			default: cout << "MMU::Error - Unrecognized TPP1 RAM quantity of " << hex << int(rom[0x0152]) << " given in cartridge" << endl; exit(1); break;
		    }

		    return banks;
		}

		switch (rom[0x0149])
		{
		    case 0: banks = 0; mbcramsize = 0; ramsize = "None"; break;
		    case 1: banks = 1; mbcramsize = 2; ramsize = "2 KB"; break;
		    case 2: banks = 1; mbcramsize = 8; ramsize = "8 KB"; break;
		    case 3: banks = 4; mbcramsize = 32; ramsize = "32 KB"; break;
		    case 4: banks = 16; mbcramsize = 128; ramsize = "128 KB"; break;
		    case 5: banks = 8; mbcramsize = 64; ramsize = "64 KB"; break;
	    	    default: cout << "MMU::Error - Unrecognized RAM quantity given in cartridge" << endl; exit(1); break;
		}

		return banks;
	    }

	    inline string determinegametitle(vector<uint8_t>& rom)
	    {
		stringstream temp;
	
		for (int i = 0x134; i < 0x0143; i++)
		{
		    temp << char(rom[i]);
		}

		return temp.str();
    	    }

	    string mbctype;
	    string romsize;
	    string ramsize;

	    bool loadBIOS(vector<uint8_t> data);
	    bool loadROM(vector<uint8_t> data);

	    uint8_t readByte(uint16_t addr);
	    void writeByte(uint16_t addr, uint8_t value);
	    uint16_t readWord(uint16_t addr);
	    void writeWord(uint16_t addr, uint16_t value);

	    uint8_t mbc1read(uint16_t addr);
	    void mbc1write(uint16_t addr, uint8_t value);

	    uint8_t mbc1rread(uint16_t addr);
	    void mbc1rwrite(uint16_t addr, uint8_t value);

	    uint8_t mbc1mread(uint16_t addr);
	    void mbc1mwrite(uint16_t addr, uint8_t value);

	    uint8_t mbc2read(uint16_t addr);
	    void mbc2write(uint16_t addr, uint8_t value);

	    uint8_t mbc3read(uint16_t addr);
	    void mbc3write(uint16_t addr, uint8_t value);

	    uint8_t mbc5read(uint16_t addr);
	    void mbc5write(uint16_t addr, uint8_t value);

	    uint8_t mbc6read(uint16_t addr);
	    void mbc6write(uint16_t addr, uint8_t value);

	    uint8_t mbc7read(uint16_t addr);
	    void mbc7write(uint16_t addr, uint8_t value);

	    uint8_t gbcameraread(uint16_t addr);
	    void gbcamerawrite(uint16_t addr, uint8_t value);

	    uint8_t huc3read(uint16_t addr);
	    void huc3write(uint16_t addr, uint8_t value);

	    uint8_t huc1read(uint16_t addr);
	    void huc1write(uint16_t addr, uint8_t value);

	    uint8_t tpp1read(uint16_t addr);
	    void tpp1write(uint16_t addr, uint8_t value);

	    uint8_t wisdomtreeread(uint16_t addr);
	    void wisdomtreewrite(uint16_t addr, uint8_t value);

	    bool ismulticart = false;

	    uint8_t readIO(uint16_t addr);
	    void writeIO(uint16_t addr, uint8_t value);

	    uint8_t dmastart = 0;

	    enum DmaState : int
	    {
		Inactive = -1,
		Starting = 0,
		Active = 1,
		Paused = 2
	    };

	    enum Bus : int
	    {
		None = -1,
		External = 0,
		Vram = 1
	    };

	    enum HdmaType : int
	    {
		Gdma = 0,
		Hdma = 1,
	    };

	    DmaState oamdmastate = DmaState::Inactive;
	    Bus dmabusblock = Bus::None;

	    uint16_t oamtransferaddr;
	    uint8_t oamtransferbyte;
	    uint8_t oamdmastart;
	    int bytesread = 160;
	    int dmadelay = 0;

	    uint16_t hdmasource = 0;
	    uint16_t hdmadest = 0;
	    int hdmabytestocopy = 0;
	    int hdmabytes = 0;

	    bool ishdmaactive = false;

	    HdmaType hdmatype;
	    DmaState hdmastate = DmaState::Inactive;

	    inline void writedma(uint8_t value)
	    {
		oamdmastart = value;
		oamdmastate = DmaState::Starting;
	    }

	    inline void updateoamdma()
	    {
		if (oamdmastate == DmaState::Starting)
		{
		    if (bytesread != 0)
		    {
			oamtransferaddr = (oamdmastart << 8);
			bytesread = 0;
		    }
		    else
		    {
			oamtransferbyte = dmacopy(oamtransferaddr);
			++bytesread;

			oamdmastate = DmaState::Active;

			if ((oamtransferaddr >= 0x8000) && (oamtransferaddr < 0xA000))
			{
			    dmabusblock = Bus::Vram;
			}
			else
			{
			    dmabusblock = Bus::External;
			}
		    }
		}
		else if (oamdmastate == DmaState::Active)
		{
		    oam[(bytesread - 1)] = oamtransferbyte;

		    if (bytesread == 160)
		    {
			oamdmastate = DmaState::Inactive;
			dmabusblock = Bus::None;
			return;
		    }

		    oamtransferbyte = dmacopy(oamtransferaddr + bytesread);
		    ++bytesread;
		}
	    }

	    inline void activatehdma()
	    {
		if (hdmastate == DmaState::Paused)
		{
		    hdmabytes = 16;
		    hdmastate = DmaState::Starting;
		}
	    }

	    inline void initgbcdma(uint8_t value)
	    {
		hdmatype = testbit(value, 7) ? HdmaType::Hdma : HdmaType::Gdma;
		hdmabytestocopy = (((value & 0x7F) + 1) * 16);
		ishdmaactive = true;
		hdmabytes = 16;

		if ((hdmatype == HdmaType::Hdma) && ((readDirectly(0xFF41) & 0x3) != 0))
		{
		    hdmastate = DmaState::Paused;
		}
		else
		{
		    hdmastate = DmaState::Starting;
		}
	    }

	    inline void updategbcdma()
	    {
		if (hdmastate == DmaState::Starting)
		{
		    hdmastate = DmaState::Active;
		}
		else if (hdmastate == DmaState::Active)
		{
		    executehdma();

		    if (hdmabytestocopy == 0)
		    {
			ishdmaactive = false;
			hdmabytestocopy = 2048;
			hdmastate = DmaState::Inactive;
		    }
		    else if ((hdmatype == HdmaType::Hdma) && (hdmabytes == 0))
		    {
			hdmastate = DmaState::Paused;
		    }
		}
	    }

	    inline void executehdma()
	    {
		int numbytes = min((2 >> doublespeed), hdmabytestocopy);

		if (hdmatype == HdmaType::Hdma)
		{
		    numbytes = min(numbytes, hdmabytes);
		    hdmabytes -= numbytes;
		}

		hdmabytestocopy -= numbytes;

		for (int i = 0; i < numbytes; i++)
		{
		    vram[(hdmadest - 0x8000) + (vrambank * 0x2000)] = dmacopy(hdmasource);

		    hdmadest = ((hdmadest + 1) & 0x9FFF);
		    ++hdmasource;
		}
	    }

	    inline bool hdmainprogress()
	    {
		return ((hdmastate == DmaState::Active) || (hdmastate == DmaState::Starting));
	    }

	    uint8_t readDirectly(uint16_t addr);
	    void writeDirectly(uint16_t addr, uint8_t value);

	    inline uint8_t dmacopy(uint16_t addr)
	    {
		if (addr < 0xE000)
		{
		    return readDirectly(addr);
		}
		else
		{
		    if (hdmastate == DmaState::Active)
		    {
			return readDirectly(addr - 0x4000);
		    }
		    else
		    {
			return readDirectly(addr - 0x2000);
		    }
		}
	    }

	    inline void exitbios()
	    {
		if ((biossize == 0x900) && (cartmem[0x143] != 0x80) && (cartmem[0x143] != 0xC0))
		{
		    gbmode = Mode::DMG;
		}

		biosload = false;
		cout << "MMU::Exiting BIOS..." << endl;
	    }

	    inline void requestinterrupt(int id)
	    {
		if (!ifwrittenthiscycle)
		{
		    interruptflags = setbit(interruptflags, id);
		}
	    }

	    inline void clearinterrupt(int id)
	    {
		if (!ifwrittenthiscycle)
		{
		    interruptflags = resetbit(interruptflags, id);
		}
	    }

	    bool ispending(int id)
	    {
		uint8_t temp = (interruptflags & interruptenabled);
		return testbit(temp, id);
	    }

	    bool requestedenabledinterrupts()
	    {
		return (interruptflags & interruptenabled);
	    }

	    inline void writeif(uint8_t value)
	    {
		interruptflags = (value & 0x1F);
		ifwrittenthiscycle = true;
	    }

	    bool ifwrittenthiscycle = false;
	

     	    bool statinterruptsignal = false;
	    bool previnterruptsignal = false;
	    bool isdivinterrupt = true;

	    uint8_t interruptflags = 0xE1;
	    uint8_t dma = 0x00;
	    uint8_t key1 = 0x00;
	    uint8_t interruptenabled = 0x00;
	    bool dmaactive = false;
	    uint8_t lylastcycle = 0xFF;

	private:
	    vector<string> gamegenie_codes;
	    vector<string> gameshark_codes;
	    void apply_gg_cheats();
	    void apply_gs_cheats();

	    void add_gg_cheat(string cheat_code);
	    void add_gs_cheat(string cheat_code);

    };
};

#endif // LIBMBGB_MMU
