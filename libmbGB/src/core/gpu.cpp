#include "../../include/libmbGB/gpu.h"
#include <iostream>
#include <math.h>
using namespace gb;
using namespace std;

namespace gb
{
    GPU::GPU()
    {
	
    }

    GPU::~GPU()
    {
	cout << "GPU::Shutting down..." << endl;
    }

    void GPU::reset()
    {
	scanlinecounter = 0;
    windowlinecounter = 0;
	clearscreen();

	cout << "GPU::Initialized" << endl;
    }

    void GPU::clearscreen()
    {
	for (int i = 0; i < (160 * 144); i++)
	{
	    framebuffer[i].red = 255;
	    framebuffer[i].green = 255;
	    framebuffer[i].blue = 255;
	}
    }

    void GPU::updategraphics(int cycles)
    {
        uint8_t stat = gmem->memorymap[0xFF41];
        uint8_t mode = stat & 0x3;
	
        scanlinecounter += cycles;	
        
        if (!TestBit(gmem->memorymap[0xFF40], 7))
        {
            scanlinecounter = 0;
            gmem->memorymap[0xFF44] = 0;
            windowlinecounter = 0;
            mode = 0;
            stat &= 0xFC;
            gmem->writeDirectly(0xFF41, stat);
            return;
        }
        
        switch (mode)
        {
            case 0:
            {
                if (scanlinecounter >= 204)
                {
                    scanlinecounter = 0;
                    gmem->memorymap[0xFF44] += 1;
                    
                    checklyc();
                    
                    if (gmem->memorymap[0xFF44] == 144)
                    {
                        mode = 1;
                        if (TestBit(stat, 4))
                        {
                            uint8_t req = gmem->readByte(0xFF0F);
                            req = BitSet(req, 1);
                            gmem->writeByte(0xFF0F, req);
                        }
                        
                        uint8_t req = gmem->readByte(0xFF0F);
                        req = BitSet(req, 0);
                        gmem->writeByte(0xFF0F, req);
                    }
                    else
                    {
                        mode = 2;
                        if (TestBit(stat, 5))
                        {
                            uint8_t req = gmem->readByte(0xFF0F);
                            req = BitSet(req, 1);
                            gmem->writeByte(0xFF0F, req);
                        }
                    }
                }
            }
            break;
            case 1:
            {
                if (scanlinecounter >= 456)
                {
                    scanlinecounter = 0;
                    gmem->memorymap[0xFF44]++;
                    
                    checklyc();
                    
                    if (gmem->memorymap[0xFF44] == 153)
                    {
                        gmem->memorymap[0xFF44] = 0;
                        windowlinecounter = 0;
                        checklyc();
                    }
                    else if (gmem->memorymap[0xFF44] == 1)
                    {
                        mode = 2;
                        if (TestBit(stat, 5))
                        {
                            uint8_t req = gmem->readByte(0xFF0F);
                            req = BitSet(req, 1);
                            gmem->writeByte(0xFF0F, req);
                        }
                        gmem->memorymap[0xFF44] = 0;
                    }
                }
            }
            break;
            case 2:
            {
                if (scanlinecounter >= 80)
                {
                    scanlinecounter = 0;
                    mode = 3;
                }
            }
            break;
            case 3:
            {
                if (scanlinecounter >= 172)
                {
                    scanlinecounter = 0;
                    mode = 0;
                    
                    if (TestBit(stat, 3))
                    {
                        uint8_t req = gmem->readByte(0xFF0F);
                        req = BitSet(req, 1);
                        gmem->writeByte(0xFF0F, req);
                    }

                    drawscanline();

		    if (gmem->gbtype == 2 && gmem->hdmaactive)
		    {
			if ((gmem->hdmalength & 0x7F) == 0)
			{
			    gmem->hdmaactive = false;
			}
			gmem->hdmatransfer();
			gmem->hdmalength -= 1;
		    }
                }
            }
            break;
            default: break;
        }
        
        stat &= 0xFC;
        stat |= mode;
        
        gmem->writeDirectly(0xFF41, stat);
    }
    
    void GPU::checklyc()
    {
        uint8_t stat = gmem->readDirectly(0xFF41);
        
        if (gmem->memorymap[0xFF44] == gmem->memorymap[0xFF45])
        {
            stat |= 0x4;
        }
        else
        {
            stat &= 0xFB;
        }
        
        if (TestBit(stat, 6) && TestBit(stat, 2))
        {
            uint8_t req = gmem->readByte(0xFF0F);
            req = BitSet(req, 1);
            gmem->writeByte(0xFF0F, req);
        }
        
        gmem->writeDirectly(0xFF41, stat);
    }

    bool GPU::dumpvram()
    {
        fstream file("vram.bin", ios::out | ios::binary);
        
        if (!file.is_open())
        {
            cout << "Error opening save state" << endl;
            return false;
        }

	file.write((char*)&gmem->vram[0], sizeof(gmem->vram));
	file.close();

	return true;
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
	    if (gmem->gbtype == 1)
	    {
	    	for (int i = 0; i < 160; i++)
	        {
                    int index = (i + (scanline * 160));
                    framebuffer[index].red = 255;
                    framebuffer[index].green = 255;
                    framebuffer[index].blue = 255;
	        }
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

	    uint8_t mapattrib = 0;

	    if (unsig) // Is the value signed or unsigned?
	    {
		tilenum = (uint8_t)gmem->vram[tileaddr - 0x8000]; // Unsigned
	    }
	    else
	    {
		tilenum = (int8_t)(gmem->vram[tileaddr - 0x8000]); // Signed
	    }

	
	    if (gmem->isgbcenabled)
	    {
		mapattrib = gmem->vram[tileaddr - 0x6000];
	    }

	    uint16_t tileloc = tiledata;

	    // Determine where the tile identifier is in memory
	    if (unsig)
	    {
		tileloc += (uint16_t)(tilenum * 16); // Unsigned
	    }
	    else
	    {
		tileloc += (int16_t)(((tilenum + 128) * 16)); // Signed
	    }

	    // Find the correct vertical line we're on of this tile

	    uint16_t banknum = 0x8000;

	    uint8_t line = (ypos % 8);

	    if (gmem->isgbcenabled)
	    {
		banknum = TestBit(mapattrib, 3) ? 0x6000 : 0x8000;

		line = TestBit(mapattrib, 6) ? (7 - (ypos % 8)) : (ypos % 8);
	    }

	    line *= 2; // Each vertical line takes up 2 bytes
	    uint8_t data1 = gmem->vram[(tileloc + line) - banknum];
	    uint8_t data2 = gmem->vram[(tileloc + line + 1) - banknum];

	    if (gmem->isgbcenabled)
	    {
		xpos = TestBit(mapattrib, 5) ? (7 - xpos) : (xpos);
	        bgprior[pixel] = TestBit(mapattrib, 7);
	    }

	    // Get color bit

	    int colorbit = (xpos % 8);
	    colorbit -= 7;
	    colorbit *= -1;

	    // Get color id
	    int colornum = BitGetVal(data2, colorbit);
	    colornum <<= 1;
	    colornum |= BitGetVal(data1, colorbit);

	    int red = 0;
	    int green = 0;
	    int blue = 0;

	    if (!gmem->isgbcenabled)
	    {
	    	// Get actual color from palette
	    	int color = getdmgcolor(colornum, 0xFF47);

	    	// Setup RGB values
	    	switch (color)
	    	{
		    case 0: red = 0xFF; green = 0xFF; blue = 0xFF; break;
		    case 1: red = 0xCC; green = 0xCC; blue = 0xCC; break;
		    case 2: red = 0x77; green = 0x77; blue = 0x77; break;
		    case 3: red = 0x00; green = 0x00; blue = 0x00; break;
	    	}
	    }
	    else
	    {
		int color = getgbccolor((mapattrib & 0x7), colornum);

	        bgscancolor[pixel] = color;

	    	red = ((color & 0x1F) << 3);
		red |= (red >> 5);
		green = (((color >> 5) & 0x1F) << 3);
		green |= (green >> 5);
		blue = (((color >> 10) & 0x1F) << 3);
		blue |= (blue >> 5);
	    }

	    uint8_t scanline = gmem->memorymap[0xFF44];

	    if ((scanline < 0) || (scanline > 144))
	    {
		continue;
	    }

	    bgscanline[pixel] = colornum;

	    // Set framebuffer values
	    int index = (pixel + (scanline * 160));
	    framebuffer[index].red = red;
	    framebuffer[index].green = green;
	    framebuffer[index].blue = blue;
	}
    }

    void GPU::renderwindow(uint8_t lcdcontrol)
    {
	if (!TestBit(lcdcontrol, 5))
	{
	    return;
	}

	uint8_t windowX = (gmem->readByte(0xFF4B) - 7);
	uint8_t windowY = gmem->readByte(0xFF4A);

	if (windowX >= 160 || windowY > gmem->memorymap[0xFF44])
	{
	    return;
	}

	int ypos = windowlinecounter;
	windowlinecounter++;

	for (int pixel = 0; pixel < 160; pixel++)
	{

	    if (pixel >= windowX)
	    {
	        int xpos = (pixel - windowX);		

		int tilenum = (xpos / 8) + ((ypos / 8) * 32);

		uint16_t maploc = 0;
		uint16_t tileloc = 0;
		uint8_t mapattrib = 0;

		if (TestBit(lcdcontrol, 6))
		{
		    maploc = 0x1C00 + tilenum;
		}
		else
		{
		    maploc = 0x1800 + tilenum;
		}

		tileloc = gmem->vram[maploc];

		if (gmem->isgbcenabled)
		{
		    mapattrib = gmem->vram[0x2000 | maploc];
		}

		if (TestBit(lcdcontrol, 4))
		{
		    tileloc = (tileloc << 4);
		}
		else
		{
		    tileloc = (128 + (int16_t)(tileloc)) & 0xFF;
		    tileloc = 0x800 + (tileloc << 4);
		}

		int pixelx = (xpos % 8);
		int pixely = (ypos % 8);

		if (gmem->isgbcenabled)
		{
		    if (TestBit(mapattrib, 3))
		    {
			tileloc |= 0x2000;
		    }

		    if (TestBit(mapattrib, 5))
	 	    {
			pixelx = (7 - pixelx);
		    }

		    if (TestBit(mapattrib, 6))
		    {
			pixely = (7 - pixely);
		    }
		}

		uint8_t line = (pixely % 8);
		line *= 2;

		uint8_t data1 = (gmem->vram[tileloc + line]);
		uint8_t data2 = (gmem->vram[tileloc + line + 1]);

		int colorbit = (pixelx % 8);
		colorbit -= 7;
		colorbit *= -1;

		int colornum = BitGetVal(data2, colorbit);
	        colornum <<= 1;
	        colornum |= BitGetVal(data1, colorbit);

	        int red = 0;
	        int green = 0;
	        int blue = 0;

	        if (!gmem->isgbcenabled)
	        {
	    	    // Get actual color from palette
	    	    int color = getdmgcolor(colornum, 0xFF47);

	    	    // Setup RGB values
	    	    switch (color)
	    	    {
		        case 0: red = 0xFF; green = 0xFF; blue = 0xFF; break;
		        case 1: red = 0xCC; green = 0xCC; blue = 0xCC; break;
		        case 2: red = 0x77; green = 0x77; blue = 0x77; break;
		        case 3: red = 0x00; green = 0x00; blue = 0x00; break;
	    	    }
	        }
	        else
	        {
		    int color = getgbccolor((mapattrib & 0x7), colornum);

	    	    red = ((color & 0x1F) << 3);
		    red |= (red >> 5);
		    green = (((color >> 5) & 0x1F) << 3);
		    green |= (green >> 5);
		    blue = (((color >> 10) & 0x1F) << 3);
		    blue |= (blue >> 5);
	        }

	        uint8_t scanline = gmem->memorymap[0xFF44];

	        if ((scanline < 0) || (scanline > 144))
	        {
		    continue;
	        }

	        bgscanline[pixel] = colornum;

	        // Set framebuffer values
	        int index = (pixel + (scanline * 160));
	        framebuffer[index].red = red;
	        framebuffer[index].green = green;
	        framebuffer[index].blue = blue;
	    }
	}
    }

    void GPU::rendersprites(uint8_t lcdcontrol)
    {
	if (!TestBit(lcdcontrol, 1))
	{
	    return;
	}

	uint16_t spritedata = 0x8000;
	int ysize = TestBit(lcdcontrol, 2) ? 16 : 8;
	int spritelimit = 40;

	uint8_t scanline = gmem->memorymap[0xFF44];

	for (int i = (spritelimit - 1); i >= 0; i--)
	{
	    uint8_t index = (i * 4);
	    uint8_t ypos = gmem->readByte(0xFE00 + index) - 16;
	    uint8_t xpos = gmem->readByte(0xFE00 + index + 1) - 8;
	    uint8_t patternnum = gmem->readByte(0xFE00 + index + 2);
	    uint8_t flags = gmem->readByte(0xFE00 + index + 3);

	    bool priority = TestBit(flags, 7);
	    bool yflip = TestBit(flags, 6);
	    bool xflip = TestBit(flags, 5);

	    int bank = 0;

	    if (gmem->isgbcenabled)
	    {
		bank = ((flags & 0x08) >> 3);
	    }

	    uint8_t line = (yflip) ? ((((scanline - ypos - ysize) + 1) * -1)) : ((scanline - ypos));


	    if (TestBit(lcdcontrol, 2))
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

	    uint8_t data1 = gmem->vram[spritedata + (patternnum * 16) + (line * 2) + (bank * 0x2000) - 0x8000];
	    uint8_t data2 = gmem->vram[spritedata + (patternnum * 16) + (line * 2 + 1) + (bank * 0x2000) - 0x8000];

	    // cout << hex << (int)((spritedata + (patternnum * 16) + (line * 2))) << endl;

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

		    if (gmem->gbtype == 1)
		    {
			int color = getdmgcolor(colornum, coloraddr);
			
	    	    	// Setup RGB values
		    	switch (color)
		    	{
			    case 0: red = 0xFF; green = 0xFF; blue = 0xFF; break;
			    case 1: red = 0xCC; green = 0xCC; blue = 0xCC; break;
			    case 2: red = 0x77; green = 0x77; blue = 0x77; break;
			    case 3: red = 0x00; green = 0x00; blue = 0x00; break;
		    	}
	 	    }
		    else
		    {
			int color = getgbcobjcolor((flags & 0x7), colornum);

	    		red = ((color & 0x1F) << 3);
			red |= (red >> 5);
			green = (((color >> 5) & 0x1F) << 3);
			green |= (green >> 5);
			blue = (((color >> 10) & 0x1F) << 3);
			blue |= (blue >> 5);
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

	    	    if (gmem->isgbcenabled && TestBit(lcdcontrol, 0) && (bgprior[xpixel] && !isbgwhite))
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

    int GPU::getdmgcolor(int id, uint16_t palette)
    {
	uint8_t data = gmem->readByte(palette);
	int hi = 2 * id + 1;
	int lo = 2 * id;
	int bit1 = (data >> hi) & 1;
	int bit0 = (data >> lo) & 1;
	return (bit1 << 1) | bit0;
    }

    int GPU::getgbccolor(int id, int color)
    {
	uint8_t idx = ((id * 8) + (color * 2));
	return gmem->gbcbgpallete[idx] | (gmem->gbcbgpallete[idx + 1] << 8);
    }

    int GPU::getgbcobjcolor(int id, int color)
    {
	uint8_t idx = ((id * 8) + (color * 2));
	return gmem->gbcobjpallete[idx] | (gmem->gbcobjpallete[idx + 1] << 8);
    }
}
