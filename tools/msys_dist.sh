#!/bin/bash

if [[ ! -f "libmbGB.a" ]]; then
	echo "Run this script from the directory where you built libmbGB."
	exit 1
fi

mkdir -p dist

if [ -d "mbGB-SDL" ]; then
	for lib in $(ldd mbGB-SDL/mbGB-SDL.exe | grep mingw | sed "s/.*=> //" | sed "s/(.*)//"); do
		cp "${lib}" dist
	done
	cp mbGB-SDL/mbGB-SDL.exe dist
fi