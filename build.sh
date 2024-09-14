#!/bin/bash
set -o verbose #echo on

rm -rf boost build
mkdir boost build

tar -xzf boost_1_86_0.tar.gz -C boost
cd boost/boost_1_86_0
./bootstrap.sh
./b2 link=static --with-system --with-json

cd ../../build
cmake -DBOOST_ROOT=../boost/boost_1_86_0/stage -DCMAKE_BUILD_TYPE=Debug ..
cmake --build . --verbose

cd ..
rm -rf boost build

./server

set +o verbose #echo off
