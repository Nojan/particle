#!/bin/bash

eval "rm -Rf build"
mkdir build
cd build
eval "cmake .."
eval make
ret_code=$?
cd ..
exit $ret_code

