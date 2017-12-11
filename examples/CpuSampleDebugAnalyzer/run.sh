#!/usr/bin/env bash
g++ -Wall -Wextra --std=c++14 -I../../include Main.cpp -lbfd -o /tmp/a.out && /tmp/a.out $@

