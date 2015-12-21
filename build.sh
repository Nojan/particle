#!/bin/bash
$CXX --version
eval "rm -Rf build"
eval "premake4 gmake"
cd build
eval make CC='gcc-4.9' CXX='g++-4.9'
ret_code=$?
cd ..
exit $ret_code

