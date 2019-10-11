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
	auto apu = bind(&APU::apulengthlow, this);
	apumem.setapucallbacks(apu);
    }

    APU::~APU()
    {

    }

    void APU::updateaudio()
    {
	frametimer += 2;

	s1update(getframesequencer());
	s2update(getframesequencer());
	waveupdate(getframesequencer());
	noiseupdate(getframesequencer());

	if (samplecounter == maxsamples)
	{
	    samplecounter = 0;
	    mixaudio();
	}
	else
	{
	    samplecounter += 1;
	}
    }

    void APU::mixs16audio()
    {
	static constexpr float ampl = 30000;

	auto sound1 = gets1outputvol();
	auto sound2 = gets2outputvol();
	auto sound3 = getwaveoutputvol();
	auto sound4 = getnoiseoutputvol();

	float leftsample = 0;
	float rightsample = 0;

	if (apumem.s1enabledleft())
	{
	    leftsample += sound1;
	}

	if (apumem.s1enabledright())
	{
	    rightsample += sound1;
	}

	if (apumem.s2enabledleft())
	{
	    leftsample += sound2;
	}

	if (apumem.s2enabledright())
	{
	    rightsample += sound2;
	}

	if (apumem.waveenabledleft())
	{
	    leftsample += sound3;
	}

	if (apumem.waveenabledright())
	{
	    rightsample += sound3;
	}

	if (apumem.noiseenabledleft())
	{
	    leftsample += sound4;
	}

	if (apumem.noiseenabledright())
	{
	    rightsample += sound4;
	}

	leftsample /= 4.0f;
	rightsample /= 4.0f;

	int mastervolleft = ((apumem.mastervolume >> 4) & 0x7);
	int mastervolright = (apumem.mastervolume & 0x7);

	auto leftvolume = (((float)(mastervolleft)) / 7.0f);
	auto rightvolume = (((float)(mastervolright)) / 7.0f);

	auto left = (int16_t)(leftsample * leftvolume * ampl);
	auto right = (int16_t)(rightsample * rightvolume * ampl);

	if (audiocallback)
	{
	    audiocallback(left, right);
	}
    }

    void APU::mixf32audio()
    {
	auto sound1 = gets1outputvol();
	auto sound2 = gets2outputvol();
	auto sound3 = getwaveoutputvol();
	auto sound4 = getnoiseoutputvol();

	float leftsample = 0;
	float rightsample = 0;

	if (apumem.s1enabledleft())
	{
	    leftsample += sound1;
	}

	if (apumem.s1enabledright())
	{
	    rightsample += sound1;
	}

	if (apumem.s2enabledleft())
	{
	    leftsample += sound2;
	}

	if (apumem.s2enabledright())
	{
	    rightsample += sound2;
	}

	if (apumem.waveenabledleft())
	{
	    leftsample += sound3;
	}

	if (apumem.waveenabledright())
	{
	    rightsample += sound3;
	}

	if (apumem.noiseenabledleft())
	{
	    leftsample += sound4;
	}

	if (apumem.noiseenabledright())
	{
	    rightsample += sound4;
	}

	leftsample /= 4.0f;
	rightsample /= 4.0f;

	int mastervolleft = ((apumem.mastervolume >> 4) & 0x7);
	int mastervolright = (apumem.mastervolume & 0x7);

	auto leftvolume = (((float)(mastervolleft)) / 7.0f);
	auto rightvolume = (((float)(mastervolright)) / 7.0f);

	auto left = (float)(leftsample * leftvolume);
	auto right = (float)(rightsample * rightvolume);

	if (audiocallback)
	{
	    audiocallback(left, right);
	}
    }
};
