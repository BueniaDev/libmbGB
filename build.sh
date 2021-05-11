#!/bin/bash

if [[ ! -d "build/" ]]
then
  mkdir build && cd build
  cmake .. -G "Unix Makefiles" -DBUILD_SDL2="ON" -DCMAKE_BUILD_TYPE="Release"
  make -j$(nproc --all)
  cd ../
fi

./build/example/sdl2/example gbtictac.gb