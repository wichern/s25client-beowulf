#!/bin/base
mkdir -p build
cd build
cmake -DCMAKE_BUILD_TYPE=$2 ..
make $1 -j$(nproc)
