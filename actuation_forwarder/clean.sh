#!/bin/bash

pushd lib/tco_libd
./clean.sh
popd

rm code/toyws.*

rm -r ./build/*
