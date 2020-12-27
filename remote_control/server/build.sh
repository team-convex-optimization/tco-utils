#!/bin/bash

mkdir -p build

pushd lib
pushd ws_server
make
mv libws.a ../../build/libws.a
popd
popd

pushd build
clang \
    -Wall \
    -std=c11 \
    -I ../code \
    -I ../lib/ws_server/include \
    -I ../lib/tco_shmem \
    -lpthread \
    -lrt \
    ../code/main.c \
    ../build/libws.a \
    -o remote_control.bin
popd
