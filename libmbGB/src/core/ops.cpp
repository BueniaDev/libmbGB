// This file is part of libmbGB.
// Copyright (C) 2019 Buenia.ui
// libmbGB is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// libmbGB is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with libmbGB.  If not, see <https://www.gnu.org/licenses/>.

#include "../../include/libmbGB/cpu.h"
using namespace gb;

namespace gb
{
    int CPU::executenextopcode(uint8_t opcode)
    {
	int cycles = 0;	

	switch (opcode)
	{
	    case 0x31: sp = getimmword(); pc += 2; cycles = 12; break;
	    default: cout << "Unrecognized opcode at 0x" << hex << (int)(opcode) << endl; exit(1); break;
	}

	return cycles;
    }

    int CPU::executenextcbopcode(uint8_t opcode)
    {
	return 0;
    }
}