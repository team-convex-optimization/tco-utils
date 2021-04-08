#!/bin/bash

mkdir -p build

pushd lib/tco_libd
./build.sh
mv -f build/tco_libd.a ../../build
popd

pushd build
clang \
    -Wall \
    -std=c11 \
    -D _DEFAULT_SOURCE \
    -I ../code \
    -I ../lib/tco_libd/include \
    -I ../lib/tco_shmem \
    -l ncurses \
    -l pthread \
    -l rt \
    ../code/*.c \
    tco_libd.a \
    -o tco_shmem_viewer.bin
popd
