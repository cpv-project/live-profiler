#!/usr/bin/env bash
cppcheck --enable=all --inconclusive --std=posix --inline-suppr --quiet --template='{file}:{line},{severity},{id},{message}' -I../include ../

