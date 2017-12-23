#!/usr/bin/env bash
# A test framework would be better but... I dont like bunch of macros
# ###################################################################
set -e

IS_WINDOWS="$(uname | grep MINGW || true)"
IS_LINUX="$(uname | grep Linux || true)"

if [ -n "${IS_WINDOWS}" ]; then
	echo "Run tests on windows..."
	SYSTEM_NAME="Windows"
	export CXX=g++
fi

if [ -n "${IS_LINUX}" ]; then
	echo "Run tests on linux..."
	SYSTEM_NAME="Linux"
fi

mkdir -p Build
cd Build
cmake -DCMAKE_SYSTEM_NAME="${SYSTEM_NAME}" ../
make
./LiveProfilerTest
