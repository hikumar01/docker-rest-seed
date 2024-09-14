#!/bin/bash
set -o verbose #echo on

rm -rf build bin
mkdir build

cd build
cmake -DCMAKE_BUILD_TYPE=Debug ..
cmake --build . --verbose

cd ..
rm -rf build

./server

set +o verbose #echo off
