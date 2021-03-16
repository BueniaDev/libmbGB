// This file is part of libmbGB.
// Copyright (C) 2021 Buenia.
//
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

#include "../../include/libmbGB/input.h"
using namespace gb;
using namespace std;
using namespace std::placeholders;

namespace gb
{
    Input::Input(MMU& memory) : p1mem(memory)
    {
	p1mem.addmemoryreadhandler(0xFF00, bind(&Input::readjoypad, this, _1));
	p1mem.addmemorywritehandler(0xFF00, bind(&Input::writejoypad, this, _1, _2));
    }

    Input::~Input()
    {
		
    }

    void Input::init()
    {
	if (p1mem.isdmgmode() && p1mem.isdmgconsole())
	{
	    joypad = 0xCF;
	}
	else
	{
	    joypad = 0xFF;
	}
		
	cout << "Input::Initialized" << endl;
    }

    void Input::shutdown()
    {
	cout << "Input::Shutting down..." << endl;
    }

    void Input::updatejoypad()
    {
	uint8_t controlbits = 0;

	if (buttonkeysselected())
	{
	    controlbits |= ((TestBit(buttonstates, gbButton::A) ? 1 : 0) << 0);
	    controlbits |= ((TestBit(buttonstates, gbButton::B) ? 1 : 0) << 1);
	    controlbits |= ((TestBit(buttonstates, gbButton::Select) ? 1 : 0) << 2);
	    controlbits |= ((TestBit(buttonstates, gbButton::Start) ? 1 : 0) << 3);
	}
	else if (directionkeysselected())
	{
	    controlbits |= ((TestBit(buttonstates, gbButton::Right) ? 1 : 0) << 0);
	    controlbits |= ((TestBit(buttonstates, gbButton::Left) ? 1 : 0) << 1);
	    controlbits |= ((TestBit(buttonstates, gbButton::Up) ? 1 : 0) << 2);
	    controlbits |= ((TestBit(buttonstates, gbButton::Down) ? 1 : 0) << 3);
	}
	else
	{
	    controlbits = 0xF;
	}
	
	joypad &= 0xF0;
	joypad |= controlbits;
	joypad |= 0xC0;

	interruptsignal = joypadpress();

	bool signalwentlow = (interruptsignal && previnterruptsignal);

	if (signalwentlow)
	{
	    p1mem.requestinterrupt(4);
	}

	previnterruptsignal = interruptsignal;
    }
}
