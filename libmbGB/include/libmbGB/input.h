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

#ifndef LIBMBGB_INPUT_H
#define LIBMBGB_INPUT_H

#include "utils.h"
using namespace gb;

namespace gb
{
    class LIBMBGB_API GBInput
    {
	public:
	    GBInput();
	    ~GBInput();

	    void init();
	    void shutdown();

	    uint8_t readIO();
	    void writeIO(uint8_t data);

	    void keyChanged(GBButton button, bool is_pressed);

	private:
	    int button_states = 0;
	    uint8_t reg_joyp = 0;

	    void updateJoypad();
    };
};

#endif // LIBMBGB_INPUT_H