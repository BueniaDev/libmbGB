/*
    This file is part of libmbGB.
    Copyright (C) 2023 BueniaDev.

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
using namespace std;

namespace gb
{
    GBAPU::GBAPU()
    {

    }

    GBAPU::~GBAPU()
    {

    }

    void GBAPU::init()
    {
	sound_enable = true;
	noise_lfsr = 1;

	noise_enabled = false;

	for (int i = 0x10; i < 0x26; i++)
	{
	    writeIO(i, 0);
	}

	setSampleRate();

	wave_ram = 
	{
	    0x00, 0xFF, 0x00, 0xFF,
	    0x00, 0xFF, 0x00, 0xFF,
	    0x00, 0xFF, 0x00, 0xFF,
	    0x00, 0xFF, 0x00, 0xFF
	};
    }

    void GBAPU::shutdown()
    {
	return;
    }

    uint8_t GBAPU::readIO(uint16_t addr)
    {
	addr &= 0xFF;

	if (inRange(addr, 0x30, 0x40))
	{
	    return wave_ram.at(addr & 0xF);
	}

	uint8_t data = 0;

	switch (addr)
	{
	    case 0x10: data = readSquare1(0); break;
	    case 0x11: data = readSquare1(1); break;
	    case 0x12: data = readSquare1(2); break;
	    case 0x13: data = readSquare1(3); break;
	    case 0x14: data = readSquare1(4); break;
	    case 0x16: data = readSquare2(0); break;
	    case 0x17: data = readSquare2(1); break;
	    case 0x18: data = readSquare2(2); break;
	    case 0x19: data = readSquare2(3); break;
	    case 0x1A: data = readWave(0); break;
	    case 0x1B: data = readWave(1); break;
	    case 0x1C: data = readWave(2); break;
	    case 0x1D: data = readWave(3); break;
	    case 0x1E: data = readWave(4); break;
	    case 0x20: data = readNoise(0); break;
	    case 0x21: data = readNoise(1); break;
	    case 0x22: data = readNoise(2); break;
	    case 0x23: data = readNoise(3); break;
	    case 0x24: data = readNR50(); break;
	    case 0x25: data = readNR51(); break;
	    case 0x26: data = readNR52(); break;
	    default: data = 0xFF; break;
	}

	if (addr <= 0x26)
	{
	    data |= apu_masks.at(addr - 0x10);
	}

	return data;
    }

    void GBAPU::writeIO(uint16_t addr, uint8_t data)
    {
	addr &= 0xFF;

	if (inRange(addr, 0x30, 0x40))
	{
	    wave_ram.at(addr - 0x30) = data;
	    return;
	}

	if (!sound_enable)
	{
	    switch (addr)
	    {
		case 0x26: writeNR52(data); break;
	    }

	    return;
	}

	switch (addr)
	{
	    case 0x10: writeSquare1(0, data); break;
	    case 0x11: writeSquare1(1, data); break;
	    case 0x12: writeSquare1(2, data); break;
	    case 0x13: writeSquare1(3, data); break;
	    case 0x14: writeSquare1(4, data); break;
	    case 0x16: writeSquare2(0, data); break;
	    case 0x17: writeSquare2(1, data); break;
	    case 0x18: writeSquare2(2, data); break;
	    case 0x19: writeSquare2(3, data); break;
	    case 0x1A: writeWave(0, data); break;
	    case 0x1B: writeWave(1, data); break;
	    case 0x1C: writeWave(2, data); break;
	    case 0x1D: writeWave(3, data); break;
	    case 0x1E: writeWave(4, data); break;
	    case 0x20: writeNoise(0, data); break;
	    case 0x21: writeNoise(1, data); break;
	    case 0x22: writeNoise(2, data); break;
	    case 0x23: writeNoise(3, data); break;
	    case 0x24: writeNR50(data); break;
	    case 0x25: writeNR51(data); break;
	    case 0x26: writeNR52(data); break;
	    default: break;
	}
    }

    uint8_t GBAPU::readCGBIO(uint16_t addr)
    {
	uint8_t data = 0;
	addr &= 0xFF;

	switch (addr)
	{
	    case 0x76: data = ((s2_output << 4) | s1_output); break;
	    case 0x77: data = ((noise_output << 4) | wave_output); break;
	}

	return data;
    }

    uint8_t GBAPU::readSquare1(int reg)
    {
	uint8_t data = 0;
	switch (reg)
	{
	    case 0: data = ((s1_sweep_period << 4) | (s1_sweep_negate << 3) | s1_sweep_shift); break;
	    case 1: data = ((s1_duty << 6)); break;
	    case 2: data = ((s1_env_volume << 4) | (s1_env_add << 3) | s1_env_period); break;
	    case 3: data = (s1_freq & 0xFF); break;
	    case 4: data = ((s1_length_enabled << 6) | (s1_freq >> 8)); break;
	}

	return data;
    }

    void GBAPU::writeSquare1(int reg, uint8_t data)
    {
	switch (reg)
	{
	    case 0:
	    {
		s1_sweep_shift = (data & 0x7);
		s1_sweep_negate = testbit(data, 3);
		s1_sweep_period = ((data >> 4) & 0x7);
	    }
	    break;
	    case 1:
	    {
		s1_duty = ((data >> 6) & 0x3);
		s1_length_load = (data & 0x3F);
		s1_length_counter = (64 - s1_length_load);
	    }
	    break;
	    case 2:
	    {
		s1_env_volume = ((data >> 4) & 0xF);
		s1_env_add = testbit(data, 3);
		s1_env_period = (data & 0x7);

		s1_dac_enabled = ((data >> 3) != 0);

		if (!s1_dac_enabled)
		{
		    s1_enabled = false;
		}
	    }
	    break;
	    case 3:
	    {
		s1_freq = ((s1_freq & 0x700) | data);
	    }
	    break;
	    case 4:
	    {
		bool prev_s1_length = s1_length_enabled;
		s1_freq = ((s1_freq & 0xFF) | ((data & 0x7) << 8));
		s1_length_enabled = testbit(data, 6);

		if (testbit(apu_shift, 0) && !prev_s1_length && s1_length_enabled && (s1_length_counter > 0))
		{
		    s1_length_counter -= 1;

		    if (s1_length_counter == 0)
		    {
			s1_enabled = false;
		    }
		}

		if (testbit(data, 7))
		{
		    s1Trigger();
		}
	    }
	    break;
	}
    }

    uint8_t GBAPU::readSquare2(int reg)
    {
	uint8_t data = 0;
	switch (reg)
	{
	    case 0: data = ((s2_duty << 6)); break;
	    case 1: data = ((s2_env_volume << 4) | (s2_env_add << 3) | s2_env_period); break;
	    case 2: data = (s2_freq & 0xFF); break;
	    case 3: data = ((s2_length_enabled << 6) | (s2_freq >> 8)); break;
	}

	return data;
    }

    void GBAPU::writeSquare2(int reg, uint8_t data)
    {
	switch (reg)
	{
	    case 0:
	    {
		s2_duty = ((data >> 6) & 0x3);
		s2_length_load = (data & 0x3F);
		s2_length_counter = (64 - s2_length_load);
	    }
	    break;
	    case 1:
	    {
		s2_env_volume = ((data >> 4) & 0xF);
		s2_env_add = testbit(data, 3);
		s2_env_period = (data & 0x7);

		s2_dac_enabled = ((data >> 3) != 0);

		if (!s2_dac_enabled)
		{
		    s2_enabled = false;
		}
	    }
	    break;
	    case 2:
	    {
		s2_freq = ((s2_freq & 0x700) | data);
	    }
	    break;
	    case 3:
	    {
		bool prev_s2_length = s2_length_enabled;
		s2_freq = ((s2_freq & 0xFF) | ((data & 0x7) << 8));
		s2_length_enabled = testbit(data, 6);

		if (testbit(apu_shift, 0) && !prev_s2_length && s2_length_enabled && (s2_length_counter > 0))
		{
		    s2_length_counter -= 1;

		    if (s2_length_counter == 0)
		    {
			s2_enabled = false;
		    }
		}

		if (testbit(data, 7))
		{
		    s2Trigger();
		}
	    }
	    break;
	}
    }

    uint8_t GBAPU::readWave(int reg)
    {
	uint8_t data = 0;

	switch (reg)
	{
	    case 0: data = (wave_dac_enabled << 7); break;
	    case 1: data = wave_length_load; break;
	    case 2: data = (wave_vol_code << 5); break;
	    case 3: data = (wave_freq & 0xFF); break;
	    case 4: data = ((wave_length_enabled << 6) | (wave_freq >> 8)); break;
	}

	return data;
    }

    void GBAPU::writeWave(int reg, uint8_t data)
    {
	switch (reg)
	{
	    case 0:
	    {
		wave_dac_enabled = testbit(data, 7);

		if (!wave_dac_enabled)
		{
		    wave_enabled = false;
		}
	    }
	    break;
	    case 1:
	    {
		wave_length_load = data;
		wave_length_counter = (256 - wave_length_load);
	    }
	    break;
	    case 2:
	    {
		wave_vol_code = ((data >> 5) & 0x3);
	    }
	    break;
	    case 3:
	    {
		wave_freq = ((wave_freq & 0x700) | data);
	    }
	    break;
	    case 4:
	    {
		bool prev_wave_length = wave_length_enabled;
		wave_freq = ((wave_freq & 0xFF) | ((data & 0x7) << 8));
		wave_length_enabled = testbit(data, 6);

		if (testbit(apu_shift, 0) && !prev_wave_length && wave_length_enabled && (wave_length_counter > 0))
		{
		    wave_length_counter -= 1;

		    if (wave_length_counter == 0)
		    {
			wave_enabled = false;
		    }
		}

		if (testbit(data, 7))
		{
		    waveTrigger();
		}
	    }
	    break;
	}
    }

    uint8_t GBAPU::readNoise(int reg)
    {
	uint8_t data = 0;
	switch (reg)
	{
	    case 0: data = noise_length_load; break;
	    case 1: data = ((noise_env_volume << 4) | (noise_env_add << 3) | noise_env_period); break;
	    case 2: data = ((noise_clock_shift << 4) | (noise_width_mode << 3) | noise_div_code); break;
	    case 3: data = (noise_length_enabled << 6); break;
	}

	return data;
    }

    void GBAPU::writeNoise(int reg, uint8_t data)
    {
	switch (reg)
	{
	    case 0:
	    {
		noise_length_load = (data & 0x3F);
		noise_length_counter = (64 - noise_length_load);
	    }
	    break;
	    case 1:
	    {
		noise_env_volume = ((data >> 4) & 0xF);
		noise_env_add = testbit(data, 3);
		noise_env_period = (data & 0x7);

		noise_dac_enabled = ((data >> 3) != 0);

		if (!noise_dac_enabled)
		{
		    noise_enabled = false;
		}
	    }
	    break;
	    case 2:
	    {
		noise_clock_shift = (data >> 4);
		noise_width_mode = testbit(data, 3);
		noise_div_code = (data & 0x7);
	    }
	    break;
	    case 3:
	    {
		bool prev_noise_length = noise_length_enabled;
		noise_length_enabled = testbit(data, 6);

		if (testbit(apu_shift, 0) && !prev_noise_length && noise_length_enabled && (noise_length_counter > 0))
		{
		    noise_length_counter -= 1;

		    if (noise_length_counter == 0)
		    {
			noise_enabled = false;
		    }
		}

		if (testbit(data, 7))
		{
		    noiseTrigger();
		}
	    }
	    break;
	}
    }

    void GBAPU::clockS1Timer()
    {
	if (s1_period == 0)
	{
	    s1_period = ((2048 - s1_freq) * 4);
	    s1_duty_counter = ((s1_duty_counter + 1) & 0x7);
	}
	else
	{
	    s1_period -= 1;
	}
    }

    void GBAPU::clockS2Timer()
    {
	if (s2_period == 0)
	{
	    s2_period = ((2048 - s2_freq) * 4);
	    s2_duty_counter = ((s2_duty_counter + 1) & 0x7);
	}
	else
	{
	    s2_period -= 1;
	}
    }

    void GBAPU::clockWaveTimer()
    {
	if (wave_period == 0)
	{
	    wave_period = ((2048 - wave_freq) * 2);
	    int sample = wave_ram.at(wave_phase / 2);

	    if (!testbit(wave_phase, 0))
	    {
		sample >>= 4;
	    }
	    else
	    {
		sample &= 0xF;
	    }

	    wave_sample = sample;
	    wave_phase += 1;

	    if (wave_phase == 32)
	    {
		wave_phase = 0;
	    }
	}	
	else
	{
	    wave_period -= 1;
	}
    }

    void GBAPU::clockNoiseTimer()
    {
	if (noise_period == 0)
	{
	    noise_period = (getNoiseDivisor() << noise_clock_shift);

	    bool result = (testbit(noise_lfsr, 0) != testbit(noise_lfsr, 1));
	    noise_lfsr >>= 1;
	    noise_lfsr = changebit(noise_lfsr, 14, result);

	    if (noise_width_mode)
	    {
		noise_lfsr = changebit(noise_lfsr, 6, result);
	    }
	}
	else
	{
	    noise_period -= 1;
	}
    }

    void GBAPU::clockS1Length()
    {
	if (s1_length_enabled && (s1_length_counter > 0))
	{
	    s1_length_counter -= 1;

	    if (s1_length_counter == 0)
	    {
		s1_enabled = false;
	    }
	}
    }

    void GBAPU::clockS2Length()
    {
	if (s2_length_enabled && (s2_length_counter > 0))
	{
	    s2_length_counter -= 1;

	    if (s2_length_counter == 0)
	    {
		s2_enabled = false;
	    }
	}
    }

    void GBAPU::clockWaveLength()
    {
	if (wave_length_enabled && (wave_length_counter > 0))
	{
	    wave_length_counter -= 1;

	    if (wave_length_counter == 0)
	    {
		wave_enabled = false;
	    }
	}
    }

    void GBAPU::clockNoiseLength()
    {
	if (noise_length_enabled && (noise_length_counter > 0))
	{
	    noise_length_counter -= 1;

	    if (noise_length_counter == 0)
	    {
		noise_enabled = false;
	    }
	}
    }

    void GBAPU::clockS1Envelope()
    {
	if (s1_env_enabled)
	{
	    s1_env_freq -= 1;

	    if (s1_env_freq == 0)
	    {
		if (s1_env_add)
		{
		    s1_volume += 1;

		    if (s1_volume == 0xF)
		    {
			s1_env_enabled = false;
		    }
		}
		else
		{
		    s1_volume -= 1;

		    if (s1_volume == 0x0)
		    {
			s1_env_enabled = false;
		    }
		}

		s1_env_freq = (s1_env_period == 0) ? 8 : s1_env_period;
	    }
	}
    }

    void GBAPU::clockS2Envelope()
    {
	if (s2_env_enabled)
	{
	    s2_env_freq -= 1;

	    if (s2_env_freq == 0)
	    {
		if (s2_env_add)
		{
		    s2_volume += 1;

		    if (s2_volume == 0xF)
		    {
			s2_env_enabled = false;
		    }
		}
		else
		{
		    s2_volume -= 1;

		    if (s2_volume == 0x0)
		    {
			s2_env_enabled = false;
		    }
		}

		s2_env_freq = (s2_env_period == 0) ? 8 : s2_env_period;
	    }
	}
    }

    void GBAPU::clockNoiseEnvelope()
    {
	if (noise_env_enabled)
	{
	    noise_env_freq -= 1;

	    if (noise_env_freq == 0)
	    {
		if (noise_env_add)
		{
		    noise_volume += 1;

		    if (noise_volume == 0xF)
		    {
			noise_env_enabled = false;
		    }
		}
		else
		{
		    noise_volume -= 1;

		    if (noise_volume == 0x0)
		    {
			noise_env_enabled = false;
		    }
		}

		noise_env_freq = (noise_env_period == 0) ? 8 : noise_env_period;
	    }
	}
    }

    void GBAPU::clockS1Sweep()
    {
	s1_sweep_freq -= 1;

	if (s1_sweep_freq == 0)
	{
	    s1_sweep_freq = (s1_sweep_period == 0) ? 8 : s1_sweep_period;

	    if (s1_sweep_enabled && (s1_sweep_period > 0))
	    {
		s1SweepCalc(true);
		s1SweepCalc(false);
	    }
	}
    }

    void GBAPU::s1SweepCalc(bool is_update)
    {
	if (!s1_sweep_enabled)
	{
	    return;
	}

	uint16_t shadow_freq = s1_shadow_freq;
	uint16_t freq_delta = (shadow_freq >> s1_sweep_shift);

	if (s1_sweep_negate)
	{
	    shadow_freq -= freq_delta;
	}
	else
	{
	    shadow_freq += freq_delta;
	}

	shadow_freq = clamp<uint16_t>(shadow_freq, 0, 2048);

	if (shadow_freq > 2047)
	{
	    s1_enabled = false;
	}
	else if ((s1_sweep_shift > 0) && is_update)
	{
	    s1_shadow_freq = shadow_freq;
	    s1_freq = shadow_freq;
	    s1_period = ((2048 - s1_freq) * 4);
	}
    }

    void GBAPU::s1Trigger()
    {
	s1_enabled = true;

	s1_period = ((2048 - s1_freq) * 4);

	if (s1_length_counter == 0)
	{
	    s1_length_counter = 64;

	    if (testbit(apu_shift, 0) && s1_length_enabled)
	    {
		s1_length_counter -= 1;
	    }
	}

	s1_volume = s1_env_volume;
	s1_env_freq = (s1_env_period == 0) ? 8 : s1_env_period;
	s1_env_enabled = (s1_env_period != 0);

	s1_shadow_freq = s1_freq;
	s1_sweep_freq = (s1_sweep_period == 0) ? 8 : s1_sweep_period;

	s1_sweep_enabled = ((s1_sweep_period > 0) || (s1_sweep_shift > 0));

	if (s1_sweep_shift > 0)
	{
	    s1SweepCalc(false);
	}

	if (!s1_dac_enabled)
	{
	    s1_enabled = false;
	}
    }

    void GBAPU::s2Trigger()
    {
	s2_enabled = true;

	s2_period = ((2048 - s2_freq) * 4);

	if (s2_length_counter == 0)
	{
	    s2_length_counter = 64;

	    if (testbit(apu_shift, 0) && s2_length_enabled)
	    {
		s2_length_counter -= 1;
	    }
	}

	s2_volume = s2_env_volume;
	s2_env_freq = (s2_env_period == 0) ? 8 : s2_env_period;
	s2_env_enabled = (s2_env_period != 0);

	if (!s2_dac_enabled)
	{
	    s2_enabled = false;
	}
    }

    void GBAPU::waveTrigger()
    {
	wave_enabled = true;

	wave_period = ((2048 - wave_freq) * 2);

	if (wave_length_counter == 0)
	{
	    wave_length_counter = 256;

	    if (testbit(apu_shift, 0) && wave_length_enabled)
	    {
		wave_length_counter -= 1;
	    }
	}

	wave_phase = 0;

	if (!wave_dac_enabled)
	{
	    wave_enabled = false;
	}
    }

    int GBAPU::getNoiseDivisor()
    {
	int clock_div = max((noise_div_code << 1), 1);
	return (clock_div << 2);
    }

    void GBAPU::noiseTrigger()
    {
	noise_enabled = true;

	noise_period = (getNoiseDivisor() << noise_clock_shift);
	noise_lfsr = 0x7FFF;

	if (noise_length_counter == 0)
	{
	    noise_length_counter = 64;

	    if (testbit(apu_shift, 0) && noise_length_enabled)
	    {
		noise_length_counter -= 1;
	    }
	}

	noise_volume = noise_env_volume;
	noise_env_freq = (noise_env_period == 0) ? 8 : noise_env_period;
	noise_env_enabled = (noise_env_period != 0);

	if (!noise_dac_enabled)
	{
	    noise_enabled = false;
	}
    }

    int GBAPU::getS1Output()
    {
	int output = 0;

	if (s1_enabled)
	{
	    bool duty_cycle = square_duty[s1_duty][s1_duty_counter];
	    output = duty_cycle ? s1_volume : 0;
	}

	return output;
    }

    int GBAPU::getS2Output()
    {
	int output = 0;

	if (s2_enabled)
	{
	    bool duty_cycle = square_duty[s2_duty][s2_duty_counter];
	    output = duty_cycle ? s2_volume : 0;
	}

	return output;
    }

    int GBAPU::getWaveOutput()
    {
	int output = 0;

	array<int, 4> vol_table = {4, 0, 1, 2};

	int wave_vol = vol_table.at(wave_vol_code);

	if (wave_enabled)
	{
	    output = (wave_sample >> wave_vol);
	}

	return output;
    }

    int GBAPU::getNoiseOutput()
    {
	int output = 0;

	bool noise_output = testbit(noise_lfsr, 0) ? false : true;

	if (noise_enabled)
	{
	    output = noise_output ? noise_volume : 0;
	}

	return output;
    }

    void GBAPU::clockSweep()
    {
	clockS1Sweep();
    }

    void GBAPU::clockLength()
    {
	clockS1Length();
	clockS2Length();
	clockWaveLength();
	clockNoiseLength();
    }

    void GBAPU::clockEnvelope()
    {
	clockS1Envelope();
	clockS2Envelope();
	clockNoiseEnvelope();
    }

    void GBAPU::clockAudio()
    {
	clockS1Timer();
	clockS2Timer();
	clockWaveTimer();
	clockNoiseTimer();
    }

    void GBAPU::tickAudio()
    {
	apu_counter += 1;

	if (apu_counter == 8192)
	{
	    apu_counter = 0;

	    switch (apu_shift)
	    {
		case 0:
		{
		    clockLength();
		}
		break;
		case 2:
		{
		    clockSweep();
		    clockLength();
		}
		break;
		case 4:
		{
		    clockLength();
		}
		break;
		case 6:
		{
		    clockSweep();
		    clockLength();
		}
		break;
		case 7:
		{
		    clockEnvelope();
		}
		break;
	    }

	    apu_shift += 1;

	    if (apu_shift == 8)
	    {
		apu_shift = 0;
	    }
	}

	clockAudio();
	mixAudio();
    }

    void GBAPU::mixAudio()
    {
	int32_t left_sample = 0;
	int32_t right_sample = 0;
	s1_output = getS1Output();
	s2_output = getS2Output();
	wave_output = getWaveOutput();
	noise_output = getNoiseOutput();

	if (testbit(left_enable, 0))
	{
	    left_sample += s1_output;
	}

	if (testbit(left_enable, 1))
	{
	    left_sample += s2_output;
	}

	if (testbit(left_enable, 2))
	{
	    left_sample += wave_output;
	}

	if (testbit(left_enable, 3))
	{
	    left_sample += noise_output;
	}

	if (testbit(right_enable, 0))
	{
	    right_sample += s1_output;
	}

	if (testbit(right_enable, 1))
	{
	    right_sample += s2_output;
	}

	if (testbit(right_enable, 2))
	{
	    right_sample += wave_output;
	}

	if (testbit(right_enable, 3))
	{
	    right_sample += noise_output;
	}

	left_sample *= (left_vol + 1);
	right_sample *= (right_vol + 1);

	left_sample = clamp<int32_t>((left_sample << 6), -32768, 32767);
	right_sample = clamp<int32_t>((right_sample << 6), -32768, 32767);

	outputSample(left_sample, right_sample);
    }
};