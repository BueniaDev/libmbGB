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
    }

    GPU::~GPU()
    {

    }

    void GPU::init()
    {
	scanlinecounter = 0;
	windowlinecounter = 0;
	clearscreen();
	cout << "GPU::Initialized" << endl;
    }

    void GPU::shutdown()
    {
	cout << "GPU::Shutting down..." << endl;
    }

    void GPU::updatelcd()
    {
	if (!gpumem.islcdenabled()) // Checks if LCD is disabled
	{
	    scanlinecounter = 0; // Disabling this breaks The Powerpuff Girls: Battle Him (obscure title, I know)
	    windowlinecounter = 0;
	    gpumem.ly = 0; // Disabling this...
	    gpumem.setstatmode(0); // or this breaks Dr. Mario
	    return;
	}

	scanlinecounter += 4; // Increment scanline counter

	updately(); // Update LY register
	updatelycomparesignal(); // Update LY=LYC comparision

	if (currentscanline <= 143) // If we are not in VBlank, then cycles between modes 0, 2, & 3
	{
	    if (scanlinecounter == ((gpumem.isdmgmode() || gpumem.doublespeed) ? 4 : 0))
	    {
		gpumem.setstatmode(2); // Set mode to 3
		
		// Dot-based renderer logic
		if (isdotrender())
		{
		    linexbias = (gpumem.scrollx & 7);
		    pixelx = linexbias;

		    scanline();
		}
	    }
	    else if (scanlinecounter == (gpumem.isdmgmode() ? 84 : (80 << gpumem.doublespeed)))
	    {	
		gpumem.setstatmode(3); // Set mode to 3

		// Scanline-based renderer logic
		if (!isdotrender())
		{
		    renderscanline();
		}
	    }
	    else if (scanlinecounter == mode3cycles()) // According to The Cycle-Accurate Gameboy Docs, the number of cycles in mode 3 varies slightly.
	    {		
		gpumem.setstatmode(0); // Set mode to 0

		// HDMA transfer logic
		if (gpumem.isgbcconsole() && gpumem.hdmaactive)
		{
		    if ((gpumem.hdmalength & 0x7F) == 0)
		    {
			gpumem.hdmaactive = false;
		    }
		    gpumem.hdmatransfer();
		    gpumem.hdmalength -= 1;
		}
	    }
	}
	else if (currentscanline == 144) // Checks if we are at the beginning of VBlank
	{
	    if (scanlinecounter == 0 && gpumem.isgbcconsole())
	    {
		gpumem.statinterruptsignal |= TestBit(gpumem.stat, 5);
	    }
	    else if (scanlinecounter == (4 << gpumem.doublespeed))
	    {
		gpumem.requestinterrupt(0); // VBlank
		gpumem.setstatmode(1); // Set mode to 1

		if (gpumem.isdmgconsole())
		{
		    gpumem.statinterruptsignal |= TestBit(gpumem.stat, 5);
		}
	    }
	}

	// Dot-based renderer logic
	if (isdotrender())
	{
	    int16_t currentpixel = ((((scanlinecounter % 456) - 92) & ~3) + linexbias);

	    for (int i = pixelx; i < currentpixel; i++)
	    {
		// Check to ensure that current pixel is within screen boundaries
		if (pixelx >= 160)
		{
		    continue;
		}

		if (pixelx < 0)
		{
		    continue;
		}

		renderpixel(); // Renders current pixel
	    }
	}

	gpumem.checkstatinterrupt(); // Check STAT IRQ signal
    }

    void GPU::updatelycomparesignal()
    {
	if (gpumem.isdmgconsole())
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
	else if (gpumem.doublespeed)
	{
	    if (currentscanline == 153 && scanlinecounter == 12)
	    {
		gpumem.setlycompare(gpumem.lyc == gpumem.lylastcycle);
	    }
	    else
	    {
		gpumem.setlycompare(gpumem.lyc == gpumem.lylastcycle);
		gpumem.lylastcycle = gpumem.ly;
	    }
	}
	else
	{
	    if (scanlinecounter == 452)
	    {
		gpumem.setlycompare(gpumem.lyc == gpumem.lylastcycle);
	    }
	    else if (gpumem.lylastcycle == 153)
	    {
		gpumem.setlycompare(gpumem.lyc == gpumem.lylastcycle);
		gpumem.lylastcycle = gpumem.ly;
	    }
	    else
	    {
		gpumem.setlycompare(gpumem.lyc == gpumem.ly);
		gpumem.lylastcycle = gpumem.ly;
	    }
	}
    }

    void GPU::updatepoweronstate(bool wasenabled)
    {
	if (!wasenabled && gpumem.islcdenabled())
	{
	    // Set scanline counter to these values so it automatically ticks over to 0
	    if (gpumem.doublespeed)
	    {
		scanlinecounter = 908;
	    }
	    else
	    {
		scanlinecounter = 452;
	    }
	    currentscanline = 153;
	}
	else if (wasenabled && !gpumem.islcdenabled())
	{
	    scanlinecounter = 0; // Disabling this breaks The Powerpuff Girls: Battle Him (obscure title, I know)
	    windowlinecounter = 0;
	    gpumem.ly = 0;
	    gpumem.setstatmode(0);
	}
    }

    void GPU::updately()
    {
	if (currentscanline == 153 && scanlinecounter == line153cycles())
	{
	    gpumem.ly = 0;
	}

	// LY increments once every 456 cycles (or every 912 cycles in double speed mode)
	if (scanlinecounter == (456 << gpumem.doublespeed)) 
	{
	    scanlinecounter = 0;

	    if (gpumem.isgbcmode() && !gpumem.doublespeed && currentscanline != 153)
	    {
		gpumem.ly = currentscanline;
	    }

	    if (currentscanline == 153) // If current scanline is 153...
	    {
		if (gpumem.isdmgconsole())
		{
		    gpumem.setstatmode(0);
		}

		currentscanline = 0; // then reset to 0
		windowlinecounter = 0;
	    }
	    else
	    {
	        currentscanline = ++gpumem.ly; // Otherwise, increment scanline
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

    void GPU::cgbscanline()
    {
	pixelx = 0;
	int height = ((TestBit(gpumem.lcdc, 2)) ? 16 : 8);
	sprites = 0;

	for (int addr = 0; addr < (40 * 4); addr += 4)
	{
	    Sprites& obj = sprite[sprites];

	    obj.y = gpumem.oam[addr];
	    obj.x = gpumem.oam[addr + 1];
	    obj.patternnum = (gpumem.oam[addr + 2] & ~BitGetVal(gpumem.lcdc, 2));
	    uint8_t temp = gpumem.oam[addr + 3];
	    obj.priority = TestBit(temp, 7);
	    obj.yflip = TestBit(temp, 6);
	    obj.xflip = TestBit(temp, 5);
	    obj.palette = TestBit(temp, 4);
	    obj.cgbbank = TestBit(temp, 3);
	    obj.cgbpalette = (temp & 0x07);

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
    }

    uint16_t GPU::readtiledmg(bool select, int x, int y)
    {
	uint16_t tmaddr = (0x1800 + (select << 10));
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

    uint16_t GPU::readtilecgbbg(bool select, int x, int y)
    {	
	int map = (select) ? 1 : 0;	

	uint16_t tmaddr = (0x1800 + (map << 10));

	tmaddr += ((((y >> 3) << 5) + (x >> 3)) & 0x03FF);

	uint16_t tile = gpumem.vram[tmaddr];
	bgattr = gpumem.vram[tmaddr + 0x2000];

	uint16_t tileaddr = (TestBit(bgattr, 3)) ? 0x2000 : 0x0000;

	if (TestBit(gpumem.lcdc, 4))
	{
	    tileaddr += (tile << 4);
	}
	else
	{
	    tileaddr += (0x1000 + ((int8_t)(tile) << 4));
	}

	y &= 7;

	if (TestBit(bgattr, 6))
	{
	    y = (7 - y);
	}


	tileaddr += (y << 1);
	
	return readvram16(tileaddr);
    }

    uint16_t GPU::readtilecgbwin(bool select, int x, int y)
    {	
	int map = (select) ? 1 : 0;	

	uint16_t tmaddr = (0x1800 + (map << 10));

	tmaddr += ((((y >> 3) << 5) + (x >> 3)) & 0x03FF);

	uint16_t tile = gpumem.vram[tmaddr];
	winattr = gpumem.vram[tmaddr + 0x2000];

	uint16_t tileaddr = (TestBit(winattr, 3)) ? 0x2000 : 0x0000;

	if (TestBit(gpumem.lcdc, 4))
	{
	    tileaddr += (tile << 4);
	}
	else
	{
	    tileaddr += (0x1000 + ((int8_t)(tile) << 4));
	}

	y &= 7;

	if (TestBit(winattr, 6))
	{
	    y = (7 - y);
	}


	tileaddr += (y << 1);
	
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

    void GPU::rendercgbpixel()
    {
	bgcolor = 0;
	bgpalette = 0;
	objcolor = 0;
	objpalette = 0;
	objprior = false;

	rendercgbbgpixel();

	if (gpumem.isgbcconsole() && gpumem.isdmgmode())
	{
	    if (gpumem.isbgenabled() && gpumem.iswinenabled())
	    {
		rendercgbwinpixel();
	    }
	}
	else
	{
	    if (gpumem.iswinenabled())
	    {
		rendercgbwinpixel();
	    }
	}

	if (gpumem.isobjenabled())
	{
	    rendercgbobjpixel();
	}

	int color = 0;
	bool isobjcolor = false;

	if (objpalette == 0)
	{
	    color = bgcolor;
	    isobjcolor = false;
	}
	else if (bgpalette == 0)
	{
	    color = objcolor;
	    isobjcolor = true;
	}
	else if (!TestBit(gpumem.lcdc, 0))
	{
	    color = objcolor;
	    isobjcolor = true;
	}
	else if (bgprior)
	{
	    color = bgcolor;
	    isobjcolor = false;
	}
	else if (objprior)
	{
	    color = objcolor;
	    isobjcolor = true;
	}
	else
	{
	    color = bgcolor;
	    isobjcolor = false;
	}

	int red = 0;
	int green = 0;
	int blue = 0;

	if (gpumem.isdmgmode() && !gpumem.biosload)
	{
	    if (!isobjcolor)
	    {
	        red = getdmgpalette(color, 0, true).red;
	        green = getdmgpalette(color, 0, true).green;
	        blue = getdmgpalette(color, 0, true).blue;
	    }
	    else
	    {
		if (objdmgpalette)
		{
		    red = getdmgpalette(color, 8, false).red;
	            green = getdmgpalette(color, 8, false).green;
	            blue = getdmgpalette(color, 8, false).blue;
		}
		else
		{
		    red = getdmgpalette(color, 0, false).red;
	            green = getdmgpalette(color, 0, false).green;
	            blue = getdmgpalette(color, 0, false).blue;
		}
	    }
	}
	else
	{
	    int tempred = (color & 0x1F);
	    int tempgreen = ((color >> 5) & 0x1F);
	    int tempblue = ((color >> 10) & 0x1F);

	    red = ((tempred << 3) | (tempred >> 2));
	    green = ((tempgreen << 3) | (tempgreen >> 2));
	    blue = ((tempblue << 3) | (tempblue >> 2));
	}

	if ((pixelx < 0) || (pixelx >= 160))
	{
	    return;
	}

	int index = (pixelx + (gpumem.ly * 160));
	framebuffer[index].red = red;
	framebuffer[index].green = green;
	framebuffer[index].blue = blue;
	pixelx += 1;
    }

    void GPU::renderdmgbgpixel()
    {
	uint8_t scrolly = ((gpumem.ly + gpumem.scrolly) & 0xFF);
	uint8_t scrollx = ((pixelx + gpumem.scrollx) & 0xFF);
	int tx = (scrollx & 7);

	if (tx == 0 || pixelx == 0)
	{
	    bgdata = readtiledmg(TestBit(gpumem.lcdc, 3), scrollx, scrolly);
	}

	bgpalette = getdmgcolornum(bgdata, tx);
	bgcolor = getdmgcolor(bgpalette, gpumem.bgpalette);
    }

    void GPU::rendercgbbgpixel()
    {
	int scrolly = ((gpumem.ly + gpumem.scrolly) & 0xFF);
	int scrollx = ((pixelx + gpumem.scrollx) & 0xFF);
	int tx = (scrollx & 7);

	if (tx == 0 || pixelx == 0)
	{
	    bgdata = readtilecgbbg(TestBit(gpumem.lcdc, 3), scrollx, scrolly);
	}

	if (TestBit(bgattr, 5))
	{
	    tx = (7 - tx);
	}

	bgpalette = getdmgcolornum(bgdata, tx);

	if (gpumem.isdmgmode() && !gpumem.biosload)
	{
	    bgcolor = getdmgcolor(bgpalette, gpumem.bgpalette);
	}
	else
	{
	    bgcolor = getgbccolor((bgattr & 0x07), bgpalette, true);
	}

	bgprior = TestBit(bgattr, 7);
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

    void GPU::rendercgbwinpixel()
    {
	int windowy = gpumem.windowy;
	int windowx = gpumem.windowx;
	int sx = (pixelx + 7 - windowx);
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
	    windata = readtilecgbwin(TestBit(gpumem.lcdc, 6), sx, sy);
	}

	if (TestBit(winattr, 5))
	{
	    tx = (7 - tx);
	}

	bgpalette = getdmgcolornum(windata, tx);

	if (gpumem.isdmgmode() && !gpumem.biosload)
	{
	    bgcolor = getdmgcolor(bgpalette, gpumem.bgpalette);
	}
	else
	{
	    bgcolor = getgbccolor((winattr & 0x07), bgpalette, true);
	}

	bgprior = TestBit(winattr, 7);
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

    void GPU::rendercgbobjpixel()
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

	    uint16_t tileaddr = (obj.cgbbank ? 0x2000 : 0x0000);
	    tileaddr += ((obj.patternnum << 4) + (ty << 1));
	    objdata = readvram16(tileaddr);

	    int temp = getdmgcolornum(objdata, tx);

	    if (temp == 0)
	    {
		continue;
	    }

	    objpalette = temp;

	    if (gpumem.isdmgmode() && !gpumem.biosload)
	    {
		uint8_t palette = (obj.palette) ? gpumem.objpalette1 : gpumem.objpalette0;
		objcolor = getdmgcolor(objpalette, palette);
	        objdmgpalette = obj.palette;
	    }
	    else
	    {
	        objcolor = getgbccolor(obj.cgbpalette, objpalette, false);
	    }

	    objprior = !obj.priority;
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

	ypos = scrolly + gpumem.ly;

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
		bgpriorline[pixel] = TestBit(mapattrib, 7);
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

	    uint8_t scanline = gpumem.ly;

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
	uint8_t windowx = (gpumem.windowx);

	if (windowx <= 7)
	{
	    windowx -= windowx;
	}
	else
	{
	    windowx -= 7;
	}

	if (windowx >= 160 || windowy > currentscanline)
	{
	    return;
	}


	bool unsig = TestBit(gpumem.lcdc, 4);
	uint16_t tiledata = (unsig) ? 0x8000 : 0x8800;
	uint16_t bgmem = TestBit(gpumem.lcdc, 6) ? 0x9C00 : 0x9800;


	int ypos = windowlinecounter;
	windowlinecounter++;

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

		if (gpumem.isgbcconsole() && gpumem.isgbcmode())
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

	        uint8_t scanline = gpumem.ly;


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
		    bool iswhite = (bgscanline[xpixel] == 0 && winscanline[xpixel] == 0);
		    bool isbgwhite = (bgscanline[xpixel] == 0);
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

		    if (gpumem.isgbcmode() && TestBit(gpumem.lcdc, 0) && (bgpriorline[xpixel] && !isbgwhite))
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
