#!/bin/sh
rm -rf build && mkdir build && cd build
cmake -DCMAKE_PREFIX_PATH=/opt/homebrew/Cellar/ncurses/6.2 -G Xcode ..
cmake --build .
