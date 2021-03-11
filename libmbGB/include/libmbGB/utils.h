// This file is part of libmbGB.
// Copyright (C) 2020 Buenia.
//
// libmbGB is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// libmbGB is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with libmbGB.  If not, see <https://www.gnu.org/licenses/>.

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
    // This type is used for creating the final waveform
    using audiotype = variant<int8_t, int16_t, int32_t, uint8_t, uint16_t, uint32_t, float>;

    // Bit manipulation functions

    // Test bit x of a value
    inline bool TestBit(uint32_t val, int bit)
    {
	return (val & (1 << bit)) ? true : false;
    }

    // Set bit x of a value
    inline uint32_t BitSet(uint32_t val, int bit)
    {
	return (val | (1 << bit));
    }

    // Reset bit x of a value
    inline uint32_t BitReset(uint32_t val, int bit)
    {
	return (val & ~(1 << bit));
    }

    // Change bit of value depending on value of "is_set"
    inline uint32_t BitChange(uint32_t val, int bit, bool is_set)
    {
	return (is_set) ? BitSet(val, bit) : BitReset(val, bit);
    }

    // Get value of bit x of a value
    inline int BitGetVal(uint32_t val, int bit)
    {
	return (val & (1 << bit)) ? 1 : 0;
    }

    // Custom struct for storing RGB colors
    struct gbRGB
    {
	uint8_t red;
	uint8_t green;
	uint8_t blue;
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
    	
    	    virtual bool init() = 0;
    	    virtual void shutdown() = 0;
    	    virtual void runapp() = 0;
    	    virtual void audiocallback(audiotype left, audiotype right) = 0;
    	    virtual void rumblecallback(bool enabled) = 0;
    	    virtual void pixelcallback() = 0;
            virtual vector<uint8_t> loadfile(string filename) = 0;
	    virtual bool savefile(string filename, vector<uint8_t> data) = 0;
	    virtual bool camerainit() = 0;
	    virtual void camerashutdown() = 0;
	    virtual bool cameraframe(array<int, (128 * 120)> &arr) = 0;
	    virtual void printerframe(vector<gbRGB> &temp, bool appending) = 0;
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