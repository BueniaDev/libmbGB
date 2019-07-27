// This file is part of libmbGB.
// Copyright (C) 2019 Buenia.
//
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

#include "../../include/libmbGB/apu.h"
using namespace gb;

namespace gb
{
    APU::APU(MMU& memory) : apumem(memory)
    {

    }

    APU::~APU()
    {

    }

    void APU::updateaudio()
    {
	int cycles = 2;

	while (cycles-- != 0)
	{
	if (framecounter++ == 8192)
	{
	    framecounter = 0;

	    switch (framesequencer)
	    {
		case 0: lengthclock(); break;
		case 1: break;
		case 2: lengthclock(); break; // TODO: Sweep clock
		case 3: break;
		case 4: lengthclock(); break;
		case 5: break;
		case 6: lengthclock(); break; // TODO: Sweep clock
		case 7: envclock(); break;
	    }

	    framesequencer = ((framesequencer + 1) % 8);
	}

	s1step();
	s2step();

	if (samplecounter++ == initialsamplerate)
	{
	    samplecounter = 0;
	    mixaudio();
	}
	}
    }

    void APU::mixaudio()
    {
	static constexpr float ampl = 30000;
	auto sound1 = gets1outputvol();
	auto sound2 = gets2outputvol();

	float leftsample = 0;
	float rightsample = 0;

	leftsample += sound1;
	rightsample += sound1;

	leftsample += sound2;
	rightsample += sound2;

	leftsample /= 2.0f;
	rightsample /= 2.0f;

	auto leftvolume = (((float)(4.0f)) / 7.0f);
	auto rightvolume = (((float)(4.0f)) / 7.0f);

	auto left = (int16_t)(leftsample * leftvolume * ampl);
	auto right = (int16_t)(rightsample * rightvolume * ampl);

	if (audiocallback)
	{
	    audiocallback(left, right);
	}
    }

    void APU::lengthclock()
    {
	s1lengthclock();
	s2lengthclock();
    }

    void APU::envclock()
    {
	s1envclock();
	s2envclock();
    }
};
