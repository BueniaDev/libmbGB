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

#ifndef LIBMBGB_APU
#define LIBMBGB_APU

#include "mmu.h"
#include "libmbgb_api.h"
#include <functional>
using namespace gb;
using namespace std;

namespace gb
{
    using apuoutputfunc = function<void(int16_t, int16_t)>;

    class LIBMBGB_API APU
    {
	public:
	    APU(MMU& memory);
	    ~APU();

	    MMU& apumem;

	    int frametimer = 0;
	    int s1seqpointer = 0;
	    int samplecounter = 0;
	    int maxsamples = 0;

	    bool prevs1lengthdec = false;

	    void updateaudio();
	    void mixaudio();

	    apuoutputfunc audiocallback;

	    inline void setaudiocallback(apuoutputfunc cb)
	    {
		audiocallback = cb;
	    }

	    // Since we're updating the APU twice per hardware tick...
	    inline void setsamplerate(int value)
	    {
		// ..collect samples every ((cpurate / 2) / samplerate) cycles
		maxsamples = (int)(2097152 / value);
	    }

	    inline void s1update(int frameseq)
	    {
		s1timertick();
		s1lengthcountertick(frameseq);
	    }

	    inline void s1lengthcountertick(int frameseq)
	    {
		bool lengthcounterdec = TestBit(frameseq, 0);

		if (TestBit(apumem.s1freqhi, 6) && apumem.s1lengthcounter > 0)
		{
		    if (!lengthcounterdec && prevs1lengthdec)
		    {
			apumem.s1lengthcounter -= 1;

			if (apumem.s1lengthcounter == 0)
			{
			    apumem.s1enabled = false;
			}
		    }
		}

		prevs1lengthdec = lengthcounterdec;
	    }

	    inline void s1timertick()
	    {
		if (apumem.s1periodtimer == 0)
		{
		    s1seqpointer = ((s1seqpointer + 1) & 7);

		    apumem.s1reloadperiod();
		}
		else
		{
		    apumem.s1periodtimer -= 1;
		}
	    }

	    float gets1outputvol()
	    {
		int outputvol = 0;
		if (apumem.s1enabled)
		{
		    outputvol = (apumem.s1dutycycle[s1seqpointer] * 4);
		}
		else
		{
		    outputvol = 0;
		}

		return ((float)(outputvol) / 15.f);
	    }

	    inline int getframesequencer()
	    {
		return (frametimer >> 13);
	    }

	    inline bool apulengthlow()
	    {
		return !TestBit(getframesequencer(), 0);
	    }
    };
};

#endif // LIBMBGB_APU
