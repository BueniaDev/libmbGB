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
#include "libmbgb_api.h"
using namespace std;

namespace gb
{
    class LIBMBGB_API mbGBSavestate
    {
	public:
	    mbGBSavestate(bool save, vector<uint8_t> &data);
	    ~mbGBSavestate();

	    bool error;
	    bool issaving;

	    uint32_t vermajor;
	    uint32_t verminor;

	    uint32_t cursection;

	    void section(string magicstr);

	    void var8(uint8_t *var);
	    void var16(uint16_t *var);
	    void var32(uint32_t *var);
	    void var64(uint64_t *var);

	    void vararray(void *data, uint32_t len);

	    bool isatleastversion(uint32_t major, uint32_t minor)
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

	    vector<uint8_t> getsavestatefile()
	    {
		return file;
	    }

	    vector<uint8_t> file;
    };
};

#endif // LIBMBGB_UTILS