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
	    framebuffer[i][0] = 0;
	    framebuffer[i][1] = 0;
	    framebuffer[i][2] = 0;
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

	uint8_t status = gmem->memorymap[0xFF41];

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

    }

    void GPU::setlcdstatus()
    {
	uint8_t status = gmem->memorymap[0xFF41];
	uint8_t mode = 0;

	if (!TestBit(gmem->memorymap[0xFF40], 7))
	{
	    mode = 1;
	    status = BitSet(status, 0);
	    status = BitReset(status, 1);
	    status |= 0x80;
	    status &= 0xF8;
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
    }

    void GPU::drawscanline()
    {
	uint8_t control = gmem->memorymap[0xFF40];
	rendertiles(control);
	rendersprites(control);
    }

    void GPU::rendertiles(uint8_t lcdcontrol)
    {	
	if (!TestBit(lcdcontrol, 0))
	{
	    return;
	}	

	uint8_t scrollY = gmem->readByte(0xFF42);
	uint8_t scrollX = gmem->readByte(0xFF43);
	uint8_t windowY = gmem->readByte(0xFF4A);
	uint8_t windowX = gmem->readByte(0xFF4B);
	uint16_t tiledata = TestBit(lcdcontrol, 4) ? 0x8000 : 0x8800;
	uint16_t bgmem = TestBit(lcdcontrol, 3) ? 0x9C00 : 0x9800;
	uint16_t bgmemaddr = bgmem;
	uint16_t winmemaddr = TestBit(lcdcontrol, 6) ? 0x9C00 : 0x9800;
	bool unsig = TestBit(lcdcontrol, 4);
	bool windowenabled = TestBit(lcdcontrol, 5);

	uint8_t scanline = gmem->memorymap[0xFF44];

	for (uint8_t x = 0; x < 160; x++)
	{
	    uint8_t ypos = (scrollY + scanline);
	    uint8_t xpos = (scrollX + x);

	    if (windowenabled && (scanline >= windowY) && (x >= (windowX - 7)))
	    {
		bgmem = winmemaddr;
		ypos = (scanline - windowY);
		xpos = ((windowX - 7) + x);
	    }
	    else
	    {
		bgmem = bgmemaddr;
	    }

	    uint16_t tilecol = (xpos / 8);
	    uint16_t tilerow = ((ypos / 8) * 32);
	    uint8_t tileyline = ((ypos % 8) * 2);
	    uint16_t tileaddr = (bgmem + tilecol + tilerow);
	    int16_t tilenum = (unsig) ? (uint8_t)gmem->readByte(tileaddr) : (int8_t)gmem->readByte(tileaddr);
	    uint16_t tileloc = (unsig) ? (tiledata + (tilenum * 16)) : ((tiledata + ((tilenum + 128) * 16)));
	    uint8_t pixeldata1 = gmem->readByte(tileloc + tileyline);
	    uint8_t pixeldata2 = gmem->readByte(tileloc + tileyline + 1);
	    int colorbit = (((xpos % 8) - 7) * -1);
	    int colornum = ((BitGetVal(pixeldata2, colorbit) << 1) | (BitGetVal(pixeldata1, colorbit)));
	    int color = getcolor(colornum, 0xFF47);
	    int red = 0;
	    int blue = 0;
	    int green = 0;

	    switch (color)
	    {
		case 0: red = 0xFF; green = 0xFF; blue = 0xFF; break;
		case 1: red = 0xCC; green = 0xCC; blue = 0xCC; break;
		case 2: red = 0x77; green = 0x77; blue = 0x77; break;
		case 3: red = 0x00; green = 0x00; blue = 0x00; break;
	    }

	    int index = (x + (scanline * 160));
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
	bool use8by16 = TestBit(lcdcontrol, 2) ? true : false;
	uint8_t spritelimit = 40;

	for (int i = 0; i < spritelimit; i++)
	{
	    uint8_t index = (i * 4);
	    uint8_t ypos = gmem->readByte(spriteattribdata + index) - 16;
	    uint8_t xpos = gmem->readByte(spriteattribdata + index + 1) - 8;
	    uint8_t patternnum = gmem->readByte(spriteattribdata + index + 2);
	    uint8_t flags = gmem->readByte(spriteattribdata + index + 3);

	    bool yflip = TestBit(flags, 6);
	    bool xflip = TestBit(flags, 5);

	    uint8_t scanline = gmem->readByte(0xFF44);

	    int ysize = 8;
	    if (use8by16)
	    {
		ysize = 16;
	    }

	    if ((scanline >= ypos) && (scanline < (ypos + ysize)))
	    {
		int line = scanline - ypos;

		if (yflip)
		{
		    line -= ysize;
		    line *= -1;
		}

		line *= 2;
		uint16_t dataaddr = (spritedata + (patternnum * 16)) + line;
		uint8_t pixeldata1 = gmem->readByte(dataaddr);
		uint8_t pixeldata2 = gmem->readByte(dataaddr + 1);

		for (int tilepixel = 7; tilepixel >= 0; tilepixel--)
		{
		    int colorbit = tilepixel;

		    if (xflip)
		    {
			colorbit -= 7;
			colorbit *= -1;
		    }

		    int colornum = ((BitGetVal(pixeldata2, colorbit) << 1) | (BitGetVal(pixeldata1, colorbit)));

		    uint16_t coloraddr = TestBit(flags, 4) ? 0xFF49 : 0xFF48;
		    int color = getcolor(colornum, coloraddr);

		    if (color == 0)
		    {
			continue;
		    }

		    int red = 0;
		    int blue = 0;
		    int green = 0;

		    switch (color)
		    {
			case 0: red = 0xFF; green = 0xFF; blue = 0xFF; break;
			case 1: red = 0xCC; green = 0xCC; blue = 0xCC; break;
			case 2: red = 0x77; green = 0x77; blue = 0x77; break;
			case 3: red = 0x00; green = 0x00; blue = 0x00; break;
		    }

		    int xpix = 0 - tilepixel;
		    xpix += 7;

		    int pixel = xpos + xpix;

		    if ((scanline < 0) || (scanline > 143) || (pixel < 0) || (pixel > 159))
		    {
			continue;
		    }

		    int index = (pixel + (scanline * 160));
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
