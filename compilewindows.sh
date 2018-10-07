#!/bin/bash

mkdir build && cd build
cmake .. -G "MSYS Makefiles"
mingw32-make -j4