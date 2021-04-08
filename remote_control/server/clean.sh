#!/bin/bash

pushd lib

pushd ws_server
make clean
popd

pushd tco_libd
./clean.sh
popd

popd

rm -r ./build/*
