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

#include "input.h"
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
	reg_joyp = 0xCF;
	button_states = 0xFF;
    }

    void GBInput::shutdown()
    {
	return;
    }

    uint8_t GBInput::readIO()
    {
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

    void GBInput::updateJoypad()
    {
	uint8_t control_bits = 0xF;

	if (!testbit(reg_joyp, 5))
	{
	    control_bits = changebit(control_bits, 0, testbit(button_states, ButtonA));
	    control_bits = changebit(control_bits, 1, testbit(button_states, ButtonB));
	    control_bits = changebit(control_bits, 2, testbit(button_states, Select));
	    control_bits = changebit(control_bits, 3, testbit(button_states, Start));
	}
	else if (!testbit(reg_joyp, 4))
	{
	    control_bits = changebit(control_bits, 0, testbit(button_states, Right));
	    control_bits = changebit(control_bits, 1, testbit(button_states, Left));
	    control_bits = changebit(control_bits, 2, testbit(button_states, Up));
	    control_bits = changebit(control_bits, 3, testbit(button_states, Down));
	}

	reg_joyp = ((reg_joyp & 0xF0) | control_bits);
    }
};