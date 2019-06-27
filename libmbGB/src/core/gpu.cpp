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
	    if (scanlinecounter == 0)
	    {
		gpumem.setstatmode(2);
	    }
	    else if (scanlinecounter == 84)
	    {
		gpumem.setstatmode(3);
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
		gpumem.statinterruptsignal |= mode2check();
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
};