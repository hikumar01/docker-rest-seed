#!/bin/bash
set -o verbose # echo on

killall rest_api
rm -rf build rest_api
mkdir boost build

if [ ! -d boost/boost_1_86_0 ]; then
    if [ ! -f boost_1_86_0.tar.gz ]; then
        wget -c --progress=bar:force https://archives.boost.io/release/1.86.0/source/boost_1_86_0.tar.gz
    fi

    tar -xzvf boost_1_86_0.tar.gz -C boost
    cd boost/boost_1_86_0
    ./bootstrap.sh
    ./b2 link=static --with-system --with-json
    cd ../..
fi

cd build
# cmake -DBOOST_ROOT=../boost/boost_1_86_0/stage -G Xcode ..
cmake -DBOOST_ROOT=../boost/boost_1_86_0/stage -DCMAKE_BUILD_TYPE=Debug ..
cmake --build . --verbose

cd ..
./rest_api&

set +o verbose # echo off
