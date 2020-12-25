#!/bin/bash

pushd lib
pushd ws_server
make clean
popd
popd

rm -r ./build/*
