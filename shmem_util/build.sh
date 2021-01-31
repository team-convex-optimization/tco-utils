#!/bin/bash

mkdir -p build

pushd build
clang \
    -D _DEFAULT_SOURCE \
    -Wall \
    -std=c11 \
    -I ../code \
    -I ../lib/tco_shmem \
    -lrt \
    -pthread \
    ../code/main.c \
    $1 \
    -o tco_shmem_util.bin
popd
