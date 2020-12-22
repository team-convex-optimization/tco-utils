#!/usr/bin/env bash

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
    ../code/main.c \
    ../build/libws.a \
    -lpthread \
    -o rc-controller.bin
popd
