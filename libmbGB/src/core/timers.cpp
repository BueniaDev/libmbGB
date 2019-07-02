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

#include "../../include/libmbGB/timers.h"
using namespace gb;
using namespace std;

namespace gb
{
    Timers::Timers(MMU& memory) : timermem(memory)
    {

    }

    Timers::~Timers()
    {

    }

    void Timers::init()
    {
	cout << "Timers::Initialized" << endl;
    }

    void Timers::shutdown()
    {
	cout << "Timers::Shutting down..." << endl;
    }

    void Timers::updatetimer()
    {
	timermem.divider += 4;

	if (timaoverflownotinterrupted)
	{
	    loadtmaintotima();

	    timaoverflownotinterrupted = false;
	}

	if (timaoverflow)
	{
	    if (timawasnotwritten(timermem.readByte(0xFF05)))
	    {
		timaoverflownotinterrupted = true;
		loadtmaintotima();
		timermem.requestinterrupt(2);
	    }
	    else
	    {
		timaoverflownotinterrupted = false;
	    }

	    timaoverflow = false;
	}

	bool divtickbit = (divbit[tacfreq()] & timermem.divider);
	bool timainc = (divtickbit && tacenable());
	uint8_t timaval = timermem.readByte(0xFF05);

	if (timaincwentlow(timainc))
	{
	    timaoverflow = (++timaval == 0x00);
	    timermem.writeByte(0xFF05, timaval);
	}

	prevtimaval = timaval;
	prevtimainc = timainc;
    }
};
