#!/bin/bash

mkdir -p build

pushd build
clang \
    -Wall \
    -std=c11 \
    -D _DEFAULT_SOURCE \
    -I ../code \
    ../code/*.c \
    -l m \
    -o tco_matrix_gen.bin
popd
