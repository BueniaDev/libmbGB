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

#ifndef LIBMBGB_APU_H
#define LIBMBGB_APU_H

#include "utils.h"
using namespace gb;

namespace gb
{
    class LIBMBGB_API GBAPU
    {
	public:
	    GBAPU();
	    ~GBAPU();

	    void init();
	    void shutdown();

	    void tickAudio();

	    uint8_t readIO(uint16_t addr);
	    void writeIO(uint16_t addr, uint8_t data);

	    uint8_t readCGBIO(uint16_t addr);

	    void setOutputCallback(apufunc cb)
	    {
		apu_func = cb;
	    }

	private:

	    void clockSweep();
	    void clockEnvelope();
	    void clockLength();
	    void clockAudio();

	    int s1_sweep_period = 0;
	    bool s1_sweep_negate = false;
	    int s1_sweep_shift = 0;

	    int getS1Output();
	    void clockS1Sweep();
	    void clockS1Envelope();
	    void clockS1Length();
	    void clockS1Timer();
	    void s1Trigger();
	    void s1SweepCalc(bool is_update);

	    int s1_duty = 0;
	    int s1_duty_counter = 0;
	    int s1_length_load = 0;
	    int s1_length_counter = 0;
	    uint16_t s1_freq = 0;
	    uint16_t s1_shadow_freq = 0;
	    uint16_t s1_period = 0;
	    uint16_t s1_sweep_freq = 0;

	    int s1_env_period = 0;
	    int s1_env_freq = 0;
	    int s1_env_volume = 0;
	    bool s1_env_add = false;
	    int s1_volume = 0;

	    bool s1_sweep_enabled = false;
	    bool s1_length_enabled = false;
	    bool s1_dac_enabled = false;
	    bool s1_env_enabled = false;
	    bool s1_enabled = false;

	    int getS2Output();
	    void clockS2Envelope();
	    void clockS2Length();
	    void clockS2Timer();
	    void s2Trigger();

	    int s2_duty = 0;
	    int s2_duty_counter = 0;
	    int s2_length_load = 0;
	    int s2_length_counter = 0;
	    uint16_t s2_freq = 0;
	    uint16_t s2_period = 0;

	    int s2_env_period = 0;
	    int s2_env_freq = 0;
	    int s2_env_volume = 0;
	    bool s2_env_add = false;
	    int s2_volume = 0;

	    bool s2_length_enabled = false;
	    bool s2_dac_enabled = false;
	    bool s2_env_enabled = false;
	    bool s2_enabled = false;

	    bool wave_dac_enabled = false;
	    int wave_length_load = 0;
	    int wave_length_counter = 0;
	    int wave_vol_code = 0;
	    int wave_phase = 0;
	    uint16_t wave_freq = 0;
	    uint16_t wave_period = 0;

	    array<uint8_t, 16> wave_ram;

	    int wave_sample = 0;

	    bool wave_length_enabled = false;
	    bool wave_enabled = false;

	    int getWaveOutput();
	    void clockWaveLength();
	    void clockWaveTimer();
	    void waveTrigger();

	    int noise_length_load = 0;
	    int noise_length_counter = 0;

	    int noise_env_period = 0;
	    int noise_env_freq = 0;
	    int noise_env_volume = 0;
	    bool noise_env_add = false;
	    int noise_volume = 0;
	    int noise_period = 0;

	    int noise_clock_shift = 0;
	    bool noise_width_mode = false;
	    int noise_div_code = 0;

	    bool noise_length_enabled = false;
	    bool noise_dac_enabled = false;
	    bool noise_env_enabled = false;
	    bool noise_enabled = false;

	    uint16_t noise_lfsr = 0;

	    int getNoiseDivisor();
	    int getNoiseOutput();
	    void clockNoiseEnvelope();
	    void clockNoiseLength();
	    void clockNoiseTimer();
	    void noiseTrigger();

	    int left_vol = 0;
	    int right_vol = 0;

	    bool vin_mix_left = false;
	    bool vin_mix_right = false;

	    int s1_output = 0;
	    int s2_output = 0;
	    int wave_output = 0;
	    int noise_output = 0;

	    int left_enable = 0;
	    int right_enable = 0;

	    bool sound_enable = false;

	    void mixAudio();

	    apufunc apu_func;

	    void audioOutput(int16_t left, int16_t right)
	    {
		if (apu_func)
		{
		    apu_func(left, right);
		}
	    }

	    void outputSample(int16_t left, int16_t right)
	    {
		while (resample_phase < 1.0)
		{
		    audioOutput(left, right);
		    resample_phase += sample_ratio;
		}

		resample_phase -= 1.0f;
	    }

	    void setSampleRate()
	    {
		sample_ratio = (4194304.f / 48000.f);
	    }

	    float sample_ratio = 1.0;
	    float resample_phase = 0.0;

	    uint8_t readSquare1(int reg);
	    void writeSquare1(int reg, uint8_t data);

	    uint8_t readSquare2(int reg);
	    void writeSquare2(int reg, uint8_t data);

	    uint8_t readWave(int reg);
	    void writeWave(int reg, uint8_t data);

	    uint8_t readNoise(int reg);
	    void writeNoise(int reg, uint8_t data);

	    uint8_t readNR50()
	    {
		uint8_t data = 0;
		data |= right_vol;
		data |= (vin_mix_right << 3);
		data |= (left_vol << 4);
		data |= (vin_mix_left << 7);
		return data;
	    }

	    void writeNR50(uint8_t data)
	    {
		left_vol = ((data >> 4) & 0x7);
		vin_mix_left = testbit(data, 7);
		right_vol = (data & 0x7);
		vin_mix_right = testbit(data, 3);
	    }

	    uint8_t readNR51()
	    {
		uint8_t data = 0;
		data |= right_enable;
		data |= (left_enable << 4);
		return data;
	    }

	    void writeNR51(uint8_t data)
	    {
		left_enable = (data >> 4);
		right_enable = (data & 0xF);
	    }

	    uint8_t readNR52()
	    {
		uint8_t data = 0;
		data = (sound_enable << 7);
		data |= (noise_enabled << 3);
		data |= (wave_enabled << 2);
		data |= (s2_enabled << 1);
		data |= s1_enabled;
		return data;
	    }

	    void writeNR52(uint8_t data)
	    {
		bool prev_sound_enable = sound_enable;
		sound_enable = testbit(data, 7);

		if (prev_sound_enable && !sound_enable)
		{
		    disableAllSound();
		}
	    }

	    void disableAllSound()
	    {
		for (int i = 0; i < 5; i++)
		{
		    writeSquare1(i, 0);
		    writeWave(i, 0);
		}

		for (int i = 0; i < 4; i++)
		{
		    writeSquare2(i, 0);
		    writeNoise(i, 0);
		}

		writeNR50(0);
		writeNR51(0);

		s1_enabled = false;
		s2_enabled = false;
		wave_enabled = false;
		noise_enabled = false;
	    }

	    int apu_counter = 0;
	    int apu_shift = 0;


	    array<array<bool, 8>, 4> square_duty = 
	    {
		false, false, false, false, false, false, false, true,
		true, false, false, false, false, false, false, true,
		true, false, false, false, false, true, true, true,
		false, true, true, true, true, true, true, false
	    };

	    array<uint8_t, 23> apu_masks = 
	    {
		0x80, 0x3F, 0x00, 0xFF, 0xBF,
		0xFF, 0x3F, 0x00, 0xFF, 0xBF,
		0x7F, 0xFF, 0x9F, 0xFF, 0xBF,
		0xFF, 0xFF, 0x00, 0x00, 0xBF,
		0x00, 0x00, 0x70
	    };

	    array<uint8_t, 23> init_values =
	    {
		0x80, 0xBF, 0xF3, 0xFF, 0xBF,
		0x00, 0x3F, 0x00, 0xFF, 0xBF,
		0x7F, 0xFF, 0x9F, 0xFF, 0xBF,
		0x00, 0xFF, 0x00, 0x00, 0xBF,
		0x77, 0xF3, 0xF1
	    };
    };
};



#endif // LIBMBGB_APU_H