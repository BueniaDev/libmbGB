<img src="https://github.com/Buenia0/libmbGB/blob/main/res/libmbgblogo.png" alt="drawing" width="200"/>

[![Run on Repl.it](https://repl.it/badge/github/Buenia0/libmbGB)](https://repl.it/github/Buenia0/libmbGB)

Game Boy (Color) emulation library, sorta

The goal here is to do things right and fast (like melonDS). But, also, you know, to have a fun challenge... :)

# Features

Able to run most commercial Game Boy (and most commercial Game Boy Color) titles with sound

Game Boy Camera, Game Boy Printer and Barcode Boy support

Exclusive "hybrid mode" that combines the Game Boy Color's DMG mode with emulation of some of the Game Boy's DMG-exclusive bugs, including the STAT IRQ bug that Legend of Zerd (aka. Zerd no Densetsu) relies on

And more to come!

# Building Instructions

The libmbGB library does not have any dependencies and can be compiled with MinGW on Windows, and both GCC and Clang on Linux, as well as AppleClang on OSX. The examples contained in this repo, however, do have additional dependencies that need to be installed. All dependencies should be findable by CMake.

## Linux:

Step 1: Install dependencies:

Core dependencies:

* Compiler: GCC or Clang. You only need one of those two:

    * GCC 10.3.0+ (earlier versions not tested):

        * Debian (not tested): `sudo apt-get install build-essential`

        * Arch (not tested): `sudo pacman -S base-devel`

        * Fedora (not tested): `sudo dnf install gcc-c++`

        * OpenSUSE (not tested): `sudo zypper in gcc-c++`

    * Clang (not tested):

        * Debian: `sudo apt-get install clang clang-format libc++-dev` (in some distros, clang-5.0)

        * Arch: `pacman -S clang`, `libc++` is in the AUR. Use pacaur or yaourt to install it.

        * Fedora: `dnf install clang libcxx-devel`

        * OpenSUSE: `zypper in clang`

* Git (if not installed already) and CMake 3.15+:

    * Debian (not tested): `sudo apt-get install git cmake`

    * Arch (not tested): `sudo pacman -S git cmake`

    * Fedora (not tested): `sudo dnf install git cmake`

    * OpenSUSE (not tested): `sudo zypper in git cmake extra-cmake-modules`

* Optional dependencies:

    * SDL2 (for the command-line frontend (and audio in the (currently WIP) Qt frontend)):

        * Debian (not tested): `sudo apt-get install libsdl2-dev`

        * Arch (not tested): `sudo pacman -S sdl2`

        * Fedora (not tested): `sudo dnf install SDL2-devel`

        * OpenSUSE: (not tested): `sudo zypper in libSDL2-devel`

    * Qt5 or Qt6 (for the (currently WIP) Qt frontend):
        * Not throughly tested yet, so certain versions might or might not work. 

	* In addition, the below commands will install packages for Qt5. Consult your distro to see which Qt6 packages, if any, are available.

        * Debian (not tested): `sudo apt-get install qtbase5-dev qtmultimedia5-dev`

        * Arch (not tested): `sudo pacman -S qt5`

        * Fedora (not tested): `sudo dnf install qt5-qtmultimedia-devel`

        * OpenSUSE (not tested): `sudo zypper in libQt5Multimedia5 libqt5-qtmultimedia-devel libQt5Concurrent-devel`

    * OpenCV (for Game Boy Camera support in mbGB-SDL2):

        * Debian (not tested): `sudo apt-get install libopencv-dev`

        * Arch (not tested): `sudo pacman -S opencv`

        * Fedora (not tested): `sudo dnf install opencv opencv-devel`

        * OpenSUSE (not tested): `sudo zypper in opencv opencv-devel`

    * OpenGL (for hardware-accelerated OpenGL rendering in mbGB-SDL2):

        * Debian (not tested): `sudo apt-get install libglu1-mesa-dev mesa-common-dev`

        * Arch (not tested): `sudo apt-get install mesa-libgl`

        * Fedora (not tested): `sudo dnf install mesa-libglapi` (if not already installed)

        * OpenSUSE (not tested): `sudo zypper in mesa-libgl-devel`

Step 2: Clone the repository:

```
git clone --recursive https://github.com/BueniaDev/libmbGB.git
cd libmbGB
```

Step 3: Compile:

```
mkdir build && cd build
cmake .. -G "Unix Makefiles" -DBUILD_EXAMPLES="<ON/OFF>" -DBUILD_SDL2="<ON/OFF>" -DBUILD_QT="<ON/OFF>" -DBUILD_QT6="<ON/OFF>" -DCMAKE_BUILD_TYPE="<Debug/Release>"
make -j$(nproc --all)
```

## Mac OS (not tested):

You will need [homebrew](https://brew.sh), a recent version of Xcode and the Xcode command-line tools to build libmbGB.
Please note that due to personal financial constraints, libmbGB has not been tested on Mac OS as of yet.

Step 1: Install dependencies:

* pkg-config: `brew install pkgconfig`

* SDL2: `brew install sdl2`

* Qt5 (Qt6 not tested): `brew install qt5` (**Note**: If you have Qt4 installed, then you will need to remove it before building with `brew unlink qt4`.)

* CMake: `brew install cmake`

Step 2: Clone the repository:
```
git clone --recursive https://github.com/BueniaDev/libmbGB.git
cd libmbGB
```

Step 3: Tell CMake where your Qt5 is installed (add this line to ~/.profile if you want to make this permanent):

`export Qt5_DIR=$(brew --prefix)/opt/qt5`

If you choose not to build the Qt5 frontend, skip this step and proceed to step 4.

Step 4: Compile:

```
mkdir build && cd build
cmake .. -G "Unix Makefiles" -DBUILD_EXAMPLES="<ON/OFF>" -DBUILD_SDL2="<ON/OFF>" -DBUILD_QT="<ON/OFF>" -DCMAKE_BUILD_TYPE="<Debug/Release>""
make -j$(sysctl -n hw.ncpu)
```

## Windows:

You will need [MSYS2](https://msys2.github.io) in order to install libmbGB.
Make sure to run `pacman -Syu` as needed.

Step 1: Install dependencies:

`pacman -Sy --needed base-devel git ${MINGW_PACKAGE_PREFIX}-{cmake,glew,opencv,qt5,SDL2,toolchain}`

Step 2: Clone the repository:

```
git clone --recursive https://github.com/BueniaDev/libmbGB.git
cd libmbGB
```

Step 3: Compile:

```
mkdir build && cd build
cmake .. -G "MSYS Makefiles" -DBUILD_EXAMPLES="<ON/OFF>" -DBUILD_SDL2="<ON/OFF>" -DBUILD_WEBCAM="<ON/OFF>" -DBUILD_OPENGL="<ON/OFF>" -DBUILD_QT="<ON/OFF>" -DCMAKE_BUILD_TYPE="<Debug/Release>"
make -j$(nproc --all)
../tools/msys-dist.sh
```

# Known issues

## MBC1-based multicart games

MBC1 multicarts, such as Mortal Kombat 1/2 and Bomberman Collection, as well as a few others, require the command line argument of "--mbc1m" passed to libmbGB in order to run properly.

However, a fix for this issue is currently in the works.

# Plans

## Near-term

* Improving existing Game Boy (Color) support
* Improve GB Printer and Mobile Adapter GB support
* Finish up graphical frontend (mbGB-Qt)

## Medium-term

* Frontends for numerous systems (via mbGB-ports)
* Add graphical debugger to Qt frontend
* Android frontend (via mbGB-droid)
* WebAssembly port (via mbGB-wasm)

## Long-term

* Add support for various obscure Game Boy (Color) addons (Zok Zok Heroes Full Changer, Turbo File GB, etc.)
* Linux GUI (via mbGB-Gtk)
* Any feature you can ask for that isn't ouright stupid

# License

<img src="https://www.gnu.org/graphics/gplv3-127x51.png" alt="drawing" width="150"/>

libmbGB is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

# Copyright

(C) 2021 BueniaDev. This project is not affiliated in any way with Nintendo. Game Boy and Game Boy Color are registered trademarks of Nintendo Co., Ltd.

For information regarding libmbGB's stance on copyright infringement, as well as proof of libmbGB's legality, see the LEGAL.md file in this repository.
