/*
    This file is part of libmbGB.
    Copyright (C) 2021 BueniaDev.

    libmbGB is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    libmbGB is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with libmbGB.  If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef LIBMBGB_APU
#define LIBMBGB_APU

#include "mmu.h"
#include "utils.h"
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

	    void init();
	    void shutdown();

	    void dosavestate(mbGBSavestate &file);

	    MMU& apumem;
		
	    int s1sweep = 0;
	    bool s1negative = false;
	    bool s1sweepenabled = false;
	    uint16_t s1shadowfreq = 0;
	    int s1sweepcounter = 0;
	    int s1soundlength = 0;
	    int s1lengthcounter = 0;
	    int s1volumeenvelope = 0;
	    bool s1envelopeenabled = false;
	    int s1envelopecounter = 0;
	    int s1volume = 0;
	    uint8_t s1freqlo = 0;
	    uint8_t s1freqhi = 0;
	    int s1periodtimer = 0;
	    bool s1enabled = false;
	    bool s1dacenabled = false;
		
	    int s2soundlength = 0;
	    int s2lengthcounter = 0;
	    int s2volumeenvelope = 0;
	    bool s2envelopeenabled = false;
	    int s2envelopecounter = 0;
	    int s2volume = 0;
	    uint8_t s2freqlo = 0;
	    uint8_t s2freqhi = 0;
	    int s2periodtimer = 0;
	    bool s2enabled = false;
	    bool s2dacenabled = false;

	    int wavesweep = 0;
	    int wavesoundlength = 0;
	    int wavelengthcounter = 0;
	    int wavevolumeenvelope = 0;
	    int wavevolume = 0;
	    uint8_t wavefreqlo = 0;
	    uint8_t wavefreqhi = 0;
	    int waveperiodtimer = 0;
	    bool waveenabled = false;
	    int waveramlengthmask = 0;
	    int wavepos = 0;
	    bool wavechannelenabled = false;
	    uint8_t wavecurrentsample = 0;
	    uint8_t wavelastplayedsample = 0;
	    array<uint8_t, 0x10> waveram = {0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF};

	    int noisesoundlength = 0;
	    int noiselengthcounter = 0;
	    int noisevolumeenvelope = 0;
	    bool noiseenvelopeenabled = false;
	    int noiseenvelopecounter = 0;
	    int noiseperiodtimer = 0;
	    uint8_t noisefreqlo = 0;
	    uint8_t noisefreqhi = 0;
	    int noisevolume = 0;
	    uint16_t noiselfsr = 1;
	    bool noiseenabled = false;
	    bool noisedacenabled = false;

	    bool issoundon = true;
		
	    int mastervolume = 0;
	    int soundselect = 0;
	    int soundon = 0;
		
	    array<int, 8> s1dutycycle;
	    array<int, 8> s2dutycycle;
		
	    uint8_t readsoundon();
		
	    uint8_t readapu(uint16_t addr);
	    void writeapu(uint16_t addr, uint8_t val);

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

	    bool prevs1dacenabled = false;

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

	    inline void s1update(int frameseq);
	    void s1sweeptick(int frameseq);
	    void s1lengthcountertick(int frameseq);
	    void s1envelopetick(int frameseq);
	    void s1timertick();
	    float gets1outputvol();
		
	    void s2update(int frameseq);
	    void s2lengthcountertick(int frameseq);
	    void s2envelopetick(int frameseq);
	    void s2timertick();
	    float gets2outputvol();
		
	    void waveupdate(int frameseq);
	    void wavetimertick();
	    void wavelengthcountertick(int frameseq);
	    float getwaveoutputvol();
		
	    void noiseupdate(int frameseq);
	    void noiselengthcountertick(int frameseq);
	    inline void noiseenvelopetick(int frameseq);
	    inline void noisetimertick();
	    float getnoiseoutputvol();
		
	    inline bool s1enabledleft()
	    {
		return (s1enabled && (TestBit(soundselect, 4)));
	    }

	    inline bool s1enabledright()
	    {
		return (s1enabled && (TestBit(soundselect, 0)));
	    }
		
	    inline void writes1sweep(uint8_t value)
	    {
		s1sweep = (value & 0x7F);

		if ((((s1sweep & 0x70) >> 4) == 0) || ((s1sweep & 0x07) == 0) || (!TestBit(s1sweep, 3) && s1negative))
		{
		    s1sweepenabled = false;
		}
	    }

	    inline void reloads1lengthcounter()
	    {
		s1lengthcounter = (64 - (s1soundlength & 0x3F));
		s1soundlength &= 0xC0;	
	    }

	    inline void sets1dutycycle()
	    {
		switch ((s1soundlength & 0xC0) >> 6)
		{
		    case 0: s1dutycycle = {{false, false, false, false, false, false, false, true}}; break;
		    case 1: s1dutycycle = {{true, false, false, false, false, false, false, true}}; break;
		    case 2: s1dutycycle = {{true, false, false, false, false, true, true, true}}; break;
		    case 3: s1dutycycle = {{false, true, true, true, true, true, true, false}}; break;
		    default: break;
		}
	    }

	    inline void s1writereset(uint8_t value)
	    {
		bool lengthwasenable = TestBit(s1freqhi, 6);
		s1freqhi = (value & 0xC7);

		if (apulengthlow() && !lengthwasenable && TestBit(s1freqhi, 6) && s1lengthcounter > 0)
		{
		    s1lengthcounter -= 1;

		    if (s1lengthcounter == 0)
		    {
			s1enabled = false;
		    }
		}

		if (TestBit(s1freqhi, 7))
		{
		    s1resetchannel();
		}
	    }
		
	    inline uint16_t s1sweepcalc()
	    {
		int freqdelta = (s1shadowfreq >> (s1sweep & 0x07));

		if (TestBit(s1sweep, 3))
		{
		    freqdelta *= -1;

		    s1negative = true;
		}

		int newfreq = (s1shadowfreq + freqdelta);

		if (newfreq > 2047)
		{
		    s1sweepenabled = false;
		    s1enabled = false;
		}

		uint16_t finalfreq = (newfreq & 0x7FF);

		return finalfreq;
	    }

	    inline void s1resetchannel()
	    {
		s1enabled = true;
		s1reloadperiod();
		s1freqhi &= 0x7F;

		s1shadowfreq = (s1freqlo | ((s1freqhi & 0x7) << 8));
		s1sweepcounter = ((s1sweep & 0x70) >> 4);
		s1sweepenabled = (s1sweepcounter > 0 && ((s1sweep & 0x07) > 0));

		if ((s1sweep & 0x07) > 0)
		{
		    s1sweepcalc();
		}

		s1negative = false;

		s1volume = ((s1volumeenvelope & 0xF0) >> 4);
		s1envelopecounter = (s1volumeenvelope & 0x07);
		s1envelopeenabled = (s1envelopecounter != 0);

		if ((!TestBit(s1volumeenvelope, 3) && s1volume == 0) || (TestBit(s1volumeenvelope, 3) && s1volume == 0x0F))
		{
		    s1envelopeenabled = false;
		}

		if (s1lengthcounter == 0)
		{
		    s1lengthcounter = 64;

		    if (apulengthlow() && TestBit(s1freqhi, 6))
		    {
			s1lengthcounter -= 1;
		    }
		}

		if (!s1dacenabled)
		{
		    s1enabled = false;
		}
	    }
		
	    inline void s1reloadperiod()
	    {
		int frequency = (s1freqlo | ((s1freqhi & 0x07) << 8));
		s1periodtimer = ((2048 - frequency) << 1);
	    }
		
	    inline bool s2enabledleft()
	    {
		return (s2enabled && (TestBit(soundselect, 5)));
	    }

	    inline bool s2enabledright()
	    {
		return (s2enabled && (TestBit(soundselect, 1)));
	    }

	    inline void reloads2lengthcounter()
	    {
		s2lengthcounter = (64 - (s2soundlength & 0x3F));
		s2soundlength &= 0xC0;	
	    }

	    inline void sets2dutycycle()
	    {
		switch ((s2soundlength & 0xC0) >> 6)
		{
		    case 0: s2dutycycle = {{false, false, false, false, false, false, false, true}}; break;
		    case 1: s2dutycycle = {{true, false, false, false, false, false, false, true}}; break;
		    case 2: s2dutycycle = {{true, false, false, false, false, true, true, true}}; break;
		    case 3: s2dutycycle = {{false, true, true, true, true, true, true, false}}; break;
		    default: break;
		}
	    }

	    inline void s2writereset(uint8_t value)
	    {
		bool lengthwasenable = TestBit(s2freqhi, 6);
		s2freqhi = (value & 0xC7);

		if (apulengthlow() && !lengthwasenable && TestBit(s2freqhi, 6) && s2lengthcounter > 0)
		{
		    s2lengthcounter -= 1;

		    if (s2lengthcounter == 0)
		    {
			s2enabled = false;
		    }
		}

		if (TestBit(s2freqhi, 7))
		{
		    s2resetchannel();
		}
	    }

	    inline void s2resetchannel()
	    {
		s2enabled = true;
		s2reloadperiod();
		s2freqhi &= 0x7F;

		s2volume = ((s2volumeenvelope & 0xF0) >> 4);
		s2envelopecounter = (s2volumeenvelope & 0x07);
		s2envelopeenabled = (s2envelopecounter != 0);


		if ((!TestBit(s2volumeenvelope, 3) && s2volume == 0) || (TestBit(s2volumeenvelope, 3) && s2volume == 0x0F))
		{
		    s2envelopeenabled = false;
		}

		if (s2lengthcounter == 0)
		{
		    s2lengthcounter = 64;

		    if (apulengthlow() && TestBit(s2freqhi, 6))
		    {
			s2lengthcounter -= 1;
		    }
		}

		if (!s2dacenabled)
		{
		    s2enabled = false;
		}
	    }

	    inline void s2reloadperiod()
	    {
		int frequency = (s2freqlo | ((s2freqhi & 0x07) << 8));
		s2periodtimer = ((2048 - frequency) << 1);
	    }
		
	    inline bool waveenabledleft()
	    {
		return (waveenabled && (TestBit(soundselect, 6)));
	    }

	    inline bool waveenabledright()
	    {
		return (waveenabled && (TestBit(soundselect, 2)));
	    }
		
	    inline void reloadwavelengthcounter()
	    {
		wavelengthcounter = (256 - wavesoundlength);
		wavesoundlength = 0;	
	    }

	    inline void wavereloadperiod()
	    {
		int frequency = (wavefreqlo | ((wavefreqhi & 0x07) << 8));
		waveperiodtimer = (2048 - frequency);
	    }

	    inline void wavewritereset(uint8_t value)
	    {
		bool lengthwasenable = TestBit(wavefreqhi, 6);
		wavefreqhi = (value & 0xC7);

		if (apulengthlow() && !lengthwasenable && TestBit(wavefreqhi, 6) && wavelengthcounter > 0)
		{
		    wavelengthcounter -= 1;

		    if (wavelengthcounter == 0)
		    {
			waveenabled = false;
		    }
		}

		if (TestBit(wavefreqhi, 7))
		{
		    waveresetchannel();
		}
	    }

	    inline void waveresetchannel()
	    {
		waveenabled = true;
		wavereloadperiod();
		wavefreqhi &= 0x7F;

		if (wavelengthcounter == 0)
		{
		    wavelengthcounter = 256;

		    if (apulengthlow() && TestBit(wavefreqhi, 6))
		    {
			wavelengthcounter -= 1;
		    }
		}

		wavepos = 0;
		waveenabled = TestBit(wavesweep, 7);
		wavecurrentsample = wavelastplayedsample;
	    }
		
	    inline bool noiseenabledleft()
	    {
		return (noiseenabled && (TestBit(soundselect, 7)));
	    }

	    inline bool noiseenabledright()
	    {
		return (noiseenabled && (TestBit(soundselect, 3)));
	    }
		
	    inline void reloadnoiselengthcounter()
	    {
		noiselengthcounter = (64 - (noisesoundlength & 0x3F));
		noisesoundlength &= 0xC0;
	    }

	    inline void noisewritereset(uint8_t value)
	    {
		bool lengthwasenable = TestBit(noisefreqhi, 6);
		noisefreqhi = (value & 0xC0);

		if (apulengthlow() && !lengthwasenable && TestBit(noisefreqhi, 6) && noiselengthcounter > 0)
		{
		    noiselengthcounter -= 1;

		    if (noiselengthcounter == 0)
		    {
			noiseenabled = false;
		    }
		}

		if (TestBit(noisefreqhi, 7))
		{
		    noiseresetchannel();
		}
	    }

	    inline void noisereloadperiod()
	    {
		uint32_t clockdivider = max(((noisefreqlo & 0x07) << 1), 1);
		noiseperiodtimer = (clockdivider << (((noisefreqlo & 0xF0) >> 4) + 2));
	    }

	    inline void noiseresetchannel()
	    {
		noiseenabled = true;
		noisereloadperiod();
		noisefreqhi &= 0x7F;

		noisevolume = ((noisevolumeenvelope & 0xF0) >> 4);
		noiseenvelopecounter = (noisevolumeenvelope & 0x07);
		noiseenvelopeenabled = (noiseenvelopecounter != 0);


		if ((!TestBit(noisevolumeenvelope, 3) && noisevolume == 0) || (TestBit(noisevolumeenvelope, 3) && noisevolume == 0x0F))
		{
		    noiseenvelopeenabled = false;
		}

		if (noiselengthcounter == 0)
		{
		    noiselengthcounter = 64;

		    if (apulengthlow() && TestBit(noisefreqhi, 6))
		    {
			noiselengthcounter -= 1;
		    }
		}
	
		noiselfsr = 0xFFFF;

		if (!noisedacenabled)
		{
		    noiseenabled = false;
		}
	    }
		
	    inline void writesoundon(uint8_t value)
	    {
		bool wasenabled = TestBit(soundon, 7);
		soundon = (value & 0x80);

		if (wasenabled && !TestBit(soundon, 7))
		{
		    clearregisters();
		    issoundon = false;
		}
		else
		{
		    issoundon = true;
		}
	    }

	    inline void clearregisters()
	    {
	    	s1sweep = 0;
	    	s1negative = false;
	    	s1sweepenabled = false;
	    	s1shadowfreq = 0;
	    	s1sweepcounter = 0;
	    	s1soundlength = 0;

		if (apumem.gameboy != Console::DMG)
		{
	    	    s1lengthcounter = 0;
		}

	    	s1volumeenvelope = 0;
	    	s1envelopeenabled = false;
	    	s1envelopecounter = 0;
	    	s1volume = 0;
	    	s1freqlo = 0;
	    	s1freqhi = 0;
	    	s1periodtimer = 0;
	    	s1enabled = false;

	    	s2soundlength = 0;

		if (apumem.gameboy != Console::DMG)
		{
	    	    s2lengthcounter = 0;
		}

	    	s2volumeenvelope = 0;
	    	s2envelopeenabled = false;
	    	s2envelopecounter = 0;
	    	s2volume = 0;
	    	s2freqlo = 0;
	    	s2freqhi = 0;
	    	s2periodtimer = 0;
	    	s2enabled = false;

	    	wavesweep = 0;
	    	wavesoundlength = 0;

		if (apumem.gameboy != Console::DMG)
		{
	    	    wavelengthcounter = 0;
		}

	    	wavevolumeenvelope = 0;
	    	wavevolume = 0;
	    	wavefreqlo = 0;
	    	wavefreqhi = 0;
	    	waveperiodtimer = 0;
	    	waveenabled = false;
	    	waveramlengthmask = 0;
	    	wavepos = 0;
	    	wavechannelenabled = false;
	    	wavecurrentsample = 0;
	    	wavelastplayedsample = 0;

	    	noisesoundlength = 0;
		
		if (apumem.gameboy != Console::DMG)
		{
	    	    noiselengthcounter = 0;
		}

	    	noisevolumeenvelope = 0;
	    	noiseenvelopeenabled = false;
	    	noiseenvelopecounter = 0;
	    	noiseperiodtimer = 0;
	    	noisefreqlo = 0;
	    	noisefreqhi = 0;
	    	noisevolume = 0;
	    	noiselfsr = 1;
	    	noiseenabled = false;

	    	mastervolume = 0;
	    	soundselect = 0;
	    	soundon = 0;
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
