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

	    int framecounter = 0;
	    int framesequencer = 0;
	    int frequencytimer = 0;
	    int s1seqpointer = 0;
	    int s2seqpointer = 0;
	    int samplecounter = 0;
	    int samplefreq = 48000;

	    int initialsamplerate = 87;

	    void updateaudio();
	    void mixaudio();
	    void lengthclock();
	    void envclock();

	    apuoutputfunc audiocallback;

	    inline void setaudiocallback(apuoutputfunc cb)
	    {
		audiocallback = cb;
	    }

	    inline void s1lengthclock()
	    {
		if (apumem.s1lengthenabled && apumem.s1lengthcounter != 0)
		{
		    apumem.s1lengthcounter -= 1;

		    if (apumem.s1lengthcounter == 0)
		    {
			apumem.s1enabled = false;
		    }
		}
	    }

	    inline void s1envclock()
	    {
		if (--apumem.s1envperiod <= 0)
		{
		    apumem.s1envperiod = apumem.s1envperiodload;

		    if (apumem.s1envperiod == 0)
		    {
			apumem.s1envperiod = 8;
		    }

		    if (apumem.s1envrunning && apumem.s1envperiodload > 0)
		    {
			if (apumem.s1envaddmode && apumem.s1volume < 15)
			{
			    apumem.s1volume += 1;
			}
			else if (!apumem.s1envaddmode && apumem.s1volume > 0)
			{
			    apumem.s1volume -= 1;
			}
		    }

		    if (apumem.s1volume == 0 || apumem.s1volume == 15)
		    {
			apumem.s1envrunning = false;
		    }
		}
	    }

	    inline void s1step()
	    {
		if (--apumem.s1timer <= 0)
		{
		    apumem.s1timer = ((2048 - apumem.s1freq) * 4);
		    s1seqpointer = ((s1seqpointer + 1) % 8);
		}

		if (apumem.s1enabled && apumem.s1dacenabled)
		{
		    apumem.s1outputvol = apumem.s1volume;
		}
		else
		{
		    apumem.s1outputvol = 0;
		}

		if (!dutytable[apumem.s1duty][s1seqpointer])
		{
		    apumem.s1outputvol = 0;
		}
	    }

	    inline float gets1outputvol()
	    {
		return ((float)((apumem.s1outputvol) / 15.f));
	    }


	    inline void s2lengthclock()
	    {
		if (apumem.s2lengthenabled && apumem.s2lengthcounter != 0)
		{
		    apumem.s2lengthcounter -= 1;

		    if (apumem.s2lengthcounter == 0)
		    {
			apumem.s2enabled = false;
		    }
		}
	    }

	    inline void s2envclock()
	    {
		if (--apumem.s2envperiod <= 0)
		{
		    apumem.s2envperiod = apumem.s2envperiodload;

		    if (apumem.s2envperiod == 0)
		    {
			apumem.s2envperiod = 8;
		    }

		    if (apumem.s2envrunning && apumem.s2envperiodload > 0)
		    {
			if (apumem.s2envaddmode && apumem.s2volume < 15)
			{
			    apumem.s2volume += 1;
			}
			else if (!apumem.s2envaddmode && apumem.s2volume > 0)
			{
			    apumem.s2volume -= 1;
			}
		    }

		    if (apumem.s2volume == 0 || apumem.s2volume == 15)
		    {
			apumem.s2envrunning = false;
		    }
		}
	    }

	    inline void s2step()
	    {
		if (--apumem.s2timer <= 0)
		{
		    apumem.s2timer = ((2048 - apumem.s2freq) * 4);
		    s2seqpointer = ((s2seqpointer + 1) % 8);
		}

		if (apumem.s2enabled && apumem.s2dacenabled)
		{
		    apumem.s2outputvol = apumem.s2volume;
		}
		else
		{
		    apumem.s2outputvol = 0;
		}

		if (!dutytable[apumem.s2duty][s2seqpointer])
		{
		    apumem.s2outputvol = 0;
		}
	    }

	    inline float gets2outputvol()
	    {
		return ((float)((apumem.s2outputvol) / 15.f));
	    }

	    bool dutytable[4][8] = 
	    {
		{false, false, false, false, false, false, false, true},
		{true, false, false, false, false, false, false, true},
		{true, false, false, false, false, true, true, true},
		{false, true, true, true, true, true, true, false}
	    };
    };
};

#endif // LIBMBGB_APU
