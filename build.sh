#!/bin/bash
set -o verbose #echo on

rm -rf build bin
mkdir boost build

if [ ! -f boost_1_86_0.tar.gz ]; then
    wget -c --progress=bar:force https://archives.boost.io/release/1.86.0/source/boost_1_86_0.tar.gz
fi

if [ ! -d boost/boost_1_86_0 ]; then
    tar -xzf boost_1_86_0.tar.gz -C boost
    cd boost/boost_1_86_0
    ./bootstrap.sh
    ./b2 link=static --with-system --with-json
    cd ../..
fi

cd build
cmake -DBOOST_ROOT=../boost/boost_1_86_0/stage -DCMAKE_BUILD_TYPE=Debug ..
cmake --build . --verbose

cd ..
./bin/server

set +o verbose #echo off
