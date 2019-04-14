<img src="https://github.com/Buenia0/libmbGB/blob/develop/res/libmbgblogo.png" alt="drawing" width="200"/>

[![Build Status](https://travis-ci.org/Buenia0/libmbGB.svg?branch=master)](https://travis-ci.org/Buenia0/libmbGB)

Gameboy emulation library, sorta

The goal here is to do things right and fast (like melonDS). But, also, you know, to have a fun challenge... :)

## Building Instructions

## Mac and Linux:

1. Install dependencies (optional step for example SDL2 renderers and optional SDL2 audio backend):

On Linux:

`sudo apt install libsdl2-dev`

On Mac:

`brew install sdl2`

2. Fetch the source:

`git clone --recursive https://github.com/Buenia0/libmbGB.git`

`cd libmbGB`

3. Compile:

`mkdir build && cd build`

For the embeddable build:

`cmake .. -G "Unix Makefiles" -DBUILD_EMBED_EXAMPLE="ON"`

`make -j4`

For all other builds:

`cmake .. -G "Unix Makefiles" -DBUILD_EXAMPLE="<ON/OFF>" -DBUILD_HEADLESS="<ON/OFF>" -DENABLE_AUDIO="<ON/OFF>" -DSDL2_AUDIO="<ON/OFF>"`

`make -j4`

## Windows:

Requires MSYS2, which can be installed from their [website.] (http://www.msys2.org/)

1. Install dependencies:

For 32-bit builds:

`pacman -S mingw-w64-i686-toolchain mingw-w64-i686-cmake "mingw-w64-i686-SDL2" git`

For 64-bit builds:

`pacman -S mingw-w64-x86_64-toolchain mingw-w64-x86_64-cmake "mingw-w64-x86_64-SDL2" git `

The packages in quotation marks are for the example SDL2 renderers and optional SDL2 audio backend.

2. Fetch the source:

`git clone --recursive https://github.com/Buenia0/libmbGB.git`

`cd libmbGB`

3. Compile:

`mkdir build && cd build`

For the embeddable build:

`cmake .. -G "MSYS Makefiles" -DBUILD_EMBED_EXAMPLE="ON"`

`mingw32-make -j4`

For all other builds:

`cmake .. -G "MSYS Makefiles" -DBUILD_EXAMPLE="<ON/OFF>" -DBUILD_HEADLESS="<ON/OFF>" -DENABLE_AUDIO="<ON/OFF>" -DSDL2_AUDIO="<ON/OFF>"`

`mingw32-make -j4`

# Plans

## Near-term

Various bugfixes

## Medium-term

GameBoy Color support

## Long-term

Link Cable support


# License

<img src="https://www.gnu.org/graphics/gplv3-127x51.png" alt="drawing" width="150"/>

libmbGB is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

# Copyright

(C) 2019 Buenia. This project is not affiliated in any way with Nintendo. Game Boy is a registered trademark of Nintendo Co., Ltd.

For information regarding libmbGB's stance on copyright infringement, see the DISCLAIMER.md file in this repository.
