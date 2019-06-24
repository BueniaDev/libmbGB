// This file is part of libmbGB.
// Copyright (C) 2019 Buenia.ui
// libmbGB is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// libmbGB is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with libmbGB.  If not, see <https://www.gnu.org/licenses/>.

#include "../../include/libmbGB/gpu.h"
using namespace gb;

namespace gb
{
    GPU::GPU(MMU& memory) : mem(memory)
    {

    }

    GPU::~GPU()
    {

    }

    void GPU::updatelcd()
    {
	updatepoweronstate();

	if (!mem.islcdenabled())
	{
	    return;
	}

	scanlinecounter += 4;

	updately();

	if (currentscanline <= 143)
	{
	    if (scanlinecounter == 4)
	    {
		mem.setstatmode(2);
	    }
	    else if (scanlinecounter == 84)
	    {
		mem.setstatmode(3);
		renderscanline();
	    }
	    else if (scanlinecounter == mode3cycles())
	    {
		mem.setstatmode(0);
	    }
	}
	else if (currentscanline == 144)
	{
	    if (scanlinecycles == 4)
	    {
		mem.setstatmode(1);
	    }
	}
    }

    void GPU::updatepoweronstate()
    {
	if (mem.islcdenabled())
	{
	    scanlinecounter = 452;
	    currentscanline = 153;
	}
	else
	{
	    mem.ly = 0;
	    mem.setstatmode(0);
	}
    }

    void GPU::updately()
    {
	if (currentscanline == 153 && scanlinecounter == line153cycles())
	{
	    mem.ly = 0;
	}

	if (scanlinecounter == 456)
	{
	    scanlinecounter = 0;

	    if (currentscanline == 153)
	    {
		mem.setstatmode(0);
		currentscanline = 0;
	    }
	    else
	    {
		mem.ly += 1;
		currentscanline = mem.ly;
	    }
	}
    }

    void GPU::renderscanline()
    {
	for (int i = 0; i < 160; i++)
	{
	    if (isbgenabled())
	    {
		renderbg(i);
	    }
	}
    }

    void GPU::renderbg(int pixel)
    {
	uint16_t tilemap = TestBit(mem.lcdc, 3) ? 0x9C00 : 0x9800;
	uint16_t tiledata = TestBit(mem.lcdc, 4) ? 0x8000 : 0x8800;
	bool unsig = TestBit(mem.lcdc, 4);

	uint8_t ypos = 0;

	ypos = mem.scrolly + mem.ly;

	uint16_t tilerow = (((uint8_t)(ypos / 8)) * 32);

	uint8_t xpos = pixel + mem.scrollx;

	uint16_t tilecol = (xpos / 8);
	int16_t tilenum = 0;

	uint16_t tileaddr = (tilemap + tilerow + tilecol);

	uint8_t mapattrib = 0;
    }
};