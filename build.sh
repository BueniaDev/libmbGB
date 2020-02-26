#!/bin/bash

if [[ ! -d "build/" ]]
then
  python beemake.py example
fi

./build/example/sdl2/example gbtictac.gb