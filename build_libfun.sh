#!/usr/bin/env bash

if [ $# -gt 0 ]; then
    rm -rf libfun/build
fi

mkdir -p libfun/build
cd libfun/build

cmake ..
make -j `nproc`
cd ../../