#!/bin/sh

cd $TRAVIS_BUILD_DIR
mkdir build && cd build
cmake .. -G "Xcode"
cmake --build $TRAVIS_BUILD_DIR/build --config Release