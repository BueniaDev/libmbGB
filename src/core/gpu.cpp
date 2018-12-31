#include "../../include/libmbGB/gpu.h"
#include <iostream>
using namespace gb;
using namespace std;

namespace gb
{
    GPU::GPU()
    {
	reset();
    }

    GPU::~GPU()
    {
	cout << "GPU::Shutting down..." << endl;
    }

    void GPU::reset()
    {
	scanlinecounter = 0;	
	clearscreen();

	cout << "GPU::Initialized" << endl;
    }

    void GPU::clearscreen()
    {
	for (int i = 0; i < (160 * 144); i++)
	{
	    framebuffer[i][0] = 255;
	    framebuffer[i][1] = 255;
	    framebuffer[i][2] = 255;
	}
    }

    void GPU::updategraphics(int cycles)
    {
	setlcdstatus();
	
	if (!TestBit(gmem->memorymap[0xFF40], 7))
	{
	    return;
	}

	scanlinecounter += cycles;

	uint8_t status = gmem->readByte(0xFF41);

	if (scanlinecounter >= 456)
	{
	    uint8_t scanline = gmem->memorymap[0xFF44];

	    if (scanline == 144)
	    {
		uint8_t req = gmem->readByte(0xFF0F);
		req = BitSet(req, 0);
		gmem->writeByte(0xFF0F, req);
	    }
	    else if (scanline == 154)
	    {
		gmem->memorymap[0xFF44] = 0xFF;
	    }
	    else if (scanline < 144)
	    {
		drawscanline();
	    }

	    gmem->memorymap[0xFF44] += 1;

	    if (scanline == gmem->readByte(0xFF45))
	    {
		status = BitSet(status, 2);
	    }
	    else
	    {
		status = BitReset(status, 2);
	    }

	    if (TestBit(status, 2) && TestBit(status, 6))
	    {
		uint8_t req = gmem->readByte(0xFF0F);
		req = BitSet(req, 1);
		gmem->writeByte(0xFF0F, req);
	    }
	
	    status |= 0x80;
	    scanlinecounter -= 456;
	}

	gmem->writeByte(0xFF41, status);
    }

    void GPU::setlcdstatus()
    {
	uint8_t status = gmem->readByte(0xFF41);
	uint8_t mode = 0;

	if (!TestBit(gmem->memorymap[0xFF40], 7))
	{
	    mode = 1;
	    status = BitSet(status, 0);
	    status = BitReset(status, 1);
	    status |= 0x80;
	    status &= 0xF8;
	    gmem->writeByte(0xFF41, status);
	    gmem->memorymap[0xFF44] = 0;
	    scanlinecounter = 0;
	    return;
	}

	uint8_t scanline = gmem->memorymap[0xFF44];
	uint8_t currentmode = (status & 0x3);

	bool reqint = false;

	if (scanline >= 144)
	{
	    mode = 1;
	    status = BitSet(status, 0);
	    status = BitReset(status, 1);
	    status |= 0x80;
	    reqint = TestBit(status, 4);
	}
	else
	{
	    int mode2bounds = (456 - 80);
	    int mode3bounds = (mode2bounds - 172);
	    if ((scanlinecounter >= mode2bounds) && (scanlinecounter <= 456))
	    {
		mode = 2;
		status = BitReset(status, 0);
		status = BitSet(status, 1);
		status |= 0x80;
		reqint = TestBit(status, 5);
	    }
	    else if ((scanlinecounter >= mode3bounds) && (scanlinecounter < mode2bounds))
	    {
		mode = 3;
		status = BitSet(status, 0);
		status = BitSet(status, 1);
		status |= 0x80;
	    }
	    else
	    {
		mode = 0;
		status = BitReset(status, 0);
		status = BitReset(status, 1);
		status |= 0x80;
		reqint = TestBit(status, 3);
	    }
	}

	if (reqint && (mode != currentmode))
	{
	    uint8_t req = gmem->readByte(0xFF0F);
	    req = BitSet(req, 1);
	    gmem->writeByte(0xFF0F, req);
	}

	gmem->writeByte(0xFF41, status);
    }

    void GPU::drawscanline()
    {
	uint8_t control = gmem->memorymap[0xFF40];
	rendertiles(control);
	renderwindow(control);
	rendersprites(control);
    }

    void GPU::rendertiles(uint8_t lcdcontrol)
    {
	if (!TestBit(lcdcontrol, 0)) // Is the BG disabled?
	{
	    uint8_t scanline = gmem->memorymap[0xFF44];

	    // If so, then fill the screen with white
	    for (int i = 0; i < 160; i++)
	    {
            int index = (i + (scanline * 160));
            framebuffer[index][0] = 255;
            framebuffer[index][1] = 255;
            framebuffer[index][2] = 255;
	    }
        return;
	}

	uint8_t scrollY = gmem->readByte(0xFF42);
	uint8_t scrollX = gmem->readByte(0xFF43);
	uint16_t tilemap = TestBit(lcdcontrol, 3) ? 0x9C00 : 0x9800;
	uint16_t tiledata = TestBit(lcdcontrol, 4) ? 0x8000 : 0x8800;
	bool unsig = TestBit(lcdcontrol, 4);

	uint8_t ypos = 0;
	
	// Which of the 32 vertical tiles is the current scanline drawing to?
	ypos = scrollY + gmem->memorymap[0xFF44]; 

	// Which of the current tile's 8 vertical pixels is the scanline on?
	uint16_t tilerow = (((uint8_t)(ypos / 8)) * 32);

	// Start drawing the 160 horizontal pixels for this scanline
	for (int pixel = 0; pixel < 160; pixel++)
	{
	    uint8_t xpos = pixel + scrollX;

	    // Which of the horizontal tiles does the x-position fall within?
	    uint16_t tilecol = (xpos / 8);
	    int16_t tilenum = 0;

	    // Get the tile identity numbers
	    uint16_t tileaddr = tilemap + tilerow + tilecol;

	    if (unsig) // Is the value signed or unsigned?
	    {
		tilenum = (uint8_t)gmem->readByte(tileaddr); // Unsigned
	    }
	    else
	    {
		tilenum = (int8_t)gmem->readByte(tileaddr); // Signed
	    }

	    uint16_t tileloc = tiledata;

	    // Determine where the tile identifier is in memory
	    if (unsig)
	    {
		tileloc += (tilenum * 16); // Unsigned
	    }
	    else
	    {
		tileloc += ((tilenum + 128) * 16); // Signed
	    }

	    // Find the correct vertical line we're on of this tile
	    uint8_t line = (ypos % 8);
	    line *= 2; // Each vertical line takes up 2 bytes
	    uint8_t data1 = gmem->readByte(tileloc + line);
	    uint8_t data2 = gmem->readByte(tileloc + line + 1);

	    // Get color bit
	    int colorbit = xpos % 8;
	    colorbit -= 7;
	    colorbit *= -1;

	    // Get color id
	    int colornum = BitGetVal(data2, colorbit);
	    colornum <<= 1;
	    colornum |= BitGetVal(data1, colorbit);

	    // Get actual color from palette
	    int color = getcolor(colornum, 0xFF47);
	    int red = 0;
	    int green = 0;
	    int blue = 0;

	    // Setup RGB values
	    switch (color)
	    {
		case 0: red = 0xFF; green = 0xFF; blue = 0xFF; break;
		case 1: red = 0xCC; green = 0xCC; blue = 0xCC; break;
		case 2: red = 0x77; green = 0x77; blue = 0x77; break;
		case 3: red = 0x00; green = 0x00; blue = 0x00; break;
	    }

	    uint8_t scanline = gmem->memorymap[0xFF44];

	    // Set framebuffer values
	    int index = (pixel + (scanline * 160));
	    framebuffer[index][0] = red;
	    framebuffer[index][1] = green;
	    framebuffer[index][2] = blue;
	}
    }

    void GPU::renderwindow(uint8_t lcdcontrol)
    {
	if (!TestBit(lcdcontrol, 5)) // Is the window disabled?
	{
	    return; // If so, then return
	}

	uint8_t windowY = gmem->readByte(0xFF4A);
	uint8_t windowX = gmem->readByte(0xFF4B);
	windowX -= (windowX <= 0x07) ? windowX : 7;

	if (gmem->memorymap[0xFF44] < windowY)
	{
	    return;
	}

	uint16_t tilemap = TestBit(lcdcontrol, 6) ? 0x9C00 : 0x9800;
	uint16_t tiledata = TestBit(lcdcontrol, 4) ? 0x8000 : 0x8800;
	bool unsig = TestBit(lcdcontrol, 4);

	uint8_t ypos = gmem->memorymap[0xFF44] - windowY;

	// Which of the current tile's 8 vertical pixels is the scanline on?
	uint16_t tilerow = (((uint8_t)(ypos / 8)) * 32);

	// Start drawing the 160 horizontal pixels for this scanline
	for (int pixel = windowX; pixel < 160; pixel++)
	{
	    uint8_t xpos = 0;	    

	    if (pixel >= windowX)
	    {
		xpos = pixel - windowX;
	    }

	    // Which of the horizontal tiles does the x-position fall within?
	    uint16_t tilecol = (xpos / 8);
	    int16_t tilenum = 0;

	    // Get the tile identity numbers
	    uint16_t tileaddr = tilemap + tilerow + tilecol;

	    if (unsig) // Is the value signed or unsigned?
	    {
		tilenum = (uint8_t)gmem->readByte(tileaddr); // Unsigned
	    }
	    else
	    {
		tilenum = (int8_t)gmem->readByte(tileaddr); // Signed
	    }

	    uint16_t tileloc = tiledata;

	    // Determine where the tile identifier is in memory
	    if (unsig)
	    {
		tileloc += (tilenum * 16); // Unsigned
	    }
	    else
	    {
		tileloc += ((tilenum + 128) * 16); // Signed
	    }

	    // Find the correct vertical line we're on of this tile
	    uint8_t line = (ypos % 8);
	    line *= 2; // Each vertical line takes up 2 bytes
	    uint8_t data1 = gmem->readByte(tileloc + line);
	    uint8_t data2 = gmem->readByte(tileloc + line + 1);

	    // Get color bit
	    int colorbit = xpos % 8;
	    colorbit -= 7;
	    colorbit *= -1;

	    // Get color id
	    int colornum = BitGetVal(data2, colorbit);
	    colornum <<= 1;
	    colornum |= BitGetVal(data1, colorbit);

	    // Get actual color from palette
	    int color = getcolor(colornum, 0xFF47);
	    int red = 0;
	    int green = 0;
	    int blue = 0;

	    // Setup RGB values
	    switch (color)
	    {
		case 0: red = 0xFF; green = 0xFF; blue = 0xFF; break;
		case 1: red = 0xCC; green = 0xCC; blue = 0xCC; break;
		case 2: red = 0x77; green = 0x77; blue = 0x77; break;
		case 3: red = 0x00; green = 0x00; blue = 0x00; break;
	    }

	    uint8_t scanline = gmem->memorymap[0xFF44];

	    // Set framebuffer values
	    int index = (pixel + (scanline * 160));
	    framebuffer[index][0] = red;
	    framebuffer[index][1] = green;
	    framebuffer[index][2] = blue;
	}
    }

    void GPU::rendersprites(uint8_t lcdcontrol)
    {
	if (!TestBit(lcdcontrol, 1))
	{
	    return;
	}

	uint16_t spritedata = 0x8000;
	uint16_t spriteattribdata = 0xFE00;
	int ysize = TestBit(lcdcontrol, 2) ? 16 : 8;
	uint8_t spritelimit = 40;

	uint8_t scanline = gmem->memorymap[0xFF44];

	for (int i = (spritelimit - 1); i >= 0; i--)
	{
	    uint8_t index = (i * 4);
	    uint8_t ypos = gmem->readByte(spriteattribdata + index) - 16;
	    uint8_t xpos = gmem->readByte(spriteattribdata + index + 1) - 8;
	    uint8_t patternnum = gmem->readByte(spriteattribdata + index + 2);
	    uint8_t flags = gmem->readByte(spriteattribdata + index + 3);

	    bool priority = TestBit(flags, 7);
	    bool yflip = TestBit(flags, 6);
	    bool xflip = TestBit(flags, 5);

	    uint8_t line = (yflip) ? ((((scanline - ypos - ysize) + 1) * -1) * 2) : ((scanline - ypos) * 2);
	    uint8_t data1 = gmem->readByte(spritedata + (patternnum * 16) + line);
	    uint8_t data2 = gmem->readByte(spritedata + (patternnum * 16) + line + 1);

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
		    bool iswhite = (framebuffer[xpixel + (scanline * 160)][0] == 255);
		    int colornum = BitGetVal(data2, spritepixel);
		    colornum <<= 1;
		    colornum |= BitGetVal(data1, spritepixel);
		    uint16_t coloraddr = TestBit(flags, 4) ? 0xFF49 : 0xFF48;

		    int color = getcolor(colornum, coloraddr);

		    int red = 0;
		    int green = 0;
		    int blue = 0;

	    	    // Setup RGB values
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

		    if (priority == true && !iswhite)
		    {
			continue;
		    }

		    int index = (xpixel + (scanline * 160));
		    framebuffer[index][0] = red;
		    framebuffer[index][1] = green;
		    framebuffer[index][2] = blue;
		}
	    }
	}
    }

    int GPU::getcolor(int id, uint16_t palette)
    {
	uint8_t data = gmem->readByte(palette);
	int hi = 2 * id + 1;
	int lo = 2 * id;
	int bit1 = (data >> hi) & 1;
	int bit0 = (data >> lo) & 1;
	return (bit1 << 1) | bit0;
    }
}
