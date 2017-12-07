#!/usr/bin/env bash
valgrind --track-fds=yes --leak-check=full ./Build/LiveProfilerTest

