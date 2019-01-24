# libmbGB [![Build Status](https://travis-ci.org/Buenia0/libmbGB.svg?branch=master)](https://travis-ci.org/Buenia0/libmbGB)

libmbGB is a GameBoy emulation library written in C++11.

A library, you ask? Yes, in fact. And because libmbGB is a dependency-free library, it means that the emulation logic and the GUI logic of an emulator made with this library are seperate. In addition, as long as the target platform has a C++11 compiler, that platform can use libmbGB.

# Examples

The libmbGB repo includes 2 example projects, one of them an SDL2 renderer, the other a headless renderer.

# Building libmbGB

libmbGB does not rely on any additional libraries as dependencies. However, it does require a C++11 compiler and CMake. 

## Building on OSX & Linux:

First, fetch the source:

`git clone --recursive https://github.com/Buenia0/libmbGB.git`

`cd libmbGB`


To build the library, simply run the following commands:

`mkdir build && cd build`

`cmake .. -G "Unix Makefiles -DBUILD_EXAMPLE="<ON/OFF> -DBUILD_HEADLESS="<ON/OFF>"`

`make -j4`


## Building on Windows:

To build libmbGB on Windows, MSYS2 is recomended. Follow the installation steps found on their [website.](http://www.msys2.org/)
Make sure you're running the 32-bit version (mingw32.exe) for 32-bit builds or the 64-bit version (mingw64.exe) for 64-bit builds and then run this additional command to install the needed dependencies:

For 32-bit builds:

`pacman -S mingw-w64-i686-toolchain mingw-w64-i686-cmake git`


For 64-bit builds:

`pacman -S mingw-w64-x86_64-toolchain mingw-w64-x86_64-cmake git`


Then fetch the source:

`git clone --recursive https://github.com/Buenia0/libmbGB.git`

`cd libmbGB`


To build the library, simply run the following commands:

`mkdir build && cd build`

`cmake .. -G "MSYS Makefiles -DBUILD_EXAMPLE="<ON/OFF> -DBUILD_HEADLESS="<ON/OFF>"`

`mingw32-make -j4`


## Notes for Building

Building the SDL2 renderer requires SDL2 to be findable by CMake.

## Development

Most of the development happens on Github. It's also where libmbGB's central repository is located.

If you want to contribute to libmbGB, please take a look at the [Contributor's Guide](https://github.com/Buenia0/libmbGB/blob/master/CONTRIBUTING.md).

## Legal Disclaimers

This project is licensed under the GNU GPL v3. For more information, see the included LICENSE file.

(C) 2019 Buenia. This project is not affiliated in any way with Nintendo. Game Boy is a registered trademark of Nintendo Co., Ltd.






