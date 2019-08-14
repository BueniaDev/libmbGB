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
    #ifdef LIBMBGB_SIGNED16
    using apuoutputfunc = function<void(int16_t, int16_t)>;
    #elif defined LIBMBGB_FLOAT32
    using apuoutputfunc = function<void(float, float)>;
    #else
    #error "Please define an audio output type."
    #endif

    class LIBMBGB_API APU
    {
	public:
	    APU(MMU& memory);
	    ~APU();

	    MMU& apumem;

	    #ifdef LIBMBGB_SIGNED16
	    float divisor = 15.f;
	    #elif defined LIBMBGB_FLOAT32
	    float divisor = 100.f;
	    #endif

	    int frametimer = 0;
	    int s1seqpointer = 0;
	    int s2seqpointer = 0;
	    int samplecounter = 0;
	    int maxsamples = 0;

	    bool prevs1sweepinc = false;
	    bool prevs1lengthdec = false;
	    bool prevs1envelopeinc = false;
	    bool prevs2lengthdec = false;
	    bool prevs2envelopeinc = false;
	    bool prevwavelengthdec = false;
	    bool prevnoiselengthdec = false;
	    bool prevnoiseenvelopeinc = false;

	    void updateaudio();

	    inline void mixaudio()
	    {
		#ifdef LIBMBGB_SIGNED16
		mixs16audio();
		#elif defined LIBMBGB_FLOAT32
		mixf32audio();
		#endif
	    }

	    void mixs16audio();
	    void mixf32audio();

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
		s1sweeptick(frameseq);
		s1timertick();
		s1lengthcountertick(frameseq);
		s1envelopetick(frameseq);
	    }

	    inline void s1sweeptick(int frameseq)
	    {
		bool sweepinc = TestBit(frameseq, 1);

		if (apumem.s1sweepenabled)
		{
		    if (!sweepinc && prevs1sweepinc)
		    {
			apumem.s1sweepcounter -= 1;

			if (apumem.s1sweepcounter == 0)
			{
			    apumem.s1shadowfreq = apumem.s1sweepcalc();
			    apumem.s1freqlo = (apumem.s1shadowfreq & 0xFF);
			    apumem.s1freqhi &= 0xF8;
			    apumem.s1freqhi |= ((apumem.s1shadowfreq & 0x0700) >> 8);

			    apumem.s1sweepcalc();

			    apumem.s1sweepcounter = (((apumem.s1sweep & 0x70) >> 4) + 1);
			}
		    }
		}

		prevs1sweepinc = sweepinc;
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

	    inline void s1envelopetick(int frameseq)
	    {
		bool envelopeinc = TestBit(frameseq, 2);

		if (apumem.s1envelopeenabled)
		{
		    if (!envelopeinc && prevs1envelopeinc)
		    {
			apumem.s1envelopecounter -= 1;

			if (apumem.s1envelopecounter == 0)
			{
			    if (!TestBit(apumem.s1volumeenvelope, 3))
			    {
				apumem.s1volume -= 1;
				if (apumem.s1volume == 0)
				{
				    apumem.s1envelopeenabled = false;
				}
			    }
			    else
			    {
				apumem.s1volume += 1;
				if (apumem.s1volume == 0x0F)
				{
				    apumem.s1envelopeenabled = false;
				}
			    }

			    apumem.s1envelopecounter = (apumem.s1volumeenvelope & 0x7);
			}
		    }
		}

		prevs1envelopeinc = envelopeinc;
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
		outputvol = (apumem.s1dutycycle[s1seqpointer] * apumem.s1volume);

		return ((float)(outputvol) / divisor);
	    }

	    inline void s2update(int frameseq)
	    {
		s2timertick();
		s2lengthcountertick(frameseq);
		s2envelopetick(frameseq);
	    }

	    inline void s2lengthcountertick(int frameseq)
	    {
		bool lengthcounterdec = TestBit(frameseq, 0);

		if (TestBit(apumem.s2freqhi, 6) && apumem.s2lengthcounter > 0)
		{
		    if (!lengthcounterdec && prevs2lengthdec)
		    {
			apumem.s2lengthcounter -= 1;

			if (apumem.s2lengthcounter == 0)
			{
			    apumem.s2enabled = false;
			}
		    }
		}

		prevs2lengthdec = lengthcounterdec;
	    }

	    inline void s2envelopetick(int frameseq)
	    {
		bool envelopeinc = TestBit(frameseq, 2);

		if (apumem.s2envelopeenabled)
		{
		    if (!envelopeinc && prevs2envelopeinc)
		    {
			apumem.s2envelopecounter -= 1;

			if (apumem.s2envelopecounter == 0)
			{
			    if (!TestBit(apumem.s2volumeenvelope, 3))
			    {
				apumem.s2volume -= 1;
				if (apumem.s2volume == 0)
				{
				    apumem.s2envelopeenabled = false;
				}
			    }
			    else
			    {
				apumem.s2volume += 1;
				if (apumem.s2volume == 0x0F)
				{
				    apumem.s2envelopeenabled = false;
				}
			    }

			    apumem.s2envelopecounter = (apumem.s2volumeenvelope & 0x7);
			}
		    }
		}

		prevs2envelopeinc = envelopeinc;
	    }

	    inline void s2timertick()
	    {
		if (apumem.s2periodtimer == 0)
		{
		    s2seqpointer = ((s2seqpointer + 1) & 7);

		    apumem.s2reloadperiod();
		}
		else
		{
		    apumem.s2periodtimer -= 1;
		}
	    }

	    float gets2outputvol()
	    {
		int outputvol = 0;
		outputvol = (apumem.s2dutycycle[s2seqpointer] * apumem.s2volume);

		return ((float)(outputvol) / divisor);
	    }

	    inline void wavetimertick()
	    {
		if (apumem.waveperiodtimer == 0)
		{
		    apumem.wavelastplayedsample = apumem.wavecurrentsample;
		    apumem.wavepos = ((apumem.wavepos + 1) & apumem.waveramlengthmask);
	
		    int playingbankoffs = (((apumem.wavesweep & 0x40) >> 6) * 32);

		    int sampleindex = ((apumem.wavepos + playingbankoffs) & 0x3F);
		    uint8_t samplebyte = apumem.waveram[sampleindex >> 1];

		    apumem.wavecurrentsample = (TestBit(sampleindex, 0) ? (samplebyte & 0x0F) : (samplebyte >> 4));

		    apumem.wavereloadperiod();
		}
		else
		{
		    apumem.waveperiodtimer -= 1;
		}
	    }

	    inline void wavelengthcountertick(int frameseq)
	    {
		bool lengthcounterdec = TestBit(frameseq, 0);

		if (TestBit(apumem.wavefreqhi, 6) && apumem.wavelengthcounter > 0)
		{
		    if (!lengthcounterdec && prevwavelengthdec)
		    {
			apumem.wavelengthcounter -= 1;

			if (apumem.wavelengthcounter == 0)
			{
			    apumem.waveenabled = false;
			}
		    }
		}

		prevwavelengthdec = lengthcounterdec;
	    }

	    inline void waveupdate(int frameseq)
	    {
		wavetimertick();
		wavelengthcountertick(frameseq);
	    }

	    float getwaveoutputvol()
	    {
		int outputvol = 0;
		if (apumem.waveenabled)
		{
		    if (TestBit(apumem.wavevolumeenvelope, 7))
		    {
			outputvol = (apumem.wavecurrentsample) - (apumem.wavecurrentsample >> 2);
		    }
		    else
		    {
			outputvol = (apumem.wavecurrentsample >> apumem.wavevolume);
		    }
		}
		else
		{
		    outputvol = 0;
		}

		return ((float)(outputvol) / divisor);
	    }

	    inline void noiseupdate(int frameseq)
	    {
		noisetimertick();
		noiselengthcountertick(frameseq);
		noiseenvelopetick(frameseq);
	    }

	    inline void noiselengthcountertick(int frameseq)
	    {
		bool lengthcounterdec = TestBit(frameseq, 0);

		if (TestBit(apumem.noisefreqhi, 6) && apumem.noiselengthcounter > 0)
		{
		    if (!lengthcounterdec && prevnoiselengthdec)
		    {
			apumem.noiselengthcounter -= 1;

			if (apumem.noiselengthcounter == 0)
			{
			    apumem.noiseenabled = false;
			}
		    }
		}

		prevnoiselengthdec = lengthcounterdec;
	    }

	    inline void noiseenvelopetick(int frameseq)
	    {
		bool envelopeinc = TestBit(frameseq, 2);

		if (apumem.noiseenvelopeenabled)
		{
		    if (!envelopeinc && prevnoiseenvelopeinc)
		    {
			apumem.noiseenvelopecounter -= 1;

			if (apumem.noiseenvelopecounter == 0)
			{
			    if (!TestBit(apumem.noisevolumeenvelope, 3))
			    {
				apumem.noisevolume -= 1;
				if (apumem.noisevolume == 0)
				{
				    apumem.noiseenvelopeenabled = false;
				}
			    }
			    else
			    {
				apumem.noisevolume += 1;
				if (apumem.noisevolume == 0x0F)
				{
				    apumem.noiseenvelopeenabled = false;
				}
			    }

			    apumem.noiseenvelopecounter = (apumem.noisevolumeenvelope & 0x7);
			}
		    }
		}

		prevnoiseenvelopeinc = envelopeinc;
	    }

	    inline void noisetimertick()
	    {
		if (apumem.noiseperiodtimer == 0)
		{
		    if (((apumem.noisefreqlo & 0xF0) >> 4) < 14)
		    {
			int xoredbits = ((apumem.noiselfsr ^ (apumem.noiselfsr >> 1)) & 1);
			apumem.noiselfsr >>= 1;
			apumem.noiselfsr |= (xoredbits << 14);

			if (TestBit(apumem.noisefreqlo, 3))
			{
			    apumem.noiselfsr = BitReset(apumem.noiselfsr, 6);
			    apumem.noiselfsr |= (xoredbits << 6);
			}
		    }

		    apumem.noisereloadperiod();
		}
		else
		{
		    apumem.noiseperiodtimer -= 1;
		}
	    }

	    float getnoiseoutputvol()
	    {
		int outputvol = 0;
		if (apumem.noiseenabled)
		{
		    outputvol = (TestBit((~apumem.noiselfsr), 0) * apumem.noisevolume);
		}
		else
		{
		    outputvol = 0;
		}

		return ((float)(outputvol) / divisor);
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
