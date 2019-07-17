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
	gpumem.setscreencallback(bind(&GPU::clearscreen, this));
	setdotrender(false);
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

    /*
    void GPU::updatelcd()
    {
	if (!gpumem.islcdenabled())
	{
	    return;
	}

	scanlinecounter += 4;

	updatelycomparesignal();

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
		        dmgscanline();
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
		}
	    }
	    break;
	}

	    		if (isdotrender())
		{
		    while ((scanlinecounter - 84) >= pixelx)
		    {
		        renderdmgpixel();
		        if (pixelx >= 160)
		        {
			    break;
		        }
		    }
		}

	gpumem.checkstatinterrupt();
    }
    */

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
	    if (scanlinecounter == (gpumem.isdmgmode() ? 4 : 0))
	    {		
		gpumem.setstatmode(2);

		if (isdotrender())
		{		
		    dmgscanline();
		}
	    }
	    else if (scanlinecounter == (gpumem.isdmgmode() ? 84 : 80))
	    {		
		gpumem.setstatmode(3);

		if (!isdotrender())
		{
		    renderscanline();
		}
	    }
	    else if (scanlinecounter == mode3cycles())
	    {
		gpumem.setstatmode(0);
	    }
	}
	else if (currentscanline == 144)
	{
	    if (scanlinecounter == 0 && gpumem.isgbcconsole())
	    {
		gpumem.statinterruptsignal |= TestBit(gpumem.stat, 5);
	    }
	    else if (scanlinecounter == 4)
	    {
		gpumem.requestinterrupt(0);
		gpumem.setstatmode(1);

		if (gpumem.isdmgconsole())
		{
		    gpumem.statinterruptsignal |= TestBit(gpumem.stat, 5);
		}
	    }
	}

	if (isdotrender())
	{
	    while ((scanlinecounter - 91) >= pixelx)
	    {
	        renderdmgpixel();
	        if (pixelx >= 160)
	        {
		    break;
	        }
	    }
	}

	gpumem.checkstatinterrupt();
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

    void GPU::dmgscanline()
    {
	pixelx = 0;
	int height = ((TestBit(gpumem.lcdc, 2)) ? 16 : 8);
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

    uint16_t GPU::readtiledmg(bool select, int x, int y)
    {	
	int map = (select) ? 1 : 0;	

	uint16_t tmaddr = (0x1800 + (map << 10));
	uint16_t tileaddr = 0;

	tmaddr += ((((y >> 3) << 5) + (x >> 3)) & 0x03FF);

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

    void GPU::renderdmgpixel()
    {
	bgcolor = 0;
	bgpalette = 0;
	objcolor = 0;
	objpalette = 0;

	if (gpumem.isbgenabled())
	{
	    renderdmgbgpixel();

	    if (gpumem.iswinenabled())
	    {
		renderdmgwinpixel();
	    }
	}

	if (gpumem.isobjenabled())
	{
	    renderdmgobjpixel();
	}

	int color = 0;

	if (objpalette == 0)
	{
	    color = bgcolor;
	}
	else if (bgpalette == 0)
	{
	    color = objcolor;
	}
	else if (!objprior)
	{
	    color = objcolor;
	}
	else
	{
	    color = bgcolor;
	}

	int gbcolor = 0;

	switch (color)
	{
	    case 0: gbcolor = 0xFF; break;
	    case 1: gbcolor = 0xCC; break;
	    case 2: gbcolor = 0x77; break;
	    case 3: gbcolor = 0x00; break;
	}

	if ((pixelx < 0) || (pixelx >= 160))
	{
	    return;
	}

	int index = (pixelx + (gpumem.ly * 160));
	framebuffer[index].red = gbcolor;
	framebuffer[index].green = gbcolor;
	framebuffer[index].blue = gbcolor;
	pixelx += 1;
    }

    void GPU::renderdmgbgpixel()
    {
	int scrolly = ((gpumem.ly + gpumem.scrolly) & 0xFF);
	int scrollx = ((pixelx + gpumem.scrollx) & 0xFF);
	int tx = (scrollx & 7);

	if (tx == 0 || pixelx == 0)
	{
	    bgdata = readtiledmg(TestBit(gpumem.lcdc, 3), scrollx, scrolly);
	}

	bgpalette = getdmgcolornum(bgdata, tx);
	bgcolor = getdmgcolor(bgpalette, gpumem.bgpalette);
    }

    void GPU::renderdmgwinpixel()
    {
	int windowy = gpumem.windowy;
	int windowx = gpumem.windowx;
	int sx = (pixelx - windowx + 7);
	int sy = (gpumem.ly - windowy);

	if (sx < 0)
	{
	    return;
	}

	if (sy < 0)
	{
	    return;
	}

	int tx = (sx % 8);

	if (tx == 0 || pixelx == 0)
	{
	    windata = readtiledmg(TestBit(gpumem.lcdc, 6), sx, sy);
	}

	bgpalette = getdmgcolornum(windata, tx);
	bgcolor = getdmgcolor(bgpalette, gpumem.bgpalette);
    }

    void GPU::renderdmgobjpixel()
    {
	int height = (TestBit(gpumem.lcdc, 2) ? 16 : 8);

	objpalette = 0;
	objcolor = 0;
	
	for (int i = (sprites - 1); i >= 0; i--)
	{
	    Sprites& obj = sprite[i];

	    int tx = (pixelx - (obj.x - 8));
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

	    objpalette = temp;

	    uint8_t palette = (obj.palette) ? gpumem.objpalette1 : gpumem.objpalette0;
	    objcolor = getdmgcolor(objpalette, palette);
	    objprior = obj.priority;
	}
    }

    void GPU::renderscanline()
    {
	if (gpumem.isdmgconsole())
	{
	    if (gpumem.isbgenabled())
	    {
		renderbg();
	    }
	}
	else
	{
	    renderbg();
	}

	if (gpumem.isgbcconsole() && gpumem.isdmgmode())
	{
	    if (gpumem.isbgenabled() && gpumem.iswinenabled())
	    {
		if (currentscanline >= gpumem.windowy)
		{
		    renderwin();
		}
	    }
	}
	else
	{
	    if (gpumem.iswinenabled())
	    {
		if (currentscanline >= gpumem.windowy)
		{
		    renderwin();
		}
	    }
	}

	if (gpumem.isobjenabled())
	{
	    renderobj();
	}
    }

    void GPU::renderbg()
    {	
	uint8_t scrolly = gpumem.scrolly;
	uint8_t scrollx = gpumem.scrollx;
	uint16_t tilemap = TestBit(gpumem.lcdc, 3) ? 0x9C00 : 0x9800;
	uint16_t tiledata = TestBit(gpumem.lcdc, 4) ? 0x8000 : 0x8800;
	bool unsig = TestBit(gpumem.lcdc, 4);

	uint8_t ypos = 0;

	ypos = scrolly + currentscanline;

	uint16_t tilerow = (((uint8_t)(ypos / 8)) * 32);

	for (int pixel = 0; pixel < 160; pixel++)
	{
	    uint8_t xpos = (pixel + scrollx);	    

	    uint16_t tilecol = (xpos / 8);
	    int16_t tilenum = 0;

   	    uint16_t tileaddr = (tilemap + tilerow + tilecol);

	    uint8_t mapattrib = 0;

	    if (unsig)
	    {
	        tilenum = (uint8_t)(gpumem.vram[tileaddr - 0x8000]);
	    }
	    else
	    {
	        tilenum = (int8_t)(gpumem.vram[tileaddr - 0x8000]);
	    }

	    if (gpumem.isgbcconsole() && !isdmgmode())
	    {
		mapattrib = gpumem.vram[tileaddr - 0x6000];
	    }

	    uint16_t tileloc = tiledata;

	    if (unsig)
	    {
	        tileloc += (uint16_t)(tilenum * 16);
	    }
	    else
	    {
	        tileloc += (int16_t)(((tilenum + 128) * 16));
	    }

	    uint16_t banknum = 0x8000;

	    uint8_t line = (ypos % 8);

	    if (gpumem.isgbcconsole() && !isdmgmode())
	    {
		banknum = TestBit(mapattrib, 3) ? 0x6000 : 0x8000;

		line = TestBit(mapattrib, 6) ? (7 - (ypos % 8)) : (ypos % 8);
	    }

	    line *= 2;
	    uint8_t data1 = gpumem.vram[(tileloc + line) - banknum];
	    uint8_t data2 = gpumem.vram[(tileloc + line + 1) - banknum];

	    if (gpumem.isgbcconsole() && !isdmgmode())
	    {
		xpos = TestBit(mapattrib, 5) ? (7 - xpos) : xpos;
	    }

	    int colorbit = (xpos % 8);
	    colorbit -= 7;
	    colorbit *= -1;

	    int colornum = BitGetVal(data2, colorbit);
	    colornum <<= 1;
	    colornum |= BitGetVal(data1, colorbit);

	    int red = 0;
	    int green = 0;
	    int blue = 0;


	    if (gpumem.isdmgconsole())
	    {
	        int color = getdmgcolor(colornum, gpumem.readByte(0xFF47));

	        switch (color)
	        {
	            case 0: red = green = blue = 0xFF; break;
	            case 1: red = green = blue = 0xCC; break;
	            case 2: red = green = blue = 0x77; break;
	            case 3: red = green = blue = 0x00; break;
	        }
	    }
	    else if (gpumem.isgbcconsole() && gpumem.isdmgmode() && !gpumem.biosload)
	    {
		int color = getdmgcolor(colornum, gpumem.readByte(0xFF47));

		red = getdmgpalette(color, 0, true).red;
		green = getdmgpalette(color, 0, true).green;
		blue = getdmgpalette(color, 0, true).blue;
	    }
	    else
	    {
		int color = getgbccolor((mapattrib & 0x7), colornum, true);

		int tempred = (color & 0x1F);
		int tempgreen = ((color >> 5) & 0x1F);
		int tempblue = ((color >> 10) & 0x1F);

		red = ((tempred << 3) | (tempred >> 2));
		green = ((tempgreen << 3) | (tempgreen >> 2));
		blue = ((tempblue << 3) | (tempblue >> 2));
	    }

	    uint8_t scanline = currentscanline;

	    if ((scanline < 0) || (scanline > 144))
	    {
		continue;
	    }

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
	uint8_t windowx = gpumem.windowx;

	if (windowx <= 0x07)
	{
	    windowx -= windowx;
	}
	else
	{
	    windowx -= 7;
	}

	

	bool unsig = TestBit(gpumem.lcdc, 4);
	uint16_t tiledata = (unsig) ? 0x8000 : 0x8800;
	uint16_t bgmem = TestBit(gpumem.lcdc, 6) ? 0x9C00 : 0x9800;

	uint8_t ypos = (currentscanline - windowy);

	uint16_t tilerow = (((ypos / 8)) * 32);

	for (int pixel = 0; pixel < 160; pixel++)
	{
	    if (pixel >= windowx)
	    {
		uint8_t xpos = (pixel - windowx);

		uint16_t tilecol = (xpos / 8);
		int16_t tilenum = 0;

	 	uint16_t tileaddr = (bgmem + tilerow + tilecol);

		uint8_t mapattrib = 0;

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

		if (gpumem.isgbcconsole())
		{
	    	    mapattrib = gpumem.vram[tileaddr - 0x6000];
		}

		uint16_t banknum = 0x8000;

	        uint8_t line = (ypos % 8);

		if (gpumem.isgbcconsole() && gpumem.biosload)
		{
		    banknum = TestBit(mapattrib, 3) ? 0x6000 : 0x8000;

		    line = TestBit(mapattrib, 6) ? (7 - (ypos % 8)) : (ypos % 8);
		}


	        line *= 2;
	        uint8_t data1 = gpumem.vram[(tileloc + line) - banknum];
	        uint8_t data2 = gpumem.vram[(tileloc + line + 1) - banknum];

		if (gpumem.isgbcconsole() && gpumem.isdmgmode() && !gpumem.biosload)
		{
		    xpos = TestBit(mapattrib, 5) ? (7 - xpos) : xpos;
		}

	        int colorbit = (xpos % 8);
	        colorbit -= 7;
	        colorbit *= -1;

	        int colornum = BitGetVal(data2, colorbit);
	        colornum <<= 1;
	        colornum |= BitGetVal(data1, colorbit);

	        int red = 0;
	        int green = 0;
	        int blue = 0;

		if (gpumem.isdmgconsole())
		{
	            int color = getdmgcolor(colornum, gpumem.readByte(0xFF47));

	            switch (color)
	            {
	        	case 0: red = green = blue = 0xFF; break;
	        	case 1: red = green = blue = 0xCC; break;
	        	case 2: red = green = blue = 0x77; break;
	        	case 3: red = green = blue = 0x00; break;
	    	    }
		}
		else if (gpumem.isgbcconsole() && gpumem.isdmgmode() && !gpumem.biosload)
		{
		    int color = getdmgcolor(colornum, gpumem.readByte(0xFF47));

		    red = getdmgpalette(color, 0, true).red;
		    green = getdmgpalette(color, 0, true).green;
		    blue = getdmgpalette(color, 0, true).blue;
		}
		else
		{
		    int color = getgbccolor((mapattrib & 0x7), colornum, true);

		    int tempred = (color & 0x1F);
		    int tempgreen = ((color >> 5) & 0x1F);
		    int tempblue = ((color >> 10) & 0x1F);

		    red = ((tempred << 3) | (tempred >> 2));
	 	    green = ((tempgreen << 3) | (tempgreen >> 2));
		    blue = ((tempblue << 3) | (tempblue >> 2));
		}

	        uint8_t scanline = currentscanline;

		if ((scanline < 0) || (scanline > 144))
		{
		    continue;
		}

	        bgscanline[pixel] = colornum;

	        int index = (pixel + (scanline * 160));
		framebuffer[index].red = red;
		framebuffer[index].green = green;
		framebuffer[index].blue = blue;
	    }
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

	    if (gpumem.isgbcconsole() && !isdmgmode())
	    {
		bank = ((flags & 0x08) >> 3);
	    }

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


		   
		    if (gpumem.isdmgconsole())
		    {
	        	int color = getdmgcolor(colornum, gpumem.readByte(coloraddr));


			switch (color)
			{
			    case 0: red = green = blue = 0xFF; break;
	        	    case 1: red = green = blue = 0xCC; break;
	        	    case 2: red = green = blue = 0x77; break;
	        	    case 3: red = green = blue = 0x00; break;
	    		}
		    }
		    else if (gpumem.isgbcconsole() && gpumem.isdmgmode() && !gpumem.biosload)
		    {
			int color = getdmgcolor(colornum, gpumem.readByte(coloraddr));

			int coloroffset = (BitGetVal(coloraddr, 0) << 3);
			
			red = getdmgpalette(color, coloroffset, false).red;
			green = getdmgpalette(color, coloroffset, false).green;
			blue = getdmgpalette(color, coloroffset, false).blue;
		    }
		    else
		    {
			int color = getgbccolor((flags & 0x7), colornum, false);

			int tempred = (color & 0x1F);
			int tempgreen = ((color >> 5) & 0x1F);
			int tempblue = ((color >> 10) & 0x1F);

			red = ((tempred << 3) | (tempred >> 2));
	 		green = ((tempgreen << 3) | (tempgreen >> 2));
			blue = ((tempblue << 3) | (tempblue >> 2));
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
