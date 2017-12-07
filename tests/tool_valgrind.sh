#!/usr/bin/env bash
cd ./Build && valgrind --track-fds=yes --leak-check=full ./LiveProfilerTest

