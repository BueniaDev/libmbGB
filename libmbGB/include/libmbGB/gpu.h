// This file is part of libmbGB.
// Copyright (C) 2019 Buenia.ui
// libmbGB is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// libmbGB is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with libmbGB.  If not, see <https://www.gnu.org/licenses/>.

#ifndef LIBMBGB_GPU
#define LIBMBGB_GPU

#include "mmu.h"
#include "libmbgb_api.h"
using namespace gb;

namespace gb
{
    struct RGB
    {
	uint8_t red;
	uint8_t green;
	uint8_t blue;
    };    


    class LIBMBGB_API GPU
    {
	GPU(MMU& memory);
	~GPU();

	MMU& mem;

	void updatelcd();
	void updatepoweronstate();
	void updately();
	void renderscanline();
	void renderbg(int pixel);

	int scanlinecounter = 0;
	int currentscanline = 0;

	RGB framebuffer[160 * 144];

	inline int line153cycles()
	{
	    return 4;
	}

	inline int mode3cycles()
	{
	    int cycles = 256;

	    int scxmod = (mem.scrollx % 8);

	    if ((scxmod > 0) && (scxmod < 5))
	    {
		cycles += 4;
	    }
	    else if (scxmod > 4)
	    {
		cycles += 8;
	    }

	    return cycles;
	}
    };
};