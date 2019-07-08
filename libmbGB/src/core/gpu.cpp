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
#include <functional>
#include <utility>
using namespace gb;
using namespace std;

namespace gb
{
    GPU::GPU(MMU& memory) : gpumem(memory)
    {
	gpumem.setstatirqcallback(bind(&GPU::coincidence, this));	
	setdotrender(true);
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

	uint8_t scanline = gpumem.ly;

	switch (gpumem.getstatmode())
	{
	    case 2:
	    {
		pixelx = 0;

		if (scanlinecounter >= 92)
		{
		    if (isdotrender())
		    {
		        lcdstartline();
		    }

		    gpumem.setstatmode(3);

		    if (!isdotrender())
		    {
			renderscanline();
		    }
		}
	    }
	    break;
	    case 3:
	    {
		if (isdotrender())
		{
		    while ((scanlinecounter - 92) >= pixelx)
		    {
		        renderpixel(pixelx++);
		        if (pixelx >= 160)
		        {
			    break;
		        }
		    }
		}

		if (scanlinecounter >= 252)
		{
		    gpumem.setstatmode(0);
		}
	    }
	    break;
	    case 0:
	    case 1:
	    {
		if (scanlinecounter >= 456)
		{
		    scanlinecounter -= 456;

		    scanline++;

		    if (scanline == 154)
		    {
			scanline = 0;

			if (isdotrender())
			{
			    ppuframes++;
			    copybuffer();
			}
		    }

		    if (scanline == 144)
		    {
			gpumem.requestinterrupt(0);			
			gpumem.setstatmode(1);
		    }
		    else if (scanline < 144)
		    {
			gpumem.setstatmode(2);
		    }
		    else
		    {

		    }

		    gpumem.ly = scanline;
		    gpumem.lcdchecklyc();
		}
	    }
	    break;
	}

	gpumem.checkstatinterrupt();
    }

    bool GPU::coincidence()
    {
	return ((gpumem.lyc && (gpumem.ly == gpumem.lyc)) || (!gpumem.lyc && (gpumem.ly == 153)));
    }

    void GPU::copybuffer()
    {
	int color = 0;
	
	for (int i = 0; i < 160; i++)	
	{
	    for (int j = 0; j < 144; j++)
	    {
		int shade = screenbuff[j][i];

		// cout << screenbuff[0][0] << endl;
				
		switch (shade)
		{
		    case 0: color = 0xFF; break;
		    case 1: color = 0xCC; break;
		    case 2: color = 0x77; break;
		    case 3: color = 0x00; break;
		}

		int index = (i + (j * 160));
		framebuffer[index].red = color;
		framebuffer[index].green = color;
		framebuffer[index].blue = color;
	    }
	}
    }

    uint16_t GPU::readtiledata(bool map, int x, int y)
    {
	uint16_t tmaddr = (0x1800 + (map << 10));
	tmaddr += ((((y >> 3) << 5) + (x >> 3)) & 0x03FF);

	uint16_t tileaddr;

	if (TestBit(gpumem.lcdc, 4))
	{
	    tileaddr = (gpumem.vram[tmaddr] << 4);
	}
	else
	{
	    tileaddr = (0x1000 + ((int8_t)(gpumem.vram[tmaddr]) << 4));
	}

	tileaddr += ((y & 7) << 1);

	return readvram16(tileaddr);
    }

    void GPU::lcdstartline()
    {
	pixelx = 0;
	int obsize = BitGetVal(gpumem.lcdc, 2);
	int height = ((obsize == 0) ? 8 : 16);
	sprites = 0;

	for (int addr = 0; addr < (40 * 4); addr += 4)
	{
	    Sprites& obj = sprite[sprites];

	    obj.y = gpumem.oam[addr];
	    obj.x = gpumem.oam[addr + 1];
	    obj.patternnum = gpumem.oam[addr + 2];
	    uint8_t temp = gpumem.oam[addr + 3];
	    obj.priority = TestBit(temp, 7);
	    obj.yflip = TestBit(temp, 6);
	    obj.xflip = TestBit(temp, 5);
	    obj.palette = TestBit(temp, 4);

	    uint8_t tempy = (gpumem.ly - (obj.y - 16));

	    if (tempy >= height)
	    {
		continue;
	    }

	    if (obj.y == 0 || obj.y >= 160)
	    {
		continue;
	    }

	    if (++sprites == 10)
	    {
		break;
	    }
	}

	for (int lo = 0; lo < sprites; lo++)
	{
	    for (int hi = (lo + 1); hi < sprites; hi++)
	    {
		if (sprite[hi].x < sprite[lo].x)
		{
		    swap(sprite[lo], sprite[hi]);
		}
	    }
	}
    }
    
    void GPU::renderpixel(int pixel)
    {
	bgcolor = 0;
	bgidx = 0;
	objcolor = 0;
	objidx = 0;
		
	if (gpumem.isbgenabled())
	{
	    renderbgpixel(pixel);
	}

	if (gpumem.iswinenabled())
	{
	    renderwinpixel(pixel);
	}

	if (gpumem.isobjenabled())
	{
	    renderobjpixel(pixel);
	}

	int color;

	if (objidx == 0)
	{
	    color = bgcolor;
	}
	else if (bgidx == 0)
	{
	    color = objcolor;
	}
	else if (objprior)
	{
	    color = objcolor;
	}
	else
	{
	    color = bgcolor;
	}

	uint8_t scanline = gpumem.ly;
	screenbuff[scanline][pixel] = color;
    }

    void GPU::renderbgpixel(int pixel)
    {
	uint8_t scanline = gpumem.ly;
	uint8_t scrolly = gpumem.scrolly;
	uint8_t scrollx = gpumem.scrollx;
	int sx = ((scrollx + pixel) & 0xFF);
	int sy = ((scrolly + scanline) & 0xFF);
	int tx = ((scrollx + pixel) % 8);

	if (tx == 0 || pixel == 0)
	{
	    bgdata = readtiledata(TestBit(gpumem.lcdc, 3), sx, sy);
	}

	bgidx = getdmgcolornum(bgdata, tx);
	bgcolor = getdmgcolor(bgidx, gpumem.readByte(0xFF47));
    }

    void GPU::renderwinpixel(int pixel)
    {
	uint8_t scanline = gpumem.ly;
	uint8_t windowy = gpumem.windowy;
	uint8_t windowx = gpumem.windowx;
	int sx = (pixel - windowx + 7);
	int sy = (scanline - windowy);

	if (sx < 0)
	{
	    return;
	}

	if (sy < 0)
	{
	    return;
	}

	int tx = (sx % 8);
	if (tx == 0 || pixel == 0)
	{
	    windata = readtiledata(TestBit(gpumem.lcdc, 6), sx, sy);
	}

	bgidx = getdmgcolornum(windata, tx);
	bgcolor = getdmgcolor(bgidx, gpumem.bgpalette);
    }

    void GPU::renderobjpixel(int pixel)
    {
	int height = (!TestBit(gpumem.lcdc, 2) ? 8 : 16);

	objidx = 0;
	objcolor = 0;
	
	for (int i = (sprites - 1); i >= 0; i--)
	{
	    Sprites& obj = sprite[i];

	    int tx = (pixel - (obj.x - 8));
	    uint8_t ty = (gpumem.ly - (obj.y - 16));

	    if (tx < 0 || tx > 7)
	    {
		continue;
	    }

	    if (obj.xflip)
	    {
		tx = (7 - tx);
	    }

	    if (obj.yflip)
	    {
		ty = ((height - 1) - ty);
	    }

	    uint16_t tileaddr = ((obj.patternnum << 4) + (ty << 1));
	    objdata = readvram16(tileaddr);

	    int temp = getdmgcolornum(objdata, tx);

	    if (temp == 0)
	    {
		continue;
	    }

	    objidx = temp;

	    uint16_t palette = (obj.palette) ? 0xFF49 : 0xFF48;
	    objcolor = getdmgcolor(objidx, gpumem.readByte(palette));
	    objprior = !obj.priority;
	}
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

	ypos = gpumem.scrolly + gpumem.ly;

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

	    uint8_t scanline = gpumem.ly;

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

	if (windowy > gpumem.ly)
	{
	    return;
	}

	bool unsig = TestBit(gpumem.lcdc, 4);
	uint16_t tiledata = (unsig) ? 0x8000 : 0x8800;
	uint16_t bgmem = TestBit(gpumem.lcdc, 6) ? 0x9C00 : 0x9800;

	uint8_t ypos = (gpumem.ly - windowy);

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

	    uint8_t scanline = gpumem.ly;

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

	uint8_t scanline = gpumem.ly;

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
