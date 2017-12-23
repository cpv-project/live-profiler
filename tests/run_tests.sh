#!/usr/bin/env bash
# A test framework would be better but... I dont like bunch of macros
# ###################################################################
set -e

mkdir -p Build
cd Build
cmake -DCMAKE_SYSTEM_NAME="Generic" ../
make
./LiveProfilerTest
