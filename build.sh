#!/bin/bash

#
echo 'Generating map files for dualfisheye' 
python3 src/make_maps.py
meson build
ninja -C build
