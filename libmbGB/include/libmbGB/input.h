// This file is part of libmbGB.
// Copyright (C) 2020 Buenia.
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

#ifndef LIBMBGB_INPUT
#define LIBMBGB_INPUT

#include "mmu.h"
#include "libmbgb_api.h"
#include <functional>
using namespace gb;
using namespace std;

namespace gb
{
    enum Button : int
    {
	Right = 0,
	Left = 1,
	Up = 2,
	Down = 3,
	A = 4,
	B = 5,
	Select = 6,
	Start = 7
    };

    class LIBMBGB_API Input
    {
	public:
	    Input(MMU& memory);
	    ~Input();

	    MMU& p1mem;

	    void init();
	    void shutdown();

	    void updatejoypad();
		
	    uint8_t joypad = 0;
		
	    uint8_t readjoypad(uint16_t addr)
	    {
		return (joypad | 0xC0);
	    }
		
	    void writejoypad(uint16_t addr, uint8_t value)
	    {
		joypad = ((joypad & 0x0F) | (value & 0x30));
		updatejoypad();
	    }

	    inline void keypressed(Button button)
	    {
		buttonstates = BitReset(buttonstates, button);
	    }

	    inline void keyreleased(Button button)
	    {
		buttonstates = BitSet(buttonstates, button);
	    }

	    uint8_t buttonstates = 0xFF;
	    bool wasunset = false;

	    inline bool joypadpress()
	    {
		return ((joypad & 0x0F) != 0x0F);
	    }

	    bool interruptsignal = false;
	    bool previnterruptsignal = false;

	    inline bool buttonkeysselected()
	    {
		return !TestBit(joypad, 5);
	    }

	    inline bool directionkeysselected()
	    {
		return !TestBit(joypad, 4);
	    }
    };
};

#endif // LIBMBGB_INPUT
