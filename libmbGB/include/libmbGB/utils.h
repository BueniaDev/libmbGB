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
	ButtonA,
	ButtonB,
	Select,
	Start,
	Up,
	Down,
	Left,
	Right
    };

    inline bool is_xmas()
    {
	time_t t = time(NULL);
	tm *time_ptr = localtime(&t);
	return (time_ptr->tm_mon == 11);
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
	ModelCgbX
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
	PocketCamera = 10,
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

    class LIBMBGB_API mbGBMapper
    {
	public:
	    mbGBMapper()
	    {

	    }

	    ~mbGBMapper()
	    {

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

	protected:
	    uint8_t fetchROM(uint32_t addr)
	    {
		addr %= cart_rom.size();
		return cart_rom.at(addr);
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

	private:
	    vector<uint8_t> cart_rom;
	    bool is_ram_active = false;
	    bool is_battery_active = false;

	    int num_rom_banks = 0;
	    int num_ram_banks = 0;
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
    };
};

#endif // LIBMBGB_UTILS_H