#!/bin/sh
rm -rf build
cmake -Bbuild .
cmake --build build