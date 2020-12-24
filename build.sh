#!/bin/bash

if [[ ! -d "build/" ]]
then
  mkdir build && cd build
  cmake .. -G "MSYS Makefiles" -DBUILD_EXAMPLE="ON" -DCMAKE_BUILD_TYPE="Release"
  make -j$(nproc --all)
  cd ../
fi

./build/example/sdl2/example gbtictac.gb