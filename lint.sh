#!/bin/sh

run-clang-tidy -p build/ src/*.cxx include/*.h -j 8 -fix
