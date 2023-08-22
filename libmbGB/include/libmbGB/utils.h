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

#ifndef LIBMBGB_UTILS_H
#define LIBMBGB_UTILS_H

#include <iostream>
#include <sstream>
#include <cstdint>
#include <cstring>
#include <cassert>
#include <vector>
#include <deque>
#include <array>
#include <exception>
#include <memory>
#include <functional>
#include "libmbgb_api.h"
using namespace std;
using namespace std::placeholders;

namespace gb
{
    enum GBButton
    {
	Right = 0,
	Left = 1,
	Up = 2,
	Down = 3,
	ButtonA = 4,
	ButtonB = 5,
	Select = 6,
	Start = 7
    };

    enum GBContextButton
    {
	ContextUp = 0,
	ContextDown = 1,
	ContextLeft = 2,
	ContextRight = 3,
    };

    inline bool is_xmas()
    {
	time_t t = time(NULL);
	tm *time_ptr = localtime(&t);
	return (time_ptr->tm_mon == 11);
    }

    inline bool is_halloween()
    {
	time_t t = time(NULL);
	tm *time_ptr = localtime(&t);
	return (time_ptr->tm_mon == 9);
    }

    template<typename T>
    bool testbit(T reg, int bit)
    {
	return ((reg >> bit) & 1) ? true : false;
    }

    template<typename T>
    T setbit(T reg, int bit)
    {
	return (reg | (1 << bit));
    }

    template<typename T>
    T resetbit(T reg, int bit)
    {
	return (reg & ~(1 << bit));
    }

    template<typename T>
    T changebit(T reg, int bit, bool is_set)
    {
	if (is_set)
	{
	    return setbit(reg, bit);
	}
	else
	{
	    return resetbit(reg, bit);
	}
    }

    template<typename T, typename U>
    bool inRange(T addr, U low, U high)
    {
	return ((addr >= T(low)) && (addr < T(high)));
    }

    template<typename T, typename U>
    bool inRangeEx(T addr, U low, U high)
    {
	return ((addr >= T(low)) && (addr <= T(high)));
    }

    enum GBDMAState : int
    {
	Inactive = 0,
	Starting = 1,
	Paused = 2,
	Active = 3
    };

    enum GBDMAType : int
    {
	Gdma = 0,
	Hdma = 1
    };

    using voidfunc = function<void()>;
    using voidintfunc = function<void(int)>;
    using voidboolfunc = function<void(bool)>;
    using apufunc = function<void(int16_t, int16_t)>;
    using savefunc = function<vector<uint8_t>()>;
    using loadfunc = function<void(vector<uint8_t>)>;
    using rumblefunc = function<void(double)>;

    struct GBRGB
    {
	uint8_t red;
	uint8_t green;
	uint8_t blue;

	GBRGB() : red(0), green(0), blue(0)
	{

	}

	GBRGB(uint8_t r, uint8_t g, uint8_t b) : red(r), green(g), blue(b)
	{

	}

	static GBRGB fromMonochrome(uint8_t color)
	{
	    return GBRGB(color, color, color);
	}

	static GBRGB from16(uint16_t color)
	{
	    int red16 = (color & 0x1F);
	    int green16 = ((color >> 5) & 0x1F);
	    int blue16 = ((color >> 10) & 0x1F);

	    int red = ((red16 << 3) | (red16 >> 2));
	    int green = ((green16 << 3) | (green16 >> 2));
	    int blue = ((blue16 << 3) | (blue16 >> 2));
	    return GBRGB(red, green, blue);
	}
    };

    enum GBModel
    {
	ModelAuto,
	ModelDmgX,
	ModelCgbX,
    };

    enum GBGameType
    {
	GameTypeDMG,
	GameTypeCGB
    };

    template<class T, size_t N>
    class GBFIFO
    {
	public:
	    GBFIFO() : max_size(N)
	    {

	    }

	    ~GBFIFO()
	    {

	    }

	    void clear()
	    {
		int_data.clear();
	    }

	    bool empty()
	    {
		return int_data.empty();
	    }

	    bool full()
	    {
		return (size() >= max_size);
	    }

	    size_t size()
	    {
		return int_data.size();
	    }

	    void push(T data)
	    {
		if (full())
		{
		    throw overflow_error("FIFO overflow error");
		}

		int_data.push_back(data);
	    }

	    T pop()
	    {
		if (empty())
		{
		    throw underflow_error("FIFO underflow error");
		}

		T data = int_data.front();
		int_data.pop_front();
		return data;
	    }

	    T &at(size_t index)
	    {
		if (index >= size())
		{
		    throw out_of_range("FIFO out-of-range");
		}

		return int_data.at(index);
	    }

	private:
	    deque<T> int_data;
	    size_t max_size = 0;
    };

    enum GBMBCType
    {
	ROMOnly = 0,
	MBC1 = 1,
	MBC2 = 2,
	MBC3 = 3,
	MBC5 = 5,
	MBC7 = 7,
	MMM01 = 8,
	PocketCamera = 10,
	HuC3 = 12,
	HuC1 = 13,
	M161 = 15,
	WisdomTree = 16
    };

    class LIBMBGB_API mbGBUnmappedMemory : public exception
    {
	public:
	    mbGBUnmappedMemory(uint16_t addr, bool is_io)
	    {
		uint16_t addr_base = (is_io) ? 0xFF00 : 0;
		uint16_t mem_addr = (addr_base + addr);
		stringstream ss;
		ss << "Reading from unimplemented address of " << hex << int(mem_addr) << endl;
		message = ss.str();
	    }

	    mbGBUnmappedMemory(uint16_t addr, uint8_t data, bool is_io)
	    {
		uint16_t addr_base = (is_io) ? 0xFF00 : 0;
		uint16_t mem_addr = (addr_base + addr);
		stringstream ss;
		ss << "Writing value of " << hex << int(data) << " to unimplemented address of " << hex << int(mem_addr) << endl;
		message = ss.str();
	    }

	    virtual const char *what() const noexcept override
	    {
		return message.c_str();
	    }

	private:
	    string message;
    };

    class LIBMBGB_API mbGBFrontend
    {
	public:
	    mbGBFrontend()
	    {

	    }

	    ~mbGBFrontend()
	    {

	    }

	    virtual bool init()
	    {
		return true;
	    }

	    virtual void shutdown()
	    {
		return;
	    }

	    virtual vector<uint8_t> loadFile(string)
	    {
		vector<uint8_t> data;
		return data;
	    }

	    virtual void saveFile(string, vector<uint8_t>)
	    {
		return;
	    }

	    virtual void audioCallback(int16_t, int16_t)
	    {
		return;
	    }

	    virtual void rumbleCallback(double)
	    {
		return;
	    }
    };

    class mbGBSavestate
    {
	public:
	    mbGBSavestate()
	    {

	    }

	    ~mbGBSavestate()
	    {

	    }

	    bool open(size_t size)
	    {
		vector<uint8_t> data;
		data.resize(size, 0);
		return open(data, true);
	    }

	    bool open(vector<uint8_t> data, bool is_save)
	    {
		loc_pos = 0;
		int_data = vector<uint8_t>(data.begin(), data.end());
		is_saving = is_save;

		if (is_saving)
		{
		    prepSave();
		    loc_pos += 4;
		}
		else
		{
		    string magic_str = readMagic();
	
		    if (magic_str != "MBGB")
		    {
			cout << "Invalid magic bytes" << endl;
			return false;
		    }

		    uint32_t version_major = read16();

		    uint32_t version_minor = read16();

		    if (version_major != ver_major)
		    {
			cout << "Invalid major version" << endl;
			return false;
		    }

		    if (version_minor != ver_minor)
		    {
			cout << "Invalid minor version" << endl;
			return false;
		    }

		    uint32_t length = read32();

		    if (length != data.size())
		    {
			cout << "Invalid length" << endl;
			return false;
		    }
		}

		current_section = -1;
		return true;
	    }

	    vector<uint8_t> getData()
	    {
		writeLength();
		return vector<uint8_t>(int_data.begin(), (int_data.begin() + loc_pos));
	    }

	    void close()
	    {
		int_data.clear();
	    }

	    void section(string magic_str)
	    {
		if (is_saving)
		{
		    writeSectLength();

		    current_section = loc_pos;

		    writeMagic(magic_str);
		    loc_pos += 8;
		}
		else
		{
		    loc_pos = 12;

		    while (true)
		    {
			string magic = readMagic();

			if (magic != magic_str)
			{
			    if (loc_pos >= int_data.size())
			    {
				cout << "Warning: section of " << magic_str << " not found." << endl;
				return;
			    }

			    uint32_t length = read32();
			    loc_pos += (length - 8);
			    continue;
			}

			loc_pos += 8;
			break;
		    }
		}
	    }

	    void var8(uint8_t &data)
	    {
		if (is_saving)
		{
		    write8(data);
		}
		else
		{
		    data = read8();
		}
	    }

	    void var16(uint16_t &data)
	    {
		if (is_saving)
		{
		    write16(data);
		}
		else
		{
		    data = read16();
		}
	    }

	    void var32(uint32_t &data)
	    {
		if (is_saving)
		{
		    write32(data);
		}
		else
		{
		    data = read32();
		}
	    }

	    void var64(uint64_t &data)
	    {
		if (is_saving)
		{
		    write64(data);
		}
		else
		{
		    data = read64();
		}
	    }

	    void varInt(int &data)
	    {
		if (is_saving)
		{
		    write32(uint32_t(data));
		}
		else
		{
		    data = read32();
		}
	    }

	    void varBool32(bool &data)
	    {
		if (is_saving)
		{
		    uint32_t val = data ? 1 : 0;
		    write32(val);
		}
		else
		{
		    data = (read32() != 0);
		}
	    }

	    template<typename T>
	    void varType(T &data)
	    {
		if (is_saving)
		{
		    write32(static_cast<uint32_t>(data));
		}
		else
		{
		    data = static_cast<T>(read32());
		}
	    }

	    template<size_t Size>
	    void varArray8(array<uint8_t, Size> &data)
	    {
		if (is_saving)
		{
		    for (size_t i = 0; i < Size; i++)
		    {
			write8(data.at(i));
		    }
		}
		else
		{
		    for (size_t i = 0; i < Size; i++)
		    {
			data.at(i) = read8();
		    }
		}
	    }

	    void varVec8(vector<uint8_t> &data)
	    {
		if (is_saving)
		{
		    write64(data.size());

		    for (size_t i = 0; i < data.size(); i++)
		    {
			write8(data.at(i));
		    }
		}
		else
		{
		    size_t size = read64();

		    data.resize(size, 0);

		    for (size_t i = 0; i < data.size(); i++)
		    {
			data.at(i) = read8();
		    }
		}
	    }

	private:
	    vector<uint8_t> int_data;
	    size_t loc_pos = 0;
	    uint16_t ver_major = 0;
	    uint16_t ver_minor = 1;

	    int current_section = -1;

	    bool is_saving = false;

	    uint8_t read8()
	    {
		if (loc_pos >= int_data.size())
		{
		    cout << "Out of range" << endl;
		    throw runtime_error("mbGB error");
		}

		return int_data.at(loc_pos++);
	    }

	    uint16_t read16()
	    {
		uint16_t high = read8();
		uint16_t low = read8();
		return ((high << 8) | low);
	    }

	    uint32_t read32()
	    {
		uint32_t high = read16();
		uint32_t low = read16();
		return ((high << 16) | low);
	    }

	    uint64_t read64()
	    {
		uint64_t high = read32();
		uint64_t low = read32();
		return ((high << 32) | low);
	    }

	    void write8(uint8_t data)
	    {
		if (loc_pos >= int_data.size())
		{
		    cout << "Out of range" << endl;
		    throw runtime_error("mbGB error");
		}

		int_data.at(loc_pos++) = data;
	    }

	    void write16(uint16_t data)
	    {
		write8((data >> 8));
		write8((data & 0xFF));
	    }

	    void write32(uint32_t data)
	    {
		write16((data >> 16));
		write16((data & 0xFFFF));
	    }

	    void write64(uint64_t data)
	    {
		write32((data >> 32));
		write32((data & 0xFFFFFFFF));
	    }

	    string readMagic()
	    {
		string magic_str;
		for (int i = 0; i < 4; i++)
		{
		    magic_str.push_back(read8());
		}

		return magic_str;
	    }

	    void writeMagic(string str)
	    {
		if (str.size() < 4)
		{
		    cout << "Invalid string size" << endl;
		    throw runtime_error("mbGB error");
		}

		for (int i = 0; i < 4; i++)
		{
		    write8(str.at(i));
		}
	    }

	    void prepSave()
	    {
		writeMagic("MBGB");
		write16(ver_major);
		write16(ver_minor);
	    }

	    void writeSectLength()
	    {
		if (current_section != -1)
		{
		    uint32_t sect_size = loc_pos;
		    loc_pos = (current_section + 4);

		    uint32_t sect_len = (sect_size - current_section);
		    write32(sect_len);
		    loc_pos = sect_size;
		}
	    }

	    void writeLength()
	    {
		writeSectLength();

		uint32_t loc_size = loc_pos;

		loc_pos = 8;
		write32(loc_size);
		loc_pos = loc_size;
	    }
    };

    class LIBMBGB_API mbGBMapper
    {
	public:
	    mbGBMapper()
	    {

	    }

	    ~mbGBMapper()
	    {

	    }

	    void setRumbleCallback(rumblefunc cb)
	    {
		rumble_func = cb;
	    }

	    void configure(int flags, int rom_banks, int ram_banks)
	    {
		is_ram_active = testbit(flags, 0);
		is_battery_active = testbit(flags, 1);
		num_rom_banks = rom_banks;
		num_ram_banks = ram_banks;
		configureMapper((flags >> 2));
	    }

	    void init(vector<uint8_t> &rom)
	    {
		cart_rom = rom;
		initMapper();
	    }

	    void shutdown()
	    {
		cart_rom.clear();
		shutdownMapper();
	    }

	    void doSavestate(mbGBSavestate &file)
	    {
		file.varInt(num_rom_banks);
		file.varInt(num_ram_banks);
		file.varBool32(is_ram_active);
		file.varBool32(is_battery_active);
		handleSavestate(file);
	    }

	    virtual uint8_t readByte(uint16_t addr)
	    {
		throw mbGBUnmappedMemory(addr, false);
		return 0;
	    }

	    virtual void writeByte(uint16_t addr, uint8_t data)
	    {
		throw mbGBUnmappedMemory(addr, data, false);
		return;
	    }

	    virtual void configureMapper(int)
	    {
		return;
	    }

	    virtual void initMapper()
	    {
		return;
	    }

	    virtual void shutdownMapper()
	    {
		return;
	    }

	    virtual void contextKeyChanged(GBContextButton, bool)
	    {
		return;
	    }

	    virtual vector<uint8_t> saveBackup()
	    {
		vector<uint8_t> data;
		return data;
	    }

	    virtual void loadBackup(vector<uint8_t>)
	    {
		return;
	    }

	    virtual void updateAccel(float, float)
	    {
		return;
	    }

	    virtual void tickMapper()
	    {
		return;
	    }

	    virtual void handleVBlank()
	    {
		return;
	    }

	    virtual void handleSavestate(mbGBSavestate&)
	    {
		return;
	    }

	protected:
	    uint8_t fetchROM(uint32_t addr)
	    {
		addr %= cart_rom.size();
		return cart_rom.at(addr);
	    }

	    int getROMSize()
	    {
		return cart_rom.size();
	    }

	    int wrapROMBanks(int bank_num)
	    {
		int num_banks = max(1, num_rom_banks);
		return (bank_num % num_banks);
	    }

	    int wrapRAMBanks(int bank_num)
	    {
		int num_banks = max(1, num_ram_banks);
		return (bank_num % num_banks);
	    }

	    bool isRAMActive()
	    {
		return is_ram_active;
	    }

	    bool isBatteryActive()
	    {
		return is_battery_active;
	    }

	    int getRAMSize()
	    {
		return (num_ram_banks << 13);
	    }

	    void callRumbleCallback(double strength)
	    {
		if (rumble_func)
		{
		    rumble_func(strength);
		}
	    }

	private:
	    vector<uint8_t> cart_rom;
	    bool is_ram_active = false;
	    bool is_battery_active = false;

	    int num_rom_banks = 0;
	    int num_ram_banks = 0;

	    rumblefunc rumble_func;
    };
};

#endif // LIBMBGB_UTILS_H