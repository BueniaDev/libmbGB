// This file is part of libmbGB.
// Copyright (C) 2019 Buenia.
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

#ifndef LIBMBGB_GPU
#define LIBMBGB_GPU

#include "mmu.h"
#include "libmbgb_api.h"
#include <functional>
using namespace gb;
using namespace std;

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
	public:
	    GPU(MMU& memory);
	    ~GPU();

	    MMU& gpumem;

	    void init();
	    void shutdown();

	    void updatelcd();
	    void updately();
	    void updatelycomparesignal();
	    void checkstatinterrupt();

	    bool lycomparezero = false;

	    int scanlinecounter = 452;
	    int currentscanline = 0;
	    int pixel = 0;

	    void updatepoweronstate(bool wasenabled);

	    RGB linebuffer[160];
	    RGB framebuffer[160 * 144];

	    inline int getdmgcolor(int id, uint8_t palette)
	    {
	        int hi = (2 * id + 1);
	        int lo = (2 * id);
	        int bit1 = ((palette >> hi) & 1);
	        int bit0 = ((palette >> lo) & 1);
	        return ((bit1 << 1) | bit0);
	    }

	    inline int line153cycles()
	    {
	        return 4;
	    }

	    inline int mode3cycles()
	    {
	        int cycles = 256;

	        int scxmod = (gpumem.scrollx % 8);

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

	    inline bool mode2check()
	    {
		return TestBit(gpumem.stat, 5);
	    }

	    inline bool mode1check()
	    {
		return TestBit(gpumem.stat, 4);
	    }

	    inline bool mode0check()
	    {
		return TestBit(gpumem.stat, 3);
	    }

	    inline int statmode()
	    {
		return (gpumem.stat & 0x03);
	    }

	    inline bool lycompcheck()
	    {
		return TestBit(gpumem.stat, 6);
	    }

	    inline bool lycompequal()
	    {
		return TestBit(gpumem.stat, 2);
	    }
    };
};

#endif // LIBMBGB_GPU