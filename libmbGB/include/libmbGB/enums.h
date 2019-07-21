// This file is part of libmbGB.
// Copyright (C) 2019 Buenia.
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

#ifndef LIBMBGB_ENUMS
#define LIBMBGB_ENUMS

enum class Console {Default, DMG, CGB};
enum class Mode {Default, DMG, CGB};
enum class MBCType {None, MBC1, MBC2, MBC3, MBC5};
enum CPUState : uint8_t
{
    Running = 0, 
    Stopped = 1, 
    Halted = 2, 
    HaltBug = 3,
};

#endif // LIBMBGB_ENUMS
