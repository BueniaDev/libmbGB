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
		case 2: sweepclock(); lengthclock(); break;
		case 3: break;
		case 4: lengthclock(); break;
		case 5: break;
		case 6: sweepclock(); lengthclock(); break; 
		case 7: envclock(); break;
	    }

	    framesequencer = ((framesequencer + 1) % 8);
	}

	s1step();
	s2step();
	wavestep();
	noisestep();

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
	auto sound3 = getwaveoutputvol();
	auto sound4 = getnoiseoutputvol();

	float leftsample = 0;
	float rightsample = 0;


	if (apumem.leftenables[0])
	{
	    leftsample += sound1;
	}

	if (apumem.leftenables[1])
	{
	    leftsample += sound2;
	}

	if (apumem.leftenables[2])
	{
	    leftsample += sound3;
	}

	if (apumem.leftenables[3])
	{
	    leftsample += sound4;
	}

	if (apumem.rightenables[0])
	{
	    rightsample += sound1;
	}

	if (apumem.rightenables[1])
	{
	    rightsample += sound2;
	}

	if (apumem.rightenables[2])
	{
	    rightsample += sound3;
	}

	if (apumem.rightenables[3])
	{
	    rightsample += sound4;
	}

	leftsample /= 4.0f;
	rightsample /= 4.0f;

	auto leftvolume = (((float)(apumem.leftvol)) / 7.0f);
	auto rightvolume = (((float)(apumem.rightvol)) / 7.0f);

	auto left = (int16_t)(leftsample * leftvolume * ampl);
	auto right = (int16_t)(rightsample * rightvolume * ampl);

	if (audiocallback)
	{
	    audiocallback(left, right);
	}
    }

    void APU::sweepclock()
    {
	s1sweepclock();
    }

    void APU::lengthclock()
    {
	s1lengthclock();
	s2lengthclock();
	wavelengthclock();
	noiselengthclock();
    }

    void APU::envclock()
    {
	s1envclock();
	s2envclock();
	noiseenvclock();
    }
};
