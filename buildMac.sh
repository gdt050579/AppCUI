#!/bin/sh
rm -rf build
cmake -Bbuild -DCMAKE_PREFIX_PATH=/opt/homebrew/Cellar/ncurses/6.2 -G Xcode .
cmake --build build
