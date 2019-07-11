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


    struct Sprites
    {
	uint8_t x;
	uint8_t y;
	uint8_t patternnum;
	bool priority;
	bool yflip;
	bool xflip;
	bool palette;
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

	    bool dotrender = false;


	    inline bool isdotrender()
	    {
		return dotrender;
	    }

	    inline void setdotrender(bool val)
	    {
		dotrender = val;
	    }

	    void copydmgbuffer();

	    void dmgscanline();
	    void renderdmgpixel();
	    void renderdmgbgpixel();
	    void renderdmgwinpixel();
	    void renderdmgobjpixel();

	    void renderscanline();
	    void renderbg();
	    void renderwin();
	    void renderobj();

	    bool lycomparezero = false;
	    bool statinterruptsignal = false;
	    bool previnterruptsignal = false;

	    int scanlinecounter = 452;
	    int currentscanline = 0;

	    void updatepoweronstate(bool wasenabled);

	    RGB framebuffer[160 * 144];
	    uint8_t screenbuffer[144][160];

	    int pixelx = 0;

	    uint16_t bgdata = 0;
	    uint16_t windata = 0;
	    uint16_t objdata = 0;
	    int bgcolor = 0;
	    int bgpalette = 0;
	    int objcolor = 0;
	    int objpalette = 0;
	    bool objprior = false;

	    Sprites sprite[10];
	    int sprites;

	    uint16_t readtiledmg(bool select, int x, int y);

	    uint8_t bgscanline[160];
	    uint8_t winscanline[160];

	    inline uint16_t readvram16(uint16_t addr)
	    {
		uint8_t lo = gpumem.vram[addr];
		uint8_t hi = gpumem.vram[addr + 1];
		return ((hi << 8) | lo);
	    }

	    inline int getdmgcolornum(uint16_t data, int num)
	    {
		int index = 0;
		index |= (data & (0x80 >> num)) ? 1 : 0;
		index |= (data & (0x8000 >> num)) ? 2 : 0;
		return index;
	    }

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
