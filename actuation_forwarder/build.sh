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
    -l rt \
    -l pthread -g\
    ../code/*.c \
    tco_libd.a \
    -o tco_actuation_forwarder.bin
popd
