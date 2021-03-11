<img src="https://github.com/Buenia0/libmbGB/blob/develop/res/libmbgblogo.png" alt="drawing" width="200"/>

[![Run on Repl.it](https://repl.it/badge/github/Buenia0/libmbGB)](https://repl.it/github/Buenia0/libmbGB)

Game Boy (Color) emulation library, sorta

The goal here is to do things right and fast (like melonDS). But, also, you know, to have a fun challenge... :)

# Features

Able to run most commercial Game Boy (and most commercial Game Boy Color) titles with sound

Game Boy Camera, Game Boy Printer and Barcode Boy support

Exclusive "hybrid mode" that combines the Game Boy Color's DMG mode with emulation of some of the Game Boy's DMG-exclusive bugs, including the STAT IRQ bug that Legend of Zerd (aka. Zerd no Densetsu) relies on

And more to come!

# Building Instructions

The libmbGB library does not have any dependencies and can be compiled with MinGW on Windows, and both GCC and Clang on Linux and (presumably) OSX. The examples contained in this repo, however, do have additional dependencies that need to be installed. All dependencies should be findable by CMake.

## Linux:

Step 1: Install dependencies:

Core dependencies:

Compiler: GCC or Clang. You only need one of those two:

GCC 10.2.0+ (earlier versions not tested):

Debian: `sudo apt-get install build-essential`
Arch (not tested): `sudo pacman -S base-devel`
Fedora: `sudo dnf install gcc-c++`
OpenSUSE (not tested): `sudo zypper in gcc-c++`

Clang (not tested):

Debian: `sudo apt-get install clang clang-format libc++-dev` (in some distros, clang-5.0)
Arch: `pacman -S clang`, `libc++` is in the AUR. Use pacaur or yaourt to install it.
Fedora: `dnf install clang libcxx-devel`
OpenSUSE: `zypper in clang`

Git (if not installed already) and CMake 3.5+:

Debian: `sudo apt-get install git cmake`
Arch (not tested): `sudo pacman -S git`
Fedora: `sudo dnf install git cmake`
OpenSUSE (not tested): `sudo zypper in git cmake extra-cmake-modules`

For the SDL2 renderer:

Debian: `sudo apt-get install libsdl2-dev`
Arch (not tested): `sudo pacman -S sdl2`
Fedora: `sudo dnf install SDL2-devel`
OpenSUSE: (not tested): `sudo zypper in libSDL2-devel`

For the (WIP) Qt5 frontend:

Debian: `sudo apt-get install qtbase5-dev qtmultimedia5-dev`
Arch (not tested): `sudo pacman -S qt5`
Fedora: `sudo dnf install qt5-qtmultimedia-devel`
OpenSUSE (not tested): `sudo zypper in libQt5Multimedia5 libqt5-qtmultimedia-devel libQt5Concurrent-devel`

For the (optional) Game Boy Camera support:

Debian (not tested): `sudo apt-get install libopencv-dev`
Arch (not tested): `sudo pacman -S opencv`
Fedora (not tested): `sudo dnf install opencv opencv-devel`
OpenSUSE (not tested): `sudo zypper in opencv opencv-devel`

For the (also optional) OpenGL rendering backend:

Debian (not tested): `sudo apt-get install libglu1-mesa-dev mesa-common-dev`
Arch (not tested): `sudo apt-get install mesa-libgl`
Fedora (not tested): `sudo dnf install mesa-libglapi` (if not already installed)
OpenSUSE (not tested): `sudo zypper in mesa-libgl-devel`

Step 2: Clone the repository:

`git clone --recursive https://github.com/Buenia0/libmbGB.git`

`cd libmbGB`

Step 3: Compile:

`mkdir build && cd build`

`cmake .. -G "Unix Makefiles" -DBUILD_HEADLESS="<ON/OFF>" -DBUILD_EXAMPLE="<ON/OFF>" -DBUILD_QT5="<ON/OFF>" -DCMAKE_BUILD_TYPE="<Debug/Release>""`

`make -j$(nproc --all)`

## Mac OS (not tested):

You will need [homebrew](https://brew.sh), a recent version of Xcode and the Xcode command-line tools to build libmbGB.
Please note that due to personal financial constraints, libmbGB has not been tested on Mac OS as of yet.

Step 1: Install dependencies:

`brew install git cmake pkg-config`

For the SDL2 renderer:

`brew install sdl2`

For the (WIP) Qt5 frontend:

`brew install qt5`

Note: If you have Qt4 installed, then you will need to remove it before building with `brew unlink qt4`.

Step 2: Clone the repository:

`git clone --recursive https://github.com/Buenia0/libmbGB.git`

`cd libmbGB`

Step 3: Tell CMake where your Qt5 is installed (add this line to ~/.profile if you want to make this permanent):

`export Qt5_DIR=$(brew --prefix)/opt/qt5

Step 3: Compile:

`mkdir build && cd build`

`cmake .. -G "Unix Makefiles" -DBUILD_HEADLESS="<ON/OFF>" -DBUILD_EXAMPLE="<ON/OFF>" -DBUILD_QT5="<ON/OFF>" -DCMAKE_BUILD_TYPE="<Debug/Release>""`

`make -j$(nproc --all)`

## Windows:

You will need [MSYS2](https://msys2.github.io) in order to install libmbGB.
Make sure to run `pacman -Syu` as needed.

Step 1: Install dependencies:

`pacman -S base-devel mingw-w64-x86_64-toolchain mingw-w64-x86_64-cmake git`

For the SDL2 renderer:

`pacman -S mingw-w64-x86_64-SDL2`

For the (WIP) Qt5 frontend:

`pacman -S mingw-w64-x86_64-qt5`

For the (optional) Game Boy Camera support:

`pacman -S mingw-w64-x86_64-opencv`

For the (also optional) OpenGL rendering backend:

`pacman -S mingw-w64-x86_64-glew`

Step 2: Clone the repository:

`git clone --recursive https://github.com/Buenia0/libmbGB.git`

`cd libmbGB`

Step 3: Compile:

`mkdir build && cd build`

`cmake .. -G "MSYS Makefiles" -DBUILD_HEADLESS="<ON/OFF>" -DBUILD_EXAMPLE="<ON/OFF>" -DBUILD_WEBCAM="<ON/OFF>" -DBUILD_QT5="<ON/OFF>" -DCMAKE_BUILD_TYPE="<Debug/Release>""`

`(mingw32-)make -j$(nproc --all)`

`../msys-dist.sh`

# Known issues

# MBC1-based multicart games

MBC1 multicarts, such as Mortal Kombat 1/2 and Bomberman Collection, as well as a few others, require the command line argument of "--mbc1m" passed to libmbGB in order to run properly.

# Plans

## Near-term

Improving existing Game Boy (Color) support

Patching up graphical debugger

GB Printer and Mobile Adapter GB support

Graphical frontend (mbGB-Qt)

## Medium-term

Frontends for numerous systems (via mbGB-ports)

Linux GUI (via mbGB-Gtk)

Android frontend (via mbGB-droid)

WebAssembly port (via mbGB-wasm)

## Long-term

Support for various obscure Game Boy (Color) addons (Zok Zok Heroes Full Changer, Turbo File GB, etc.)

# License

<img src="https://www.gnu.org/graphics/gplv3-127x51.png" alt="drawing" width="150"/>

libmbGB is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

# Copyright

(C) 2020 Buenia. This project is not affiliated in any way with Nintendo. Game Boy and Game Boy Color are registered trademarks of Nintendo Co., Ltd.

For information regarding libmbGB's stance on copyright infringement, as well as proof of libmbGB's legality, see the LEGAL.md file in this repository.
