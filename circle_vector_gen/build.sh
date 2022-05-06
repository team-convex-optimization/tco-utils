#!/bin/bash

mkdir -p build

pushd lib/tco_linalg
./build.sh
mv -f build/tco_linalg.a ../../build
popd

pushd build
clang \
    -Wall \
    -std=c11 \
    -I ../code \
    -I ../lib/tco_linalg/include \
    ../code/*.c \
    tco_linalg.a \
    -o tco_circle_vector_gen.bin
popd
