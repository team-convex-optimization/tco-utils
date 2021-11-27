#!/bin/bash

mkdir -p build

pushd build
clang \
    -D _DEFAULT_SOURCE \
    -Wall \
    -std=c11 \
    -D TRAINING \
    -I ../code \
    -I ../lib/tco_shmem \
    -lrt \
    -pthread \
    ../code/main.c \
    -o tco_oled.bin
popd
