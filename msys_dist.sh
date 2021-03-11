#!/bin/bash

if [[ ! -f "libmbGB.a" ]]; then
	echo "Run this script from the directory where you built libmbGB."
	exit 1
fi

mkdir -p dist

if [ -d "example/sdl2" ]; then
	for lib in $(ldd example/sdl2/example.exe | grep mingw | sed "s/.*=> //" | sed "s/(.*)//"); do
		cp "${lib}" dist
	done
	cp example/sdl2/example.exe dist
fi

if [ -d "example/qt" ]; then
	for lib in $(ldd example/qt/mbGB-Qt.exe | grep mingw | sed "s/.*=> //" | sed "s/(.*)//"); do
		cp "${lib}" dist
	done
	cp example/qt/mbGB-Qt.exe dist
fi

cp ../res/bootroms/bin/dmg_bios.bin dist
cp ../res/bootroms/bin/cgb_bios.bin dist
cp ../res/bootroms/bin/agb_bios.bin dist
cp -r ../res/shaders dist
cp ../res/libmbgb.toml dist
if [ -d "example/qt" ]; then
    windeployqt dist/mbGB-Qt.exe
fi