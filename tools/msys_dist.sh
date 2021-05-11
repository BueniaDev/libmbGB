#!/bin/bash

if [[ ! -f "libmbGB.a" ]]; then
	echo "Run this script from the directory where you built libmbGB."
	exit 1
fi

mkdir -p dist

if [ -d "mbGB-SDL2" ]; then
	for lib in $(ldd mbGB-SDL2/mbGB-SDL2.exe | grep mingw | sed "s/.*=> //" | sed "s/(.*)//"); do
		cp "${lib}" dist
	done
	cp mbGB-SDL2/mbGB-SDL2.exe dist
fi

if [ -d "mbGB-Qt" ]; then
	for lib in $(ldd mbGB-Qt/mbGB-Qt.exe | grep mingw | sed "s/.*=> //" | sed "s/(.*)//"); do
		cp "${lib}" dist
	done
	cp mbGB-Qt/mbGB-Qt.exe dist
fi

cp ../res/bootroms/bin/dmg_bios.bin dist
cp ../res/bootroms/bin/cgb_bios.bin dist
cp ../res/bootroms/bin/agb_bios.bin dist
cp -r ../res/shaders dist
cp ../res/libmbgb.toml dist
if [ -d "mbGB-Qt" ]; then
    windeployqt dist/mbGB-Qt.exe
fi