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

// TODO: Rewrite input system to allow "Telling LYs?" test ROM to pass

#include "input.h"
#include <cstdlib>
using namespace gb;
using namespace std;

namespace gb
{
    GBInput::GBInput()
    {

    }

    GBInput::~GBInput()
    {

    }

    void GBInput::init()
    {
	joypad_counter = 0;
	reg_joyp = 0xCF;
	button_states = 0xFF;
    }

    void GBInput::shutdown()
    {
	return;
    }

    uint8_t GBInput::readIO()
    {
	fireJoypadCallback(false);
	return (reg_joyp | 0xC0);
    }

    void GBInput::writeIO(uint8_t data)
    {
	reg_joyp = ((reg_joyp & 0xF) | (data & 0x30));
	updateJoypad();
    }

    void GBInput::keyChanged(GBButton button, bool is_pressed)
    {
	button_states = changebit(button_states, button, !is_pressed);
    }

    void GBInput::tickJoypad()
    {
	joypad_counter += 1;
	updateJoypad();
    }

    void GBInput::fireJoypadCallback(bool wait)
    {
	if (!wait || ((joypad_counter - last_ticks) >= 10000))
	{
	    keypad_states = button_states;
	    last_ticks = joypad_counter;
	}
    }

    void GBInput::updateJoypad()
    {
	fireJoypadCallback(true);
	reg_joyp |= 0xF;

	if (!testbit(reg_joyp, 5))
	{
	    reg_joyp = ((reg_joyp & 0xF0) | (keypad_states >> 4));
	}

	if (!testbit(reg_joyp, 4))
	{
	    reg_joyp = ((reg_joyp & 0xF0) | (keypad_states & reg_joyp));
	}

	bool irq_signal = ((reg_joyp & 0xF) == 0xF);
	bool signal_low = (!irq_signal && prev_irq_signal);

	if (signal_low)
	{
	    fireJoypadIRQ();
	}

	prev_irq_signal = irq_signal;
    }
};