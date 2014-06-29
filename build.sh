#!/bin/bash
eval "rm -Rf build"
eval "premake4 gmake"
cd build
eval make
ret_code=$?
cd ..
exit $ret_code

