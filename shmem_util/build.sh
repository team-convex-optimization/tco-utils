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
    ../code/main.c \
    -o tco_shmem_util.bin
popd
