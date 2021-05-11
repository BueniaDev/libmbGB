/*
    This file is part of libmbGB.
    Copyright (C) 2021 BueniaDev.

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

#ifndef LIBMBGB_GPU
#define LIBMBGB_GPU

#include "mmu.h"
#include "libmbgb_api.h"
#include <functional>
using namespace gb;
using namespace std;

namespace gb
{
    using pixelfunc = function<void()>;

    struct Sprites
    {
	uint8_t x;
	uint8_t y;
	uint8_t patternnum;
	bool priority;
	bool yflip;
	bool xflip;
	bool palette;
	bool cgbbank;
	int cgbpalette;
    };  

    class LIBMBGB_API GPU
    {
	public:
	    GPU(MMU& memory);
	    ~GPU();

	    MMU& gpumem;

	    void init();
	    void shutdown();

	    void dosavestate(mbGBSavestate &file);

	    inline void clearscreen()
	    {
		for (int i = 0; i < (160 * 144); i++)
		{
		    framebuffer[i].red = 0xFF;
		    framebuffer[i].green = 0xFF;
		    framebuffer[i].blue = 0xFF;
		}
	    }

	    void updatelcd();
	    void updately();
	    void updatelycomparesignal();
	    bool isly0 = false;

	    bool dotrender = false;
		
	    uint8_t lcdc = 0x91;
	    uint8_t stat = 0x01;
	    uint8_t scrolly = 0x00;
	    uint8_t scrollx = 0x00;
	    uint8_t windowy = 0x00;
	    uint8_t windowx = 0x00;
	    uint8_t ly = 0x00;
	    uint8_t lylastcycle = 0xFF;
	    uint8_t lyc = 0x00;
	    uint8_t bgpalette = 0xFC;
	    uint8_t objpalette0 = 0xFF;
	    uint8_t objpalette1 = 0xFF;

	    uint8_t scrollxtemp = 0;
		
	    uint8_t readlcd(uint16_t addr);
	    void writelcd(uint16_t addr, uint8_t val);
	    
	    pixelfunc drawpixels;
	    
	    void setpixelcallback(pixelfunc cb)
	    {
	        drawpixels = cb;
	    }
		
	    inline bool islcdenabled()
	    {
		return TestBit(lcdc, 7);
	    }

	    inline bool iswinenabled()
	    {
		return TestBit(lcdc, 5);
	    }

	    inline bool isobjenabled()
	    {
		return TestBit(lcdc, 1);
	    }

	    inline bool isbgenabled()
	    {
		return TestBit(lcdc, 0);
	    }

	    inline void setstatmode(int mode)
	    {
		stat = ((stat & 0xFC) | mode);
	    }
		
	    inline void writelcdc(uint8_t value)
	    {
		bool wasenabled = TestBit(value, 7);
		lcdc = value;
		updatepoweronstate(wasenabled);
	    }
		
	    inline void writestat(uint8_t value)
	    {
		stat = ((value & 0x78) | (stat & 0x07));
		if ((gpumem.isdmgconsole() || gpumem.ishybridconsole()) && TestBit(lcdc, 7) && !TestBit(stat, 1))
		{
		    statinterruptsignal = true;
		}
	    }

	    inline void setlycompare(bool cond)
	    {
		if (cond)
		{
		    stat = BitSet(stat, 2);
		}
		else
		{
		    stat = BitReset(stat, 2);
		}
	    }

	    inline int getstatmode()
	    {
		return (stat & 0x3);
	    }

	    inline void checkstatinterrupt()
	    {
		statinterruptsignal |= (TestBit(stat, 3) && getstatmode() == 0);
		statinterruptsignal |= (TestBit(stat, 4) && getstatmode() == 1);
		statinterruptsignal |= (TestBit(stat, 5) && getstatmode() == 2);
		statinterruptsignal |= (TestBit(stat, 6) && TestBit(stat, 2));

		if (statinterruptsignal && !previnterruptsignal)
		{
		    gpumem.requestinterrupt(1);
		}

		previnterruptsignal = statinterruptsignal;
		statinterruptsignal = false;
	    }


	    inline bool isdotrender()
	    {
		return dotrender;
	    }

	    inline void setdotrender(bool val)
	    {
		dotrender = val;
	    }

	    void copydmgbuffer();

	    int pixelx = 0;

	    void dmgscanline();
	    void renderdmgpixel();
	    void renderdmgbgpixel();
	    void renderdmgwinpixel();
	    void renderdmgobjpixel();

	    void cgbscanline();
	    void rendercgbpixel();
	    void rendercgbbgpixel();
	    void rendercgbwinpixel();
	    void rendercgbobjpixel();

	    inline void scanline()
	    {
		if (gpumem.isdmgconsole())
		{
		    dmgscanline();
		}
		else
		{
		    cgbscanline();
		}
	    }

	    inline void renderpixel()
	    {
		if (gpumem.isdmgconsole())
		{
		    renderdmgpixel();
		}
		else
		{
		    rendercgbpixel();
		}
	    }

	    void renderscanline();
	    void renderbg();
	    void renderwin();
	    void renderobj();

	    bool lycomparezero = false;
	    bool statinterruptsignal = false;
	    bool previnterruptsignal = false;

	    int scanlinecounter = 452;
	    int currentscanline = 0;
	    int windowlinecounter = 0;

	    void updatepoweronstate(bool wasenabled);

	    array<gbRGB, (160 * 144)> framebuffer;
	    uint8_t screenbuffer[144][160];
	    bool bgpriorline[160] = {false};

	    bool accuratecolors = false;

	    inline gbRGB getcolors(int color)
	    {
		gbRGB temp;

		int tempred = (color & 0x1F);
		int tempgreen = ((color >> 5) & 0x1F);
		int tempblue = ((color >> 10) & 0x1F);

		int red = 0;
		int green = 0;
		int blue = 0;

		if (accuratecolors)
		{
		    int acctempr = (tempred * 26 + tempgreen * 4 + tempblue * 2);
		    int acctempg = (tempgreen * 24 + tempblue * 8);
		    int acctempb = (tempred * 6 + tempgreen * 4 + tempblue * 22);

		    red = (min(960, acctempr) >> 2);
		    green = (min(960, acctempg) >> 2);
		    blue = (min(960, acctempb) >> 2);
		}
		else
		{
		    red = ((tempred << 3) | (tempred >> 2));
		    green = ((tempgreen << 3) | (tempgreen >> 2));
		    blue = ((tempblue << 3) | (tempblue >> 2));
		}

		temp.red = red;
		temp.green = green;
		temp.blue = blue;

		return temp;
	    }


	    inline gbRGB getdmgpalette(int color, int offset, bool bg)
	    {
		gbRGB temp;

		int tempc = ((offset + 2) * color);
		
		int tmpr = 0;

		if (bg)
		{
		    tmpr = (gpumem.gbcbgpalette[tempc] | (gpumem.gbcbgpalette[tempc + 1] << 8));
		}
		else
		{
		    tmpr = (gpumem.gbcobjpalette[tempc] | (gpumem.gbcobjpalette[tempc + 1] << 8));
		}

		temp = getcolors(tmpr);

		return temp;
	    }

	    inline bool isdmgmode()
	    {
		return (gpumem.isdmgmode() && !gpumem.biosload);
	    }

	    int linexbias = 0;

	    uint16_t bgdata = 0;
	    uint16_t windata = 0;
	    uint16_t objdata = 0;
	    uint8_t bgattr = 0;
	    uint8_t winattr = 0;
	    int bgcolor = 0;
	    int bgpal = 0;
	    int objcolor = 0;
	    int objpal = 0;
	    bool bgprior = false;
	    bool objprior = false;
	    bool objdmgpalette = false;

	    Sprites sprite[10];
	    int sprites;

	    uint16_t readtiledmg(bool select, int x, int y);
	    uint16_t readtilecgbbg(bool select, int x, int y);
	    uint16_t readtilecgbwin(bool select, int x, int y);

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
		int lo = ((data & 0xFF) & (0x80 >> num)) ? 1 : 0;
		int hi = ((data >> 8) & (0x80 >> num)) ? 2 : 0;
		return (hi + lo);
	    }

	    inline int getdmgcolor(int id, uint8_t palette)
	    {
	        int hi = (2 * id + 1);
	        int lo = (2 * id);
	        int bit1 = ((palette >> hi) & 1);
	        int bit0 = ((palette >> lo) & 1);
	        return ((bit1 << 1) | bit0);
	    }

	    inline int getgbccolor(int id, int color, bool bg)
	    {
		uint8_t idx = ((id * 8) + (color * 2));
		if (bg)
		{
		    return (gpumem.gbcbgpalette[idx] | (gpumem.gbcbgpalette[idx + 1] << 8));
		}
		else
		{
		    return (gpumem.gbcobjpalette[idx] | (gpumem.gbcobjpalette[idx + 1] << 8));
		}
	    }

	    inline int line153cycles()
	    {
		// Note: In this implementation, the scanline counter is incremented
		// by 2 cycles per update in double-speed mode, and 4 cycles per
		// update in single-speed mode, so we need to convert the cycle
		// counts into the appropriate scanline-counter ticks

	        if (gpumem.isdmgconsole())
		{
		    return 4;
		}
		else if (gpumem.isdmgmode())
		{
		    return 8;
		}
		else if (gpumem.doublespeed)
		{
		    // 12 cycles for line 153 divided by 2 scanline counter incrementations in double-speed mode
		    return 6;
		}
		else
		{
		    return 4;
		}
	    }

	    inline int mode3cycles()
	    {
	        int cycles = 256;

	        int scxmod = (scrollx % 8);

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
		return TestBit(stat, 5);
	    }

	    inline bool mode1check()
	    {
		return TestBit(stat, 4);
	    }

	    inline bool mode0check()
	    {
		return TestBit(stat, 3);
	    }

	    inline int statmode()
	    {
		return (stat & 0x03);
	    }

	    inline bool lycompcheck()
	    {
		return TestBit(stat, 6);
	    }

	    inline bool lycompequal()
	    {
		return TestBit(stat, 2);
	    }
    };
};

#endif // LIBMBGB_GPU
