<img src="https://github.com/Buenia0/libmbGB/blob/develop/res/libmbgblogo.png" alt="drawing" width="200"/>

Game Boy (Color) emulation library, sorta

The goal here is to do things right and fast (like melonDS). But, also, you know, to have a fun challenge... :)

# Features

Able to run most commercial Game Boy (and most commercial Game Boy Color) titles with sound

Exclusive "hybrid mode" that combines the Game Boy Color's DMG mode with emulation of some of the Game Boy's DMG-exclusive bugs, including the STAT IRQ bug that Legend of Zerd (aka. Zerd no Densetsu) relies on

And more to come!

# Quickstart

`git clone --recursive -b rewrite https://github.com/Buenia0/libmbGB.git`

`cd libmbGB && source beemake`

`bm build`

`./build/example/sdl2/example some.gb(c)`

# Building

For beginning programmers, a simple build script is included to build the library and its main example project.

`python beemake.py --help`

The beemake bash file has some aliases that save some keypresses.

`source beemake`

`bm  -> python beemake.py`

`bmp  -> python beemake.py pull`

`bmb  -> python beemake.py build`

`bme  -> python beemake.py example`

`bmn  -> python beemake.py nuke`

# Commands

## pull
Does a `git pull` in additin to updating submodules. Use this, if possible, instead of git pull.

`bm pull`

## build

Builds libmbGB with CMake.

`bm build`

## example

Builds both libmbGB and its SDL2 renderer with CMake.

`bm example`

## nuke

Cleans up libmbGB outputs. A full build will be required after this, so only run this command if you want to reclaim your disk space or something goes horribly wrong.

`bm nuke`


# Building Instructions (advanced)

For advanced programmers who want to create a custom build of libmbGB, the following instructions apply:

## Linux:

Step 1: Install dependencies:

`sudo apt-get install git cmake build-essential`

For the SDL2 renderer:

`sudo apt-get install libsdl2-dev`

Step 2: Clone the repository:

`git clone --recursive -b rewrite https://github.com/Buenia0/libmbGB.git`
`cd libmbGB`

Step 3: Compile:

`mkdir build && cd build`
`cmake .. -G "Unix Makefiles" -DBUILD_HEADLESS="<ON/OFF>" -DBUILD_EXAMPLE="<ON/OFF> -DCMAKE_BUILD_TYPE="<Debug/Release""`
`make -j4`

## Mac OS:

You will need [homebrew](https://brew.sh), a recent version of Xcode and the Xcode command-line tools to build libmbGB.
Please note that due to personal financial constraints, libmbGB has not been tested on Mac OS as of yet.

Step 1: Install dependencies:

`brew install git cmake`

For the SDL2 renderer:

`brew install sdl2`

Step 2: Clone the repository:

`git clone --recursive -b rewrite https://github.com/Buenia0/libmbGB.git`
`cd libmbGB`

Step 3: Compile:

`mkdir build && cd build`
`cmake .. -G "Unix Makefiles" -DBUILD_HEADLESS="<ON/OFF>" -DBUILD_EXAMPLE="<ON/OFF> -DCMAKE_BUILD_TYPE="<Debug/Release""`
`make -j4`

## Windows:

You will need [MSYS2](https://msys2.github.io) in order to install libmbGB.
Make sure to run `pacman -Syu` as needed.

Step 1: Install dependencies:

`pacman -S mingw-w64-x86_64-toolchain mingw-w64-x86_64-cmake git`

For the SDL2 renderer:

`pacman -S mingw-w64-x86_64-SDL2`

Step 2: Clone the repository:

`git clone --recursive -b rewrite https://github.com/Buenia0/libmbGB.git`
`cd libmbGB`

Step 3: Compile:

`mkdir build && cd build`
`cmake .. -G "MSYS Makefiles" -DBUILD_HEADLESS="<ON/OFF>" -DBUILD_EXAMPLE="<ON/OFF> -DCMAKE_BUILD_TYPE="<Debug/Release""`
`(mingw32-)make -j4`

# Plans

## Near-term

Improving existing Game Boy (Color) support

Adding Game Boy Advance support

Patching up graphical debugger

Link Cable emulation and netplay support

## Medium-term

Frontends for numerous systems (via mbGB-ports)

Graphical frontend (via mbGB-Qt)

WebAssembly port

## Long-term

Support for various obscure Game Boy (Color) addons (Zok Zok Heroes Full Changer, Barcode Boy, etc.)

# License

<img src="https://www.gnu.org/graphics/gplv3-127x51.png" alt="drawing" width="150"/>

libmbGB is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

# Copyright

(C) 2019 Buenia. This project is not affiliated in any way with Nintendo. Game Boy is a registered trademark of Nintendo Co., Ltd.

For information regarding libmbGB's stance on copyright infringement, see the DISCLAIMER.md file in this repository.
