#!/usr/bin/env bash
# A test framework would be better but... I dont like bunch of macros
# ###################################################################
mkdir -p Build
cd Build
cmake ../ && \
make && \
./LiveProfilerTest
