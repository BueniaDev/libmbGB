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

#include "gpu.h"
using namespace gb;
using namespace std;
using namespace std::placeholders;

namespace gb
{
    GPU::GPU(MMU& memory) : gpumem(memory)
    {
	for (int i = 0xFF40; i < 0xFF46; i++)
	{
	    gpumem.addmemoryreadhandler(i, bind(&GPU::readlcd, this, _1));
	    gpumem.addmemorywritehandler(i, bind(&GPU::writelcd, this, _1, _2));
	}
	
	for (int i = 0xFF47; i <= 0xFF4B; i++)
	{
	    gpumem.addmemoryreadhandler(i, bind(&GPU::readlcd, this, _1));
	    gpumem.addmemorywritehandler(i, bind(&GPU::writelcd, this, _1, _2));
	}
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
	framebuffer.clear();
	cout << "GPU::Shutting down..." << endl;
    }

    void GPU::dosavestate(mbGBSavestate &file)
    {
	file.section("GPU ");
	file.var8(&lcdc);
	file.var8(&stat);
	file.var8(&scrolly);
	file.var8(&scrollx);
	file.var8(&ly);
	file.var8(&lyc);
	file.var8(&windowy);
	file.var8(&windowx);
	file.var8(&bgpalette);
	file.var8(&objpalette0);
	file.var8(&objpalette1);
    }
	
    uint8_t GPU::readlcd(uint16_t addr)
    {
	uint8_t temp = 0;
		
	switch ((addr & 0xFF))
	{
	    case 0x40: temp = lcdc; break;
	    case 0x41: temp = (stat | 0x80); break;
	    case 0x42: temp = scrolly; break;
	    case 0x43: temp = scrollx; break;
	    case 0x44: temp = ly; break;
	    case 0x45: temp = lyc; break;
	    case 0x47: temp = bgpalette; break;
	    case 0x48: temp = objpalette0; break;
	    case 0x49: temp = objpalette1; break;
	    case 0x4A: temp = windowy; break;
	    case 0x4B: temp = windowx; break;
	    default: temp = 0xFF; break;
	}
		
	return temp;
    }
	
    void GPU::writelcd(uint16_t addr, uint8_t value)
    {
	switch ((addr & 0xFF))
	{
	    case 0x40: writelcdc(value); break;
	    case 0x41: writestat(value); break;
	    case 0x42: scrolly = value; break;
	    case 0x43: scrollx = value; break;
	    case 0x44: break; // LY should not be written to
	    case 0x45: lyc = value; break;
	    case 0x47: bgpalette = value; break;
	    case 0x48: objpalette0 = value; break;
	    case 0x49: objpalette1 = value; break;
	    case 0x4A: windowy = value; break;
	    case 0x4B: windowx = value; break;
	    default: return; break;
	}
    }

    void GPU::updatelcd()
    {
	if (!islcdenabled()) // Checks if LCD is disabled
	{
	    scanlinecounter = 0;
	    currentscanline = 0;
	    windowlinecounter = 0;
	    ly = 0;
	    setstatmode(0);
	    return;
	}

	// Each scanline takes 456 cycles in single-speed mode, or 912 cycles in double-speed mode
	int atomic_increase = (gpumem.doublespeed) ? 2 : 4;

	// Increment scanline counter
	scanlinecounter += atomic_increase;

	updately(); // Update LY register
	updatelycomparesignal(); // Update LY=LYC comparision

	if (currentscanline <= 143) // If we are not in VBlank, then cycles between modes 0, 2, & 3
	{
	    if (scanlinecounter == ((gpumem.isdmgmode() || gpumem.doublespeed) ? 4 : 0))
	    {
		setstatmode(2); // Set mode to 3
		
		// Dot-based renderer logic
		if (isdotrender())
		{
		    linexbias = -(scrollx & 7);
		    pixelx = linexbias;

		    scanline();

		    if ((ly >= windowy) && (windowx < 7))
		    {
			windowlinecounter += 1;
		    }
		}
	    }
	    else if (scanlinecounter == (gpumem.isdmgmode() ? 84 : (80 << gpumem.doublespeed)))
	    {	
		setstatmode(3); // Set mode to 3

		// Scanline-based renderer logic
		if (!isdotrender())
		{
		    renderscanline();
		    updateframebuffer();
		}
	    }
	    else if (scanlinecounter == mode3cycles()) // According to The Cycle-Accurate Gameboy Docs, the number of cycles in mode 3 varies slightly.
	    {		
		setstatmode(0); // Set mode to 0

		// HDMA transfer logic
		if (gpumem.isgbcconsole())
		{
		    gpumem.activatehdma();
		}
	    }
	}
	else if (currentscanline == 144) // Checks if we are at the beginning of VBlank
	{
	    if (scanlinecounter == 0 && gpumem.isgbcconsole())
	    {
		statinterruptsignal |= testbit(stat, 5);
	    }
	    else if (scanlinecounter == (4 << gpumem.doublespeed))
	    {
		gpumem.requestinterrupt(0); // VBlank
		setstatmode(1); // Set mode to 1

		if (gpumem.isdmgconsole())
		{
		    statinterruptsignal |= testbit(stat, 5);
		}

		if (drawpixels)
		{
		    drawpixels();
		}
	    }
	}

	checkstatinterrupt(); // Check STAT IRQ signal

	// Dot-based renderer logic
	if (isdotrender())
	{
	    int16_t currentpixel = (((scanlinecounter % 456 - 80) & ~7) + linexbias);

	    for (; pixelx < currentpixel; pixelx++)
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
    }

    void GPU::updatelycomparesignal()
    {
	if (gpumem.isdmgconsole())
	{
	    if (lycomparezero)
	    {
	        setlycompare(lyc == lylastcycle);

	        lycomparezero = false;
	    }
	    else if (ly != lylastcycle)
	    {
	        setlycompare(false);
	        lycomparezero = true;
	        lylastcycle = ly;
	    }
	    else
	    {
	        setlycompare(lyc == ly);
	        lylastcycle = ly;
	    }
	}
	else if (gpumem.doublespeed)
	{
	    if (currentscanline == 153 && scanlinecounter == 12)
	    {
		setlycompare(lyc == lylastcycle);
	    }
	    else
	    {
		setlycompare(lyc == lylastcycle);
		lylastcycle = ly;
	    }
	}
	else
	{
	    if (scanlinecounter == 452)
	    {
		setlycompare(lyc == lylastcycle);
	    }
	    else if (lylastcycle == 153)
	    {
		setlycompare(lyc == lylastcycle);
		lylastcycle = ly;
	    }
	    else
	    {
		setlycompare(lyc == ly);
		lylastcycle = ly;
	    }
	}
    }

    void GPU::updatepoweronstate(bool wasenabled)
    {
	if (!wasenabled && islcdenabled())
	{
	    // Set scanline counter to these values so it automatically ticks over to 0
	    scanlinecounter = 452;
	    currentscanline = 153;
	}
	else if (wasenabled && !islcdenabled())
	{
	    scanlinecounter = 0;
	    windowlinecounter = 0;
	    ly = 0;
	    setstatmode(0);
	}
    }

    void GPU::updately()
    {
	if (currentscanline == 153 && scanlinecounter == line153cycles())
	{
	    ly = 0;
	}

	// LY increments once every 456 cycles (or every 912 cycles in double speed mode)
	if (scanlinecounter == 456) 
	{
	    scanlinecounter = 0;

	    if (gpumem.isgbcmode() && !gpumem.doublespeed && currentscanline != 153)
	    {
		ly = currentscanline;
	    }

	    if (currentscanline == 153) // If current scanline is 153...
	    {
		if (gpumem.isdmgconsole())
		{
		    setstatmode(0);
		}

		currentscanline = 0; // then reset to 0
		windowlinecounter = 0;
	    }
	    else
	    {
	        currentscanline = ++ly; // Otherwise, increment scanline
	    }
	}
    }

    void GPU::setpixel(int x, int y, gbRGB color)
    {
	render_scanline = y;
	scanlinebuffer[x] = color;
    }

    void GPU::updateframebuffer()
    {
	if (gpumem.isagbconsole())
	{
	    if (is_gba_stretched)
	    {
		int offset = 1920 + (render_scanline * 240);
		int stretched_pos = 0;
		int old_pos = 0;

		for (int x = 0; x < 160; x++)
		{
		    old_pos = x;
		    stretchedbuffer[stretched_pos++] = scanlinebuffer[x++];
		    stretchedbuffer[stretched_pos++] = rgbBlend(scanlinebuffer[old_pos], scanlinebuffer[x]);
		    stretchedbuffer[stretched_pos++] = scanlinebuffer[x];
		}

		for (int x = 0; x < 240; x++)
		{
		    int index = (offset + x);
		    framebuffer[index] = stretchedbuffer[x];
		}

		stretchedbuffer.fill({0, 0, 0});
		scanlinebuffer.fill({0, 0, 0});
	    }
	    else
	    {
		int offset = 1960 + (render_scanline * 240);

		for (int x = 0; x < 160; x++)
		{
		    int index = (offset + x);
		    framebuffer[index] = scanlinebuffer[x];
		}

		scanlinebuffer.fill({0, 0, 0});
	    }
	}
	else
	{
	    for (int x = 0; x < 160; x++)
	    {
		int index = (x + (render_scanline * screenwidth));
		framebuffer[index] = scanlinebuffer[x];
	    }

	    scanlinebuffer.fill({0, 0, 0});
	}
    }

    void GPU::dmgscanline()
    {
	pixelx = 0;
	int height = ((testbit(lcdc, 2)) ? 16 : 8);
	sprites = 0;

	for (int addr = 0; addr < (40 * 4); addr += 4)
	{
	    Sprites& obj = sprite[sprites];

	    obj.y = gpumem.oam[addr];
	    obj.x = gpumem.oam[addr + 1];
	    obj.patternnum = gpumem.oam[addr + 2];
	    uint8_t temp = gpumem.oam[addr + 3];
	    obj.priority = testbit(temp, 7);
	    obj.yflip = testbit(temp, 6);
	    obj.xflip = testbit(temp, 5);
	    obj.palette = testbit(temp, 4);

	    uint8_t tempy = (ly - (obj.y - 16));

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
	int height = ((testbit(lcdc, 2)) ? 16 : 8);
	sprites = 0;

	for (int addr = 0; addr < (40 * 4); addr += 4)
	{
	    Sprites& obj = sprite[sprites];

	    obj.y = gpumem.oam[addr];
	    obj.x = gpumem.oam[addr + 1];
	    obj.patternnum = (gpumem.oam[addr + 2] & ~BitGetVal(lcdc, 2));
	    uint8_t temp = gpumem.oam[addr + 3];
	    obj.priority = testbit(temp, 7);
	    obj.yflip = testbit(temp, 6);
	    obj.xflip = testbit(temp, 5);
	    obj.palette = testbit(temp, 4);
	    obj.cgbbank = testbit(temp, 3);
	    obj.cgbpalette = (temp & 0x07);

	    uint8_t tempy = (ly - (obj.y - 16));

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

	if (testbit(lcdc, 4))
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

	uint8_t tile = gpumem.vram[tmaddr];
	bgattr = gpumem.vram[tmaddr + 0x2000];

	uint16_t tileaddr = (testbit(bgattr, 3)) ? 0x2000 : 0x0000;

	if (testbit(lcdc, 4))
	{
	    tileaddr += (tile << 4);
	}
	else
	{
	    tileaddr += (0x1000 + (int8_t(tile) << 4));
	}

	if (testbit(bgattr, 6))
	{
	    y ^= 7;
	}


	tileaddr += ((y & 7) << 1);
	
	return readvram16(tileaddr);
    }

    uint16_t GPU::readtilecgbwin(bool select, int x, int y)
    {	
	int map = (select) ? 1 : 0;	

	uint16_t tmaddr = (0x1800 + (map << 10));

	tmaddr += ((((y >> 3) << 5) + (x >> 3)) & 0x03FF);

	uint16_t tile = gpumem.vram[tmaddr];
	winattr = gpumem.vram[tmaddr + 0x2000];

	uint16_t tileaddr = (testbit(winattr, 3)) ? 0x2000 : 0x0000;

	if (testbit(lcdc, 4))
	{
	    tileaddr += (tile << 4);
	}
	else
	{
	    tileaddr += (0x1000 + ((int8_t)tile << 4));
	}

	y &= 7;

	if (testbit(winattr, 6))
	{
	    y = (7 - y);
	}


	tileaddr += (y << 1);
	
	return readvram16(tileaddr);
    }

    void GPU::renderdmgpixel()
    {	
	bgcolor = 0;
	bgpal = 0;
	objcolor = 0;
	objpal = 0;

	if (isbgenabled())
	{
	    renderdmgbgpixel();

	    if (iswinenabled())
	    {
		renderdmgwinpixel();
	    }
	}

	if (isobjenabled())
	{
	    renderdmgobjpixel();
	}

	int color = 0;

	if (objpal == 0)
	{
	    color = bgcolor;
	}
	else if (bgpal == 0)
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

	uint8_t dmgcolor = 0;

	switch (color)
	{
	    case 0: dmgcolor = 0xFF; break;
	    case 1: dmgcolor = 0xCC; break;
	    case 2: dmgcolor = 0x77; break;
	    case 3: dmgcolor = 0x00; break;
	}

	if ((pixelx < 0) || (pixelx >= 160))
	{
	    return;
	}

	gbRGB gbcolor = {dmgcolor, dmgcolor, dmgcolor};
	setpixel(pixelx, ly, gbcolor);

	if (pixelx == 159)
	{
	    updateframebuffer();
	}
    }

    void GPU::rendercgbpixel()
    {
	bgcolor = 0;
	bgpal = 0;
	objcolor = 0;
	objpal = 0;
	objprior = false;

	rendercgbbgpixel();

	if (gpumem.isgbcconsole() && gpumem.isdmgmode())
	{
	    if (isbgenabled() && iswinenabled())
	    {
		rendercgbwinpixel();
	    }
	}
	else
	{
	    if (iswinenabled())
	    {
		rendercgbwinpixel();
	    }
	}

	if (isobjenabled())
	{
	    rendercgbobjpixel();
	}

	int color = 0;
	bool isobjcolor = false;

	if (objpal == 0)
	{
	    color = bgcolor;
	    isobjcolor = false;
	}
	else if (bgpal == 0)
	{
	    color = objcolor;
	    isobjcolor = true;
	}
	else if (!testbit(lcdc, 0))
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

	uint8_t red = 0;
	uint8_t green = 0;
	uint8_t blue = 0;

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
	    red = getcolors(color).red;
	    green = getcolors(color).green;
	    blue = getcolors(color).blue;
	}

	if ((pixelx < 0) || (pixelx >= 160))
	{
	    return;
	}

	gbRGB gbcolor = {red, green, blue};
	setpixel(pixelx, ly, gbcolor);

	if (pixelx == 159)
	{
	    updateframebuffer();
	}
    }

    void GPU::renderdmgbgpixel()
    {
	uint8_t scy = (ly + scrolly);
	uint8_t scx = (pixelx + scrollx);
	int tx = (scx & 7);

	if (tx == 0 || pixelx == 0)
	{
	    bgdata = readtiledmg(testbit(lcdc, 3), scx, scy);
	}

	bgpal = getdmgcolornum(bgdata, tx);
	bgcolor = getdmgcolor(bgpal, bgpalette);
    }

    void GPU::rendercgbbgpixel()
    {
	uint8_t scy = (ly + scrolly);
	uint8_t scx = (pixelx + scrollx);
	int tx = (scx & 7);

	if (tx == 0 || pixelx == 0)
	{
	    bgdata = readtilecgbbg(testbit(lcdc, 3), scx, scy);
	}

	if (testbit(bgattr, 5))
	{
	    tx = (7 - tx);
	}

	bgpal = getdmgcolornum(bgdata, tx);

	if (gpumem.isdmgmode() && !gpumem.biosload)
	{
	    bgcolor = getdmgcolor(bgpal, bgpalette);
	}
	else
	{
	    bgcolor = getgbccolor((bgattr & 0x07), bgpal, true);
	}

	bgprior = testbit(bgattr, 7);
    }

    void GPU::renderdmgwinpixel()
    {
	if (ly < windowy)
	{
	    return;
	}

	if ((pixelx + 7) < windowx)
	{
	    return;
	}

	if ((pixelx + 7) == windowx)
	{
	    windowlinecounter += 1;
	}

	uint8_t sy = (windowlinecounter - 1);
	uint8_t sx = (pixelx + 7 - windowx);
	int tx = (sx & 7);

	if (tx == 0 || pixelx == 0)
	{
	    windata = readtiledmg(testbit(lcdc, 6), sx, sy);
	}

	bgpal = getdmgcolornum(windata, tx);
	bgcolor = getdmgcolor(bgpal, bgpalette);
    }

    void GPU::rendercgbwinpixel()
    {
	if (ly < windowy)
	{
	    return;
	}

	if ((pixelx + 7) < windowx)
	{
	    return;
	}

	if ((pixelx + 7) == windowx)
	{
	    windowlinecounter += 1;
	}

	

	uint8_t sy = (windowlinecounter - 1);
	uint8_t sx = (pixelx + 7 - windowx);
	int tx = (sx & 7);

	if (tx == 0 || pixelx == 0)
	{
	    windata = readtilecgbwin(testbit(lcdc, 6), sx, sy);
	}

	if (testbit(winattr, 5))
	{
	    tx = (7 - tx);
	}

	bgpal = getdmgcolornum(windata, tx);

	if (gpumem.isdmgmode() && !gpumem.biosload)
	{
	    bgcolor = getdmgcolor(bgpal, bgpalette);
	}
	else
	{
	    bgcolor = getgbccolor((winattr & 0x07), bgpal, true);
	}

	bgprior = testbit(winattr, 7);
    }

    void GPU::renderdmgobjpixel()
    {
	int height = (testbit(lcdc, 2) ? 16 : 8);

	objpal = 0;
	objcolor = 0;
	
	for (int i = (sprites - 1); i >= 0; i--)
	{
	    Sprites& obj = sprite[i];

	    int tx = (pixelx - (obj.x - 8));
	    uint8_t ty = (ly - (obj.y - 16));

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

	    objpal = temp;

	    uint8_t palette = (obj.palette) ? objpalette1 : objpalette0;
	    objcolor = getdmgcolor(objpal, palette);
	    objprior = obj.priority;
	}
    }

    void GPU::rendercgbobjpixel()
    {
	int height = (testbit(lcdc, 2) ? 16 : 8);

	objpal = 0;
	objcolor = 0;
	
	for (int i = (sprites - 1); i >= 0; i--)
	{
	    Sprites& obj = sprite[i];

	    int tx = (pixelx - (obj.x - 8));
	    uint8_t ty = (ly - (obj.y - 16));

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

	    objpal = temp;

	    if (gpumem.isdmgmode() && !gpumem.biosload)
	    {
		uint8_t palette = (obj.palette) ? objpalette1 : objpalette0;
		objcolor = getdmgcolor(objpal, palette);
	        objdmgpalette = obj.palette;
	    }
	    else
	    {
	        objcolor = getgbccolor(obj.cgbpalette, objpal, false);
	    }

	    objprior = !obj.priority;
	}
    }

    void GPU::renderscanline()
    {
	if (gpumem.isdmgconsole())
	{
	    if (isbgenabled())
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
	    if (isbgenabled() && iswinenabled())
	    {
		if (currentscanline >= windowy)
		{
		    renderwin();
		}
	    }
	}
	else
	{
	    if (iswinenabled())
	    {
		if (currentscanline >= windowy)
		{
		    renderwin();
		}
	    }
	}

	if (isobjenabled())
	{
	    renderobj();
	}
    }

    void GPU::renderbg()
    {	
	uint8_t scy = scrolly;
	uint8_t scx = scrollx;
	uint16_t tilemap = testbit(lcdc, 3) ? 0x9C00 : 0x9800;
	uint16_t tiledata = testbit(lcdc, 4) ? 0x8000 : 0x8800;
	bool unsig = testbit(lcdc, 4);

	uint8_t ypos = 0;

	ypos = scy + ly;

	uint16_t tilerow = (((uint8_t)(ypos / 8)) * 32);

	for (int pixel = 0; pixel < 160; pixel++)
	{
	    uint8_t xpos = (pixel + scx);	    

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

	    if (gpumem.isgbcconsole() && !gpumem.isdmgmode())
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

	    if (gpumem.isgbcconsole() && !gpumem.isdmgmode())
	    {
		banknum = testbit(mapattrib, 3) ? 0x6000 : 0x8000;

		line = testbit(mapattrib, 6) ? (7 - (ypos % 8)) : (ypos % 8);
	    }

	    line *= 2;
	    uint8_t data1 = gpumem.vram[(tileloc + line) - banknum];
	    uint8_t data2 = gpumem.vram[(tileloc + line + 1) - banknum];

	    if (gpumem.isgbcconsole() && !gpumem.isdmgmode())
	    {
		xpos = testbit(mapattrib, 5) ? (7 - xpos) : xpos;
		bgpriorline[pixel] = testbit(mapattrib, 7);
	    }

	    int colorbit = (xpos % 8);
	    colorbit -= 7;
	    colorbit *= -1;

	    int colornum = BitGetVal(data2, colorbit);
	    colornum <<= 1;
	    colornum |= BitGetVal(data1, colorbit);

	    uint8_t red = 0;
	    uint8_t green = 0;
	    uint8_t blue = 0;


	    if (gpumem.isdmgconsole())
	    {
	        int color = getdmgcolor(colornum, bgpalette);

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
		int color = getdmgcolor(colornum, bgpalette);

		red = getdmgpalette(color, 0, true).red;
		green = getdmgpalette(color, 0, true).green;
		blue = getdmgpalette(color, 0, true).blue;
	    }
	    else
	    {
		int color = getgbccolor((mapattrib & 0x7), colornum, true);

		red = getcolors(color).red;
	 	green = getcolors(color).green;
		blue = getcolors(color).blue;
	    }

	    uint8_t scanline = ly;

	    if ((scanline < 0) || (scanline > 144))
	    {
		continue;
	    }

	    bgscanline[pixel] = colornum;

	    gbRGB gbcolor = {red, green, blue};
	    setpixel(pixel, scanline, gbcolor);
	}
    }

    void GPU::renderwin()
    {
	uint8_t winy = windowy;
	uint8_t winx = (windowx);

	if (winx <= 7)
	{
	    winx -= winx;
	}
	else
	{
	    winx -= 7;
	}

	if (winx >= 160 || winy > currentscanline)
	{
	    return;
	}


	bool unsig = testbit(lcdc, 4);
	uint16_t tiledata = (unsig) ? 0x8000 : 0x8800;
	uint16_t bgmem = testbit(lcdc, 6) ? 0x9C00 : 0x9800;


	int ypos = windowlinecounter;
	windowlinecounter++;

	uint16_t tilerow = (((ypos / 8)) * 32);

	for (int pixel = 0; pixel < 160; pixel++)
	{
	    if (pixel >= winx)
	    {
		uint8_t xpos = (pixel - winx);

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
		    banknum = testbit(mapattrib, 3) ? 0x6000 : 0x8000;
		    line = testbit(mapattrib, 6) ? (7 - (ypos % 8)) : (ypos % 8);
		}


	        line *= 2;
	        uint8_t data1 = gpumem.vram[(tileloc + line) - banknum];
	        uint8_t data2 = gpumem.vram[(tileloc + line + 1) - banknum];

		if (gpumem.isgbcconsole() && !gpumem.isdmgmode() && !gpumem.biosload)
		{
		    xpos = testbit(mapattrib, 5) ? (7 - xpos) : xpos;
		}

	        int colorbit = (xpos % 8);
	        colorbit -= 7;
	        colorbit *= -1;

	        int colornum = BitGetVal(data2, colorbit);
	        colornum <<= 1;
	        colornum |= BitGetVal(data1, colorbit);

	        uint8_t red = 0;
	        uint8_t green = 0;
	        uint8_t blue = 0;

		if (gpumem.isdmgconsole())
		{
	            int color = getdmgcolor(colornum, bgpalette);

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
		    int color = getdmgcolor(colornum, bgpalette);

		    red = getdmgpalette(color, 0, true).red;
		    green = getdmgpalette(color, 0, true).green;
		    blue = getdmgpalette(color, 0, true).blue;
		}
		else
		{
		    int color = getgbccolor((mapattrib & 0x7), colornum, true);

		    red = getcolors(color).red;
	 	    green = getcolors(color).green;
		    blue = getcolors(color).blue;
		}

	        uint8_t scanline = ly;


		if ((scanline < 0) || (scanline > 144))
		{
		    continue;
		}

	        bgscanline[pixel] = colornum;


		gbRGB gbcolor = {red, green, blue};
		setpixel(pixel, scanline, gbcolor);
	    }
	}
    }

    void GPU::renderobj()
    {
	uint16_t spritedata = 0x8000;
	int ysize = testbit(lcdc, 2) ? 16 : 8;
	int spritelimit = 40;

	uint8_t scanline = currentscanline;

	for (int i = (spritelimit - 1); i >= 0; i--)
	{
	    uint8_t index = (i * 4);
	    uint8_t ypos = gpumem.readDirectly(0xFE00 + index) - 16;
	    uint8_t xpos = gpumem.readDirectly(0xFE00 + index + 1) - 8;
	    uint8_t patternnum = gpumem.readDirectly(0xFE00 + index + 2);
	    uint8_t flags = gpumem.readDirectly(0xFE00 + index + 3);

	    bool priority = testbit(flags, 7);
	    bool yflip = testbit(flags, 6);
	    bool xflip = testbit(flags, 5);

	    int bank = 0;

	    if (gpumem.isgbcconsole() && !gpumem.isdmgmode())
	    {
		bank = ((flags & 0x08) >> 3);
	    }

	    uint8_t line = (yflip) ? ((((scanline - ypos - ysize) + 1) * -1)) : ((scanline - ypos));


	    if (testbit(lcdc, 2))
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
		    uint8_t coloraddr = testbit(flags, 4) ? objpalette1 : objpalette0;

		    uint8_t red = 0;
		    uint8_t green = 0;
		    uint8_t blue = 0;

		   
		    if (gpumem.isdmgconsole())
		    {
	        	int color = getdmgcolor(colornum, coloraddr);

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
			int color = getdmgcolor(colornum, coloraddr);

			uint16_t tempoffs = testbit(flags, 4) ? 0xFF49 : 0xFF48;

			int coloroffset = (BitGetVal(tempoffs, 0) << 3);
			
			red = getdmgpalette(color, coloroffset, false).red;
			green = getdmgpalette(color, coloroffset, false).green;
			blue = getdmgpalette(color, coloroffset, false).blue;
		    }
		    else
		    {
			int color = getgbccolor((flags & 0x7), colornum, false);
			red = getcolors(color).red;
	 		green = getcolors(color).green;
			blue = getcolors(color).blue;
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
		    
		    if (gpumem.isgbcmode() && testbit(lcdc, 0) && (bgpriorline[xpixel] && !iswhite))
		    {
			continue;
		    }
		    

		    gbRGB gbcolor = {red, green, blue};
		    setpixel(xpixel, scanline, gbcolor);
		}
	    }
	}
    }
};
