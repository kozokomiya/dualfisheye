#!/bin/bash

mkdir -p build

#
echo 'Generating map files for dualfisheye' 
cp make_maps.py build
cd build
python3 make_maps.py
cd ..
meson build
ninja -C build
