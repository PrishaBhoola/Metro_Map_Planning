#!/bin/bash
# Compile C++ sources into run1 and run2
set -e
mkdir -p bin
g++ -std=c++11 -O2 -Wall src/run1.cpp -o bin/run1
g++ -std=c++11 -O2 -Wall src/run2.cpp -o bin/run2

echo "Built bin/run1 and bin/run2"