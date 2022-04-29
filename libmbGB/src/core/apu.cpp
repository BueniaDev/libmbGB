/*
    This file is part of libmbGB.
    Copyright (C) 2022 BueniaDev.

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

#include "apu.h"
using namespace gb;
using namespace std::placeholders;

namespace gb
{
    APU::APU(MMU& memory) : apumem(memory)
    {
	for (int i = 0xFF10; i < 0xFF40; i++)
	{
	    apumem.addmemoryreadhandler(i, bind(&APU::readapu, this, _1));
	    apumem.addmemorywritehandler(i, bind(&APU::writeapu, this, _1, _2));
	}
    }

    APU::~APU()
    {

    }

    void APU::init()
    {
	clearregisters();
	s1lengthcounter = 0;
	s2lengthcounter = 0;
	wavelengthcounter = 0;
	noiselengthcounter = 0;
	cout << "APU::Initialized" << endl;
    }

    void APU::shutdown()
    {
	cout << "APU::Shutting down..." << endl;
    }

    void APU::dosavestate(mbGBSavestate &file)
    {
	file.section("APU ");
	file.varint(&s1sweep);
	file.bool32(&s1negative);
	file.bool32(&s1sweepenabled);
	file.var16(&s1shadowfreq);
	file.varint(&s1sweepcounter);
	file.varint(&s1soundlength);
	file.varint(&s1lengthcounter);
	file.varint(&s1volumeenvelope);
	file.bool32(&s1envelopeenabled);
	file.varint(&s1envelopecounter);
	file.varint(&s1volume);
	file.var8(&s1freqlo);
	file.var8(&s1freqhi);
	file.varint(&s1periodtimer);
	file.bool32(&s1enabled);

	file.varint(&s2soundlength);
	file.varint(&s2lengthcounter);
	file.varint(&s2volumeenvelope);
	file.bool32(&s2envelopeenabled);
	file.varint(&s2envelopecounter);
	file.varint(&s2volume);
	file.var8(&s2freqlo);
	file.var8(&s2freqhi);
	file.varint(&s2periodtimer);
	file.bool32(&s2enabled);

	file.varint(&wavesweep);
	file.varint(&wavesoundlength);
	file.varint(&wavelengthcounter);
	file.varint(&wavevolumeenvelope);
	file.varint(&wavevolume);
	file.var8(&wavefreqlo);
	file.var8(&wavefreqhi);
	file.varint(&waveperiodtimer);
	file.bool32(&waveenabled);
	file.varint(&waveramlengthmask);
	file.varint(&wavepos);
	file.bool32(&wavechannelenabled);
	file.var8(&wavecurrentsample);
	file.var8(&wavelastplayedsample);
	file.vararray(waveram.data(), 0x10);

	file.varint(&noisesoundlength);
	file.varint(&noiselengthcounter);
	file.varint(&noisevolumeenvelope);
	file.bool32(&noiseenvelopeenabled);
	file.varint(&noiseenvelopecounter);
	file.varint(&noiseperiodtimer);
	file.var8(&noisefreqlo);
	file.var8(&noisefreqhi);
	file.varint(&noisevolume);
	file.var16(&noiselfsr);
	file.bool32(&noiseenabled);
	
	file.bool32(&issoundon);

	file.varint(&mastervolume);
	file.varint(&soundselect);
	file.varint(&soundon);
	file.vararray(s1dutycycle.data(), 8);
	file.vararray(s2dutycycle.data(), 8);

	file.varint(&frametimer);
	file.varint(&s1seqpointer);
	file.varint(&s2seqpointer);
	file.varint(&samplecounter);
	file.varint(&maxsamples);

	file.bool32(&prevs1sweepinc);
	file.bool32(&prevs1lengthdec);
	file.bool32(&prevs1envelopeinc);
	file.bool32(&prevs2lengthdec);
	file.bool32(&prevs2envelopeinc);
	file.bool32(&prevwavelengthdec);
	file.bool32(&prevnoiselengthdec);
	file.bool32(&prevnoiseenvelopeinc);
    }
	
    uint8_t APU::readapu(uint16_t addr)
    {
	uint8_t temp = 0;
		
	if (addr <= 0xFF26)
	{
	    switch ((addr & 0xFF))
	    {
		case 0x10: temp = (s1sweep | 0x80); break;
		case 0x11: temp = (s1soundlength | 0x3F); break;
		case 0x12: temp = s1volumeenvelope; break;
		case 0x14: temp = (s1freqhi | 0xBF); break;
		case 0x16: temp = (s2soundlength | 0x3F); break;
		case 0x17: temp = s2volumeenvelope; break;
		case 0x19: temp = (s2freqhi | 0xBF); break;
		case 0x1A: temp = (wavesweep | 0x7F); break;
		case 0x1C: temp = (wavevolumeenvelope | 0x9F); break;
		case 0x1E: temp = (wavefreqhi | 0xBF); break;
		case 0x21: temp = noisevolumeenvelope; break;
		case 0x22: temp = noisefreqlo; break;
		case 0x23: temp = (noisefreqhi | 0xBF); break;
		case 0x24: temp = mastervolume; break;
		case 0x25: temp = soundselect; break;
		case 0x26: temp = readsoundon(); break;
		default: temp = 0xFF; break;
	    }
	}
	else if (inRange(addr, 0xFF30, 0xFF40))
	{
	    temp = waveram[addr - 0xFF30];
	}
	else
	{
	    temp = 0xFF;
	}
		
	return temp;
    }
	
    void APU::writeapu(uint16_t addr, uint8_t value)
    {
	if (addr <= 0xFF26)
	{
	    if ((!issoundon) && (addr != 0xFF26))
	    {
		switch ((addr & 0xFF))
		{
		    case 0x11:
		    {
			s1soundlength = (value & 0x3F);
			reloads1lengthcounter();
		    }
		    break;
		    case 0x16:
		    {
			s2soundlength = (value & 0x3F);
			reloads2lengthcounter();
		    }
		    break;
		    case 0x1B:
		    {
			wavesoundlength = value;
			reloadwavelengthcounter();
		    }
		    break;
		    case 0x20:
		    {
			noisesoundlength = (value & 0x3F);
			reloadnoiselengthcounter();
		    }
		    break;
		}


		return;
	    }

	    switch ((addr & 0xFF))
	    {
		case 0x10: writes1sweep(value); break;
		case 0x11:
	    	{
		    s1soundlength = value;
		    reloads1lengthcounter();
		    sets1dutycycle();
	    	}
		break;
		case 0x12:
	    	{
		    s1volumeenvelope = value;

		    if (((s1volumeenvelope & 0xF8) >> 3) == 0)
		    {
			s1dacenabled = false;
			s1enabled = false;
		    }
		    else
		    {
			s1dacenabled = true;
		    }
	   	}
		break;
		case 0x13: s1freqlo = value; break;
		case 0x14: s1writereset(value); break;
		case 0x16:
	    	{
		    s2soundlength = value;
		    reloads2lengthcounter();
		    sets2dutycycle();
	    	}
		break;
		case 0x17:
	    	{
		    s2volumeenvelope = value;

		    if (((s2volumeenvelope & 0xF8) >> 3) == 0)
		    {
			s2dacenabled = false;
			s2enabled = false;
		    }
		    else
		    {
			s2dacenabled = true;
		    }
	    	}
		break;
		case 0x18: s2freqlo = value; break;
		case 0x19: s2writereset(value); break;
		case 0x1A:
	    	{
		    wavesweep = (value & 0x80);

		    if (!testbit(wavesweep, 7))
		    {
			waveenabled = false;
		    }

		    waveramlengthmask = 0x1F;
	    	}
		break;
		case 0x1B:
	    	{
		    wavesoundlength = value;
		    reloadwavelengthcounter();
	    	}
		break;
		case 0x1C:
	    	{
		    wavevolumeenvelope = (value & 0x60);
		    int wavevolumeshift = ((wavevolumeenvelope & 0x60) >> 5);
		    wavevolume = (wavevolumeshift) ? (wavevolumeshift - 1) : 4;
	    	}
		break;
		case 0x1D: wavefreqlo = value; break;
		case 0x1E: wavewritereset(value); break;
		case 0x20:
	    	{
		    noisesoundlength = (value & 0x3F);
		    reloadnoiselengthcounter();
	    	}
		break;
		case 0x21:
		{
		    noisevolumeenvelope = value;

		    if (((noisevolumeenvelope & 0xF8) >> 3) == 0)
		    {
			noisedacenabled = false;
			noiseenabled = false;
		    }
		    else
		    {
			noisedacenabled = true;
		    }
		}
		break;
		case 0x22: noisefreqlo = value; break;
		case 0x23: noisewritereset(value); break;
		case 0x24: mastervolume = value; break;
		case 0x25: soundselect = value; break;
		case 0x26: writesoundon(value); break;	
		default: return; break;
	    }
    	}
	else if (inRange(addr, 0xFF30, 0xFF40))
	{
	    waveram[addr - 0xFF30] = value;
    	}
	else
	{
	    return;
    	}
    }
	
    uint8_t APU::readsoundon()
    {
	uint8_t temp = soundon;
	temp |= 0x70;
	temp |= ((int)(s1enabled) << 0);
	temp |= ((int)(s2enabled) << 1);
	temp |= ((int)(waveenabled) << 2);
	temp |= ((int)(noiseenabled) << 3);

	return temp;
    }

    void APU::s1sweeptick(int frameseq)
    {
	bool sweepinc = testbit(frameseq, 1);

	if (s1sweepenabled)
	{
	    if (!sweepinc && prevs1sweepinc)
	    {
		s1sweepcounter -= 1;

		if (s1sweepcounter == 0)
	    	{
		    s1shadowfreq = s1sweepcalc();
		    s1freqlo = (s1shadowfreq & 0xFF);
		    s1freqhi &= 0xF8;
		    s1freqhi |= ((s1shadowfreq & 0x0700) >> 8);

		    s1sweepcalc();
		    s1sweepcounter = (((s1sweep & 0x70) >> 4) + 1);
	    	}
	    }
    	}

	prevs1sweepinc = sweepinc;
    }
	
    void APU::s1lengthcountertick(int frameseq)
    {
	bool lengthcounterdec = testbit(frameseq, 0);

	if (testbit(s1freqhi, 6) && s1lengthcounter > 0)
	{
	    if (!lengthcounterdec && prevs1lengthdec)
	    {
		s1lengthcounter -= 1;

		if (s1lengthcounter == 0)
		{
		    s1enabled = false;
		}
	    }
	}

	prevs1lengthdec = lengthcounterdec;
    }
	
    void APU::s1envelopetick(int frameseq)
    {
	bool envelopeinc = testbit(frameseq, 2);

	if (s1envelopeenabled)
	{
	    if (!envelopeinc && prevs1envelopeinc)
	    {
		s1envelopecounter -= 1;

		if (s1envelopecounter == 0)
		{
		    if (!testbit(s1volumeenvelope, 3))
		    {
			s1volume -= 1;

			if (s1volume == 0)
			{
			    s1envelopeenabled = false;
			}
		    }
		    else
		    {
			s1volume += 1;
			if (s1volume == 0x0F)
			{
			    s1envelopeenabled = false;
			}
		    }

		    s1envelopecounter = (s1volumeenvelope & 0x7);
		}
	    }
	}

	prevs1envelopeinc = envelopeinc;
    }
	
    inline void APU::s1timertick()
    {
	if (s1periodtimer == 0)
	{
	    s1seqpointer = ((s1seqpointer + 1) & 7);
	    s1reloadperiod();
	}
	else
	{
	    s1periodtimer -= 1;
	}
    }
	
    void APU::s1update(int frameseq)
    {
	s1sweeptick(frameseq);
	s1timertick();
	s1lengthcountertick(frameseq);
	s1envelopetick(frameseq);
    }
	
    float APU::gets1outputvol()
    {
	int outputvol = 0;
	outputvol = (s1dutycycle[s1seqpointer] * s1volume);

	return ((float)(outputvol) / 100.f);
    }
	
    void APU::s2lengthcountertick(int frameseq)
    {
	bool lengthcounterdec = testbit(frameseq, 0);

	if (testbit(s2freqhi, 6) && s2lengthcounter > 0)
	{
	    if (!lengthcounterdec && prevs2lengthdec)
	    {
		s2lengthcounter -= 1;

		if (s2lengthcounter == 0)
		{
		    s2enabled = false;
		}
	    }
	}

	prevs2lengthdec = lengthcounterdec;
    }

    void APU::s2envelopetick(int frameseq)
    {
	bool envelopeinc = testbit(frameseq, 2);

	if (s2envelopeenabled)
	{
	    if (!envelopeinc && prevs2envelopeinc)
	    {
		s2envelopecounter -= 1;

		if (s2envelopecounter == 0)
		{
		    if (!testbit(s2volumeenvelope, 3))
		    {
			s2volume -= 1;
			if (s2volume == 0)
			{
			    s2envelopeenabled = false;
			}
		    }
		    else
		    {
			s2volume += 1;
			if (s2volume == 0x0F)
			{
			    s2envelopeenabled = false;
			}
		    }

		    s2envelopecounter = (s2volumeenvelope & 0x7);
		}
	    }
	}

	prevs2envelopeinc = envelopeinc;
    }

    void APU::s2timertick()
    {
	if (s2periodtimer == 0)
	{
	    s2seqpointer = ((s2seqpointer + 1) & 7);
	    s2reloadperiod();
	}
	else
	{
	    s2periodtimer -= 1;
	}
    }
	
    void APU::s2update(int frameseq)
    {
	s2timertick();
	s2lengthcountertick(frameseq);
	s2envelopetick(frameseq);
    }

    float APU::gets2outputvol()
    {
	int outputvol = 0;
	outputvol = (s2dutycycle[s2seqpointer] * s2volume);

	return ((float)(outputvol) / 100.f);
    }
	
    void APU::wavetimertick()
    {
	if (waveperiodtimer == 0)
	{
	    wavelastplayedsample = wavecurrentsample;
	    wavepos = ((wavepos + 1) & waveramlengthmask);
	
	    int playingbankoffs = (((wavesweep & 0x40) >> 6) * 32);

	    int sampleindex = ((wavepos + playingbankoffs) & 0x3F);
	    uint8_t samplebyte = waveram[sampleindex >> 1];

	    wavecurrentsample = (testbit(sampleindex, 0) ? (samplebyte & 0x0F) : (samplebyte >> 4));
	    wavereloadperiod();
	}
	else
	{
	    waveperiodtimer -= 1;
	}
    }

    void APU::wavelengthcountertick(int frameseq)
    {
	bool lengthcounterdec = testbit(frameseq, 0);

	if (testbit(wavefreqhi, 6) && wavelengthcounter > 0)
	{
	    if (!lengthcounterdec && prevwavelengthdec)
	    {
		wavelengthcounter -= 1;

		if (wavelengthcounter == 0)
		{
		    waveenabled = false;
		}
	    }
	}

	prevwavelengthdec = lengthcounterdec;
    }
	
    void APU::waveupdate(int frameseq)
    {
	wavetimertick();
	wavelengthcountertick(frameseq);
    }
	
    float APU::getwaveoutputvol()
    {
	int outputvol = 0;
	if (waveenabled)
	{
	    outputvol = (wavecurrentsample >> wavevolume);
	}
	else
	{
	    outputvol = 0;
	}

	return ((float)(outputvol) / 100.f);
    }
	
    void APU::noiselengthcountertick(int frameseq)
    {
	bool lengthcounterdec = testbit(frameseq, 0);

	if (testbit(noisefreqhi, 6) && noiselengthcounter > 0)
	{
	    if (!lengthcounterdec && prevnoiselengthdec)
	    {
		noiselengthcounter -= 1;

		if (noiselengthcounter == 0)
		{
		    noiseenabled = false;
		}
	    }
	}

	prevnoiselengthdec = lengthcounterdec;
    }

    void APU::noiseenvelopetick(int frameseq)
    {
	bool envelopeinc = testbit(frameseq, 2);

	if (noiseenvelopeenabled)
	{
	    if (!envelopeinc && prevnoiseenvelopeinc)
	    {
		noiseenvelopecounter -= 1;

		if (noiseenvelopecounter == 0)
		{
		    if (!testbit(noisevolumeenvelope, 3))
		    {
			noisevolume -= 1;

			if (noisevolume == 0)
			{
			    noiseenvelopeenabled = false;
			}
		    }
		    else
		    {
			noisevolume += 1;

			if (noisevolume == 0x0F)
			{
			    noiseenvelopeenabled = false;
			}
		    }

		    noiseenvelopecounter = (noisevolumeenvelope & 0x7);
		}
	    }
	}

	prevnoiseenvelopeinc = envelopeinc;
    }

    void APU::noisetimertick()
    {
	if (noiseperiodtimer == 0)
	{
	    if (((noisefreqlo & 0xF0) >> 4) < 14)
	    {
		int xoredbits = ((noiselfsr ^ (noiselfsr >> 1)) & 1);
		noiselfsr >>= 1;
		noiselfsr |= (xoredbits << 14);

		if (testbit(noisefreqlo, 3))
		{
		    noiselfsr = resetbit(noiselfsr, 6);
		    noiselfsr |= (xoredbits << 6);
		}
	    }

	    noisereloadperiod();
	}
	else
	{
	    noiseperiodtimer -= 1;
	}
    }
	
    void APU::noiseupdate(int frameseq)
    {
	noisetimertick();
	noiselengthcountertick(frameseq);
	noiseenvelopetick(frameseq);
    }

    float APU::getnoiseoutputvol()
    {
	int outputvol = 0;
	if (noiseenabled)
	{
	    outputvol = (testbit((~noiselfsr), 0) * noisevolume);
	}
	else
	{
	    outputvol = 0;
	}

	return ((float)(outputvol) / 100.f);
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

    void APU::mixaudio()
    {
	auto sound1 = gets1outputvol();
	auto sound2 = gets2outputvol();
	auto sound3 = getwaveoutputvol();
	auto sound4 = getnoiseoutputvol();

	float leftsample = 0;
	float rightsample = 0;

	if (s1enabledleft())
	{
	    leftsample += sound1;
	}

	if (s1enabledright())
	{
	    rightsample += sound1;
	}

	if (s2enabledleft())
	{
	    leftsample += sound2;
	}

	if (s2enabledright())
	{
	    rightsample += sound2;
	}

	if (waveenabledleft())
	{
	    leftsample += sound3;
	}

	if (waveenabledright())
	{
	    rightsample += sound3;
	}

	if (noiseenabledleft())
	{
	    leftsample += sound4;
	}

	if (noiseenabledright())
	{
	    rightsample += sound4;
	}

	leftsample /= 4.0f;
	rightsample /= 4.0f;

	int mastervolleft = ((mastervolume >> 4) & 0x7);
	int mastervolright = (mastervolume & 0x7);

	auto leftvolume = (((float)(mastervolleft)) / 7.0f);
	auto rightvolume = (((float)(mastervolright)) / 7.0f);

	float leftfloat = (float)(leftsample * leftvolume);
	float rightfloat = (float)(rightsample * rightvolume);

	static constexpr float ampl = 30000;
	auto left = (int16_t)(leftfloat * ampl);
	auto right = (int16_t)(rightfloat * ampl);
	

	if (audiocallback)
	{
	    audiocallback(left, right);
	}
    }
};
