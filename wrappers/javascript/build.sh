#!/usr/bin/env bash

ln -sf /usr/include/eigen3 /usr/local/include/eigen3
ln -sf /usr/include/nlohmann /usr/local/include/nlohmann

mkdir build
cd build 
ls -al
cmake -DCMAKE_TOOLCHAIN_FILE="/emsdk/upstream/emscripten/cmake/Modules/Platform/Emscripten.cmake" -DBUILD_TYPE=javascript -DCOMMIT_HASH=$1 -DGEOFIX_VERSION=$2 ..
make
cd ..
rm -rf build
