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
	cout << "GPU::Shutdown" << endl;
    }

    void GPU::reset()
    {
	clearscreen();

	cout << "GPU::Initialized" << endl;
    }

    void GPU::clearscreen()
    {
	for (int i = 0; i < 160 * 144; i++)
	{
	    framebuffer[i][0] = 0;
	    framebuffer[i][1] = 0;
	    framebuffer[i][2] = 0;
	}
    }

    void GPU::updategraphics(int cycles)
    {
	setlcdstatus();
	
	if (TestBit(gmem->readByte(0xFF40), 7))
	{
	    scanlinecounter -= cycles;
	}
	else
	{
	    return;
	}

	if (scanlinecounter <= 0)
	{
	    gmem->memorymap[0xFF44]++;
	    uint8_t currentline = gmem->readByte(0xFF44);

	    scanlinecounter = 456;

	    if (currentline == 144)
	    {
		gcpu->requestinterrupt(0);
	    }
	    else if (currentline > 153)
	    {
		gmem->memorymap[0xFF44] = 0;
	    }
	    else if (currentline < 144)
	    {
		drawscanline();
	    }
	}
    }

    void GPU::setlcdstatus()
    {
	uint8_t status = gmem->readByte(0xFF41);
	if (TestBit(gmem->readByte(0xFF40), 7) == false)
	{
	    scanlinecounter = 456;
	    gmem->memorymap[0xFF44] = 0;
	    status &= 252;
	    BitSet(status, 0);
	    gmem->writeByte(0xFF41, status);
	    return;
	}

	uint8_t currentline = gmem->readByte(0xFF44);
	uint8_t currentmode = status & 0x3;

	uint8_t mode = 0;
	bool reqint = false;

	if (currentline >= 144)
	{
	    mode = 1;
	    BitSet(status, 0);
	    BitReset(status, 1);
	    reqint = TestBit(status, 4);
	}
	else
	{
	    int mode2bounds = 456 - 80;
	    int mode3bounds = mode2bounds - 172;

	    if (scanlinecounter >= mode2bounds)
	    {
		mode = 2;
		BitSet(status, 1);
		BitReset(status, 0);
		reqint = TestBit(status, 5);
	    }
	    else if (scanlinecounter >= mode3bounds)
	    {
		mode = 3;
		BitSet(status, 1);
		BitSet(status, 0);
	    }
	    else
	    {
		mode = 0;
		BitReset(status, 1);
		BitReset(status, 0);
		reqint = TestBit(status, 3);
	    }
	}

	if (reqint && (mode != currentmode))
	{
	    gcpu->requestinterrupt(1);
	}

	if (gmem->readByte(0xFF44) == gmem->readByte(0xFF45))
	{
	    BitSet(status, 2);
	    if (TestBit(status, 6))
	    {
		gcpu->requestinterrupt(1);
	    }
	}
	else
	{
	    BitReset(status, 2);
	}
	
	gmem->writeByte(0xFF41, status);
    }

    void GPU::drawscanline()
    {
	uint8_t control = gmem->readByte(0xFF40);
	if (TestBit(control, 0))
	{
	    rendertiles(control);
	}
    }

    void GPU::rendertiles(uint8_t lcdcontrol)
    {
	bool unsig = true;
	uint16_t tiledata = 0;
	uint16_t bgmem = 0;
	uint8_t scrollY = gmem->readByte(0xFF42);
	uint8_t scrollX = gmem->readByte(0xFF43);
	uint8_t windowY = gmem->readByte(0xFF4A);
	uint8_t windowX = gmem->readByte(0xFF4B) - 7;

	bool usingwindow = false;

	if (TestBit(lcdcontrol, 5))
	{
	    if (windowY <= gmem->readByte(0xFF44))
	    {
		usingwindow = true;
	    }
	}

	if (TestBit(lcdcontrol, 4))
	{
	    tiledata = 0x8000;
	}
	else
	{
	    tiledata = 0x8800;
	    unsig = false;
	}

	if (usingwindow == false)
	{
	    if (TestBit(lcdcontrol, 3))
	    {
		bgmem = 0x9C00;
	    }
	    else
	    {
		bgmem = 0x9800;
	    }
	}
	else
	{
	    if (TestBit(lcdcontrol, 6))
	    {
		bgmem = 0x9C00;
	    }
	    else
	    {
		bgmem = 0x9800;
	    }
	}

	uint8_t ypos = 0;
	
	if (!usingwindow)
	{
	    ypos = scrollY + gmem->readByte(0xFF44);
	}
	else
	{
	    ypos = gmem->readByte(0xFF44) - windowY;
	}

	uint16_t tilerow = (((uint8_t)(ypos / 8)) * 32);

	for (int pixel = 0; pixel < 160; pixel++)
	{
	    uint8_t xpos = pixel + scrollX;

	    if (usingwindow)
	    {
		if (pixel >= windowX)
		{
		    xpos = pixel - windowX;
		}
	    }

	    uint16_t tilecol = (xpos / 8);
	    int16_t tilenum;

	    uint16_t tileaddr = bgmem + tilerow + tilecol;
	    if (unsig)
	    {
		tilenum = (uint8_t)gmem->readByte(tileaddr);
	    }
	    else
	    {
		tilenum = (int8_t)gmem->readByte(tileaddr);
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

	    uint8_t line = ypos % 8;
	    line *= 2;
	    uint8_t data1 = gmem->readByte(tileloc + line);
	    uint8_t data2 = gmem->readByte(tileloc + line + 1);

	    uint8_t reqbit = 7 - (xpos % 8);
	    uint8_t bit1 = (data1 >> reqbit) & 1;
	    uint8_t bit2 = (data2 >> reqbit) & 1;

	    uint8_t colorid = (bit1 << 1) | bit2;
	    int color = getcolor(colorid, 0xFF47);

	    int red, green, blue;
	    
	    switch (color)
	    {
		case 0: red = 0xFF; green = 0xFF; blue = 0xFF; break;
		case 1: red = 0xCC; green = 0xCC; blue = 0xCC; break;
		case 2: red = 0x77; green = 0x77; blue = 0x77; break;
		default: red = 0; green = 0; blue = 0; break;
	    }

	    int finaly = gmem->readByte(0xFF44);

	    if ((finaly < 0) || (finaly > 143) || (pixel < 0) || (pixel > 159))
	    {
		continue;
	    }

	    int index = pixel + (finaly * 160);
	    framebuffer[index][0] = red;
	    framebuffer[index][1] = green;
	    framebuffer[index][2] = blue;
	    
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
