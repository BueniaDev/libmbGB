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

#ifndef LIBMBGB_UTILS
#define LIBMBGB_UTILS

#include <cstdint>
#include <memory>
#include <vector>
#include <utility>
#include <cstring>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <functional>
#include <array>
#include <ctime>
#include <variant>
#include "vecfile.h"
#include "libmbgb_api.h"
using namespace std;

namespace gb
{
    // Bit manipulation functions
    // Test bit x of a value
    template<typename T>
    inline bool testbit(T val, int bit)
    {
	return (val & (1 << bit)) ? true : false;
    }

    // Set bit x of a value
    template<typename T>
    inline T setbit(T val, int bit)
    {
	return (val | (1 << bit));
    }

    // Reset bit x of a value
    template<typename T>
    inline T resetbit(T val, int bit)
    {
	return (val & ~(1 << bit));
    }

    // Change bit of value depending on value of "is_set"
    template<typename T>
    inline T changebit(T val, int bit, bool is_set)
    {
	return (is_set) ? setbit(val, bit) : resetbit(val, bit);
    }

    // Get value of bit x of a value
    template<typename T>
    inline int getbitval(T val, int bit)
    {
	return (val & (1 << bit)) ? 1 : 0;
    }

    // Check if a value is in the range of [low:high)
    template<typename T>
    bool inRange(T value, int low, int high)
    {
	int val = int(value);
	return ((val >= low) && (val < high));
    }

    // Check if a value is in the range of [low:high]
    template<typename T>
    bool inRangeEx(T value, int low, int high)
    {
	int val = int(value);
	return ((val >= low) && (val <= high));
    }

    // Custom struct for storing RGB colors
    struct gbRGB
    {
	uint8_t red;
	uint8_t green;
	uint8_t blue;

	bool operator ==(const gbRGB &b) const
	{
	    const gbRGB a = *this;
	    bool result = (a.red == b.red);
	    result &= (a.green == b.green);
	    result &= (a.blue == b.blue);
	    return result;
	}

	bool operator !=(const gbRGB &b) const
	{
	    const gbRGB a = *this;
	    return !(a == b);
	}
    };

    inline uint32_t from_hex_str(string hex_str)
    {
	return strtoul(hex_str.c_str(), NULL, 16);
    }

    inline void gen_noise(array<int, (128 * 120)> &arr)
    {
	srand(time(NULL));

	for (size_t index = 0; index < arr.size(); index++)
	{
	    arr.at(index) = (rand() & 0xFF);
	}
    }

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

    	    virtual void runapp()
	    {
		return;
	    }

    	    virtual void audiocallback(int16_t left, int16_t right)
	    {
		(void)left;
		(void)right;
		return;
	    }

    	    virtual void rumblecallback(double strength)
	    {
		(void)strength;
		return;
	    }

    	    virtual void pixelcallback()
	    {
		return;
	    }

	    virtual vector<uint8_t> loadfile(string filename) = 0;
	    virtual bool savefile(string filename, vector<uint8_t> data) = 0;

	    virtual bool camerainit()
	    {
		return true;
	    }

	    virtual void camerashutdown()
	    {
		return;
	    }

	    virtual bool cameraframe(array<int, (128 * 120)> &arr)
	    {
		gen_noise(arr);
		return true;
	    }

	    virtual void printerframe(vector<gbRGB> &temp, bool appending)
	    {
		(void)appending;
		return;
	    }
    };

    class mbGBSavestate
    {
	public:
	    mbGBSavestate(VecFile &file, bool save);
	    ~mbGBSavestate();

	    bool error = false;

	    bool issaving;
	    uint16_t vermajor;
	    uint16_t verminor;
	    
	    int cursection;

	    void section(const char* magic);
	    void var8(uint8_t *var);
	    void var16(uint16_t *var);
	    void var32(uint32_t *var);
	    void var64(uint64_t *var);
	    void varint(int *var);
	    void bool32(bool *var);
	    void vararray(void *data, uint32_t len);

	    template<typename Enum>
	    void varenum(Enum *var)
	    {
		varint(reinterpret_cast<int*>(var));
	    }

	    bool isatleastversion(uint16_t major, uint16_t minor)
	    {
		if (vermajor > major)
		{
		    return true;
		}

		if ((vermajor == major) && (verminor >= minor))
		{
		    return true;
		}

		return false;
	    }

	    VecFile state_file;

	    VecFile get_savestate_file();
    };
};

#endif // LIBMBGB_UTILS