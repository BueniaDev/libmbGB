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
	    void updatelinelcd();
	    void updatepixellcd();
	    void renderscanline();
	    void renderbg();
	    void renderwin();
	    void renderobj();

	    bool coincidence();

	    bool isdotrenderer = false;

	    void lcdchecklyc();

	    void lcdstartline();
	    void renderpixel(int pixel);
	    void renderbgpixel(int pixel);
	    void renderwinpixel(int pixel);
	    void renderobjpixel(int pixel);
	    uint16_t readtiledata(bool map, int x, int y);

	    inline uint16_t hflip(uint16_t data)
	    {
		return (((data & 0x8080) >> 7) | ((data & 0x4040) >> 5) | ((data & 0x2020 >> 3) | ((data & 0x1010 >> 1) | ((data & 0x0808 << 1) | ((data & 0x0404 << 3) | ((data & 0x0202 << 5) | (data & 0x0101 << 7)))))));
	    }

	    void copybuffer();

	    inline bool isdotrender()
	    {
		return isdotrenderer;
	    }

	    inline void setdotrender(bool val)
	    {
		if (val)
		{
		    isdotrenderer = true;
		}
		else
		{
		    isdotrenderer = false;
		}
	    }

	    int pixelx = 0;
	    int numobj = 0;
	    int ppuframes = 0;
	    int bgcolor = 0;
	    int bgidx = 0;
	    int objcolor = 0;
	    int objidx = 0;
	    bool objprior = false;
	    uint16_t bgdata = 0;
	    uint16_t windata = 0;
	    uint16_t objdata = 0;

	    Sprites sprite[10];
	    int sprites;

	    bool lycomparezero = false;
	    bool statinterruptsignal = false;
	    bool previnterruptsignal = false;

	    int scanlinecounter = 452;
	    int currentscanline = 0;

	    void updatepoweronstate(bool wasenabled);

	    RGB framebuffer[160 * 144];
	    uint8_t screenbuff[144][160];

	    uint8_t bgscanline[160];
	    uint8_t winscanline[160];

	    inline uint16_t readvram16(uint16_t addr)
	    {
		uint8_t lo = gpumem.vram[addr];
		uint8_t hi = gpumem.vram[addr + 1];
		return ((hi << 8) | lo);
	    }

	    inline int getdmgcolornum(uint16_t data, uint8_t tilex)
	    {
		int lo = (data & (0x80 >> tilex)) ? 1 : 0;
		int hi = (data & (0x8000 >> tilex)) ? 2 : 0;
		return (hi + lo);
	    }

	    inline int getdmgcolor(int id, uint8_t palette)
	    {
		int temp = (id * 2);
		int temphi = (0x3 << temp);

		return ((palette & temphi) >> temp);
	    }

	    inline int getstatmode()
	    {
		return (gpumem.stat & 0x3);
	    }
    };
};

#endif // LIBMBGB_GPU
