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

#include "../../include/libmbGB/gpu.h"
using namespace gb;
using namespace std;

namespace gb
{
    GPU::GPU(MMU& memory) : gpumem(memory)
    {
	gpumem.setpoweroncallback(bind(&GPU::updatepoweronstate, this, placeholders::_1));
    }

    GPU::~GPU()
    {

    }

    void GPU::init()
    {
	cout << "GPU::Initialized" << endl;
    }

    void GPU::shutdown()
    {
	cout << "GPU::Shutting down..." << endl;
    }

    void GPU::updatelcd()
    {
	if (!gpumem.islcdenabled())
	{
	    return;
	}

	scanlinecounter += 4;

	updately();
	updatelycomparesignal();

	if (currentscanline <= 143)
	{
	    if (scanlinecounter == 4)
	    {
		gpumem.setstatmode(2);
	    }
	    else if (scanlinecounter == 84)
	    {
		gpumem.setstatmode(3);
		renderscanline();
	    }
	    else if (scanlinecounter == mode3cycles())
	    {
		gpumem.setstatmode(0);
	    }
	}
	else if (currentscanline == 144)
	{
	    if (scanlinecounter == 4)
	    {
		gpumem.requestinterrupt(0);
		gpumem.setstatmode(1);
	    }
	}

	checkstatinterrupt();
    }

    void GPU::updatelycomparesignal()
    {
	if (lycomparezero)
	{
	    gpumem.setlycompare(gpumem.lyc == gpumem.lylastcycle);

	    lycomparezero = false;
	}
	else if (gpumem.ly != gpumem.lylastcycle)
	{
	    gpumem.setlycompare(false);
	    lycomparezero = true;
	    gpumem.lylastcycle = gpumem.ly;
	}
	else
	{
	    gpumem.setlycompare(gpumem.lyc == gpumem.ly);
	    gpumem.lylastcycle = gpumem.ly;
	}
    }

    void GPU::updatepoweronstate(bool wasenabled)
    {
	if (!wasenabled && gpumem.islcdenabled())
	{
	    scanlinecounter = 452;
	    currentscanline = 153;
	}
	else if (wasenabled && !gpumem.islcdenabled())
	{
	    gpumem.ly = 0;
	    gpumem.setstatmode(0);
	    gpumem.statinterruptsignal = false;
	    gpumem.previnterruptsignal = false;
	}
    }

    void GPU::updately()
    {
	if (currentscanline == 153 && scanlinecounter == line153cycles())
	{
	    gpumem.ly = 0;
	}

	if (scanlinecounter == 456)
	{
	    scanlinecounter = 0;

	    if (currentscanline == 153)
	    {
		gpumem.setstatmode(0);
		currentscanline = 0;
	    }
	    else
	    {
		currentscanline = ++gpumem.ly;
	    }
	}
    }

    void GPU::checkstatinterrupt()
    {
	gpumem.statinterruptsignal |= (mode0check() && statmode() == 0);
	gpumem.statinterruptsignal |= (mode1check() && statmode() == 1);
	gpumem.statinterruptsignal |= (mode2check() && statmode() == 2);
	gpumem.statinterruptsignal |= (lycompcheck() && lycompequal());

	if (gpumem.statinterruptsignal && !gpumem.previnterruptsignal)
	{
	    gpumem.requestinterrupt(1);
	}

	gpumem.previnterruptsignal = gpumem.statinterruptsignal;
	gpumem.statinterruptsignal = false;
    }

    void GPU::renderscanline()
    {	
	if (gpumem.isbgenabled())
	{
	    renderbg();
	}

	if (gpumem.iswinenabled())
	{
	    renderwin();
	}

	if (gpumem.isobjenabled())
	{
	    renderobj();
	}
    }

    void GPU::renderbg()
    {
	uint16_t tilemap = TestBit(gpumem.lcdc, 3) ? 0x9C00 : 0x9800;
	uint16_t tiledata = TestBit(gpumem.lcdc, 4) ? 0x8000 : 0x8800;
	bool unsig = TestBit(gpumem.lcdc, 4);

	uint8_t ypos = 0;

	ypos = gpumem.scrolly + currentscanline;

	uint16_t tilerow = (((uint8_t)(ypos / 8)) * 32);

	for (int pixel = 0; pixel < 160; pixel++)
	{
	    uint8_t xpos = (pixel + gpumem.scrollx);	    

	    uint16_t tilecol = (xpos / 8);
	    int16_t tilenum = 0;

   	    uint16_t tileaddr = (tilemap + tilerow + tilecol);

	    if (unsig)
	    {
	        tilenum = (uint8_t)(gpumem.vram[tileaddr - 0x8000]);
	    }
	    else
	    {
	        tilenum = (int8_t)(gpumem.vram[tileaddr - 0x8000]);
	    }

	    uint16_t tileloc = tiledata;

	    if (unsig)
	    {
	        tileloc += (tilenum * 16);
	    }
	    else
	    {
	        tileloc += (((tilenum + 128) * 16));
	    }

	    uint8_t line = (ypos % 8);

	    line *= 2;
	    uint8_t data1 = gpumem.readByte((tileloc + line));
	    uint8_t data2 = gpumem.readByte((tileloc + line + 1));

	    int colorbit = (xpos % 8);
	    colorbit -= 7;
	    colorbit *= -1;

	    int colornum = BitGetVal(data2, colorbit);
	    colornum <<= 1;
	    colornum |= BitGetVal(data1, colorbit);

	    int red = 0;
	    int green = 0;
	    int blue = 0;

	    int color = getdmgcolor(colornum, gpumem.readByte(0xFF47));

	    switch (color)
	    {
	        case 0: red = green = blue = 0xFF; break;
	        case 1: red = green = blue = 0xCC; break;
	        case 2: red = green = blue = 0x77; break;
	        case 3: red = green = blue = 0x00; break;
	    }

	    uint8_t scanline = currentscanline;

	    bgscanline[pixel] = colornum;

	    int index = (pixel + (scanline * 160));
	    framebuffer[index].red = red;
	    framebuffer[index].green = green;
	    framebuffer[index].blue = blue;
	}
    }

    void GPU::renderwin()
    {
	uint8_t windowy = gpumem.windowy;
	uint8_t windowx = (gpumem.windowx - 7);

	if (windowy > currentscanline)
	{
	    return;
	}

	bool unsig = TestBit(gpumem.lcdc, 4);
	uint16_t tiledata = (unsig) ? 0x8000 : 0x8800;
	uint16_t bgmem = TestBit(gpumem.lcdc, 6) ? 0x9C00 : 0x9800;

	uint8_t ypos = (currentscanline - windowy);

	uint16_t tilerow = (((uint8_t)(ypos / 8)) * 32);

	for (int pixel = 0; pixel < 160; pixel++)
	{
	    uint8_t xpos = (pixel - windowx);

	    uint16_t tilecol = (xpos / 8);
	    int16_t tilenum = 0;

	    uint16_t tileaddr = (bgmem + tilerow + tilecol);

	    if (unsig)
	    {
		tilenum = (uint8_t)(gpumem.vram[tileaddr - 0x8000]);
	    }
	    else
	    {
		tilenum = (int8_t)(gpumem.vram[tileaddr - 0x8000]);
	    }

	    uint16_t tileloc = tiledata;

	    if (unsig)
	    {
		tileloc += (tilenum * 16);
	    }
	    else
	    {
		tileloc += ((tilenum + 128) * 16);
	    }

	    uint8_t line = (ypos % 8);
	    line *= 2;
	    uint8_t data1 = gpumem.readByte(tileloc + line);
	    uint8_t data2 = gpumem.readByte(tileloc + line + 1);

	    int colorbit = (xpos % 8);
	    colorbit -= 7;
	    colorbit *= -1;

	    int colornum = BitGetVal(data2, colorbit);
	    colornum <<= 1;
	    colornum |= BitGetVal(data1, colorbit);

	    int red = 0;
	    int green = 0;
	    int blue = 0;

	    int color = getdmgcolor(colornum, gpumem.readByte(0xFF47));

	    switch (color)
	    {
	        case 0: red = green = blue = 0xFF; break;
	        case 1: red = green = blue = 0xCC; break;
	        case 2: red = green = blue = 0x77; break;
	        case 3: red = green = blue = 0x00; break;
	    }

	    uint8_t scanline = currentscanline;

	    bgscanline[pixel] = colornum;

	    int index = (pixel + (scanline * 160));
	    framebuffer[index].red = red;
	    framebuffer[index].green = green;
	    framebuffer[index].blue = blue;
	}
    }

    void GPU::renderobj()
    {
	uint16_t spritedata = 0x8000;
	int ysize = TestBit(gpumem.lcdc, 2) ? 16 : 8;
	int spritelimit = 40;

	uint8_t scanline = currentscanline;

	for (int i = (spritelimit - 1); i >= 0; i--)
	{
	    uint8_t index = (i * 4);
	    uint8_t ypos = gpumem.readByte(0xFE00 + index) - 16;
	    uint8_t xpos = gpumem.readByte(0xFE00 + index + 1) - 8;
	    uint8_t patternnum = gpumem.readByte(0xFE00 + index + 2);
	    uint8_t flags = gpumem.readByte(0xFE00 + index + 3);

	    bool priority = TestBit(flags, 7);
	    bool yflip = TestBit(flags, 6);
	    bool xflip = TestBit(flags, 5);

	    int bank = 0;

	    uint8_t line = (yflip) ? ((((scanline - ypos - ysize) + 1) * -1)) : ((scanline - ypos));


	    if (TestBit(gpumem.lcdc, 2))
	    {
		if (line < 8)
		{
		    patternnum &= 0xFE;
		}
		else
		{
		    patternnum |= 0x01;
		}

		line = (line & 0x7);
	    }

	    uint8_t data1 = gpumem.vram[spritedata + (patternnum * 16) + (line * 2) + (bank * 0x2000) - 0x8000];
	    uint8_t data2 = gpumem.vram[spritedata + (patternnum * 16) + (line * 2 + 1) + (bank * 0x2000) - 0x8000];

	    if (xpos == 0 && ypos == 0)
	    {
		continue;
	    }

	    if (scanline >= ypos && scanline < (ypos + ysize))
	    {
		for (int pixel = 7; pixel >= 0; pixel--)
		{
		    uint8_t xpixel = (xpos + pixel);
		    int spritepixel = (xflip) ? pixel : ((pixel - 7) * -1);
		    bool iswhite = (bgscanline[xpixel] == 0);
		    int colornum = BitGetVal(data2, spritepixel);
		    colornum <<= 1;
		    colornum |= BitGetVal(data1, spritepixel);
		    uint16_t coloraddr = TestBit(flags, 4) ? 0xFF49 : 0xFF48;

		    int red = 0;
		    int green = 0;
		    int blue = 0;

		    int color = getdmgcolor(colornum, gpumem.readByte(coloraddr));

		    switch (color)
		    {
			case 0: red = 0xFF; green = 0xFF; blue = 0xFF; break;
	    	        case 1: red = 0xCC; green = 0xCC; blue = 0xCC; break;
		        case 2: red = 0x77; green = 0x77; blue = 0x77; break;
		        case 3: red = 0x00; green = 0x00; blue = 0x00; break;
		    }

		    if (xpixel >= 160)
		    {
			continue;
		    }

		    if (colornum == 0)
		    {
			continue;
		    }

		    if ((priority == true && !iswhite))
		    {
			continue;
		    }

		    if ((scanline < 0) || (scanline > 144))
		    {
			continue;
		    }

		    int index = (xpixel + (scanline * 160));
		    framebuffer[index].red = red;
		    framebuffer[index].green = green;
		    framebuffer[index].blue = blue;
		}
	    }
	}
    }
};
