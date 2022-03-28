#!/bin/bash

mkdir -p build

pushd lib

pushd ws_server
make all
mv libws.a ../../build
popd

pushd tco_libd
./build.sh
mv build/tco_libd.a ../../build
popd

popd

pushd build
clang \
    -D _DEFAULT_SOURCE \
    -Wall \
    -std=c11 \
    -I ../code \
    -I ../lib/ws_server/include \
    -I ../lib/tco_shmem \
    -I ../lib/tco_libd/include \
    -lpthread \
    -lrt \
    -O3 \
    ../code/*.c \
    libws.a \
    tco_libd.a \
    -o tco_actuation_forwarder.bin
popd
