#!/usr/bin/env bash

if [ "$COVERITY_SCAN_BRANCH" == 1 ]; then
  cat build/cov-int/scm_log.txt
  exit
fi

if [ "$IS_COVERAGE_BUILD" == 1 ]; then
  flags="-g -O0 --coverage"
  cmake -DCMAKE_CXX_COMPILER=$COMPILER -DCMAKE_CXX_FLAGS="$flags" -DCMAKE_BUILD_TYPE=Debug .
else
  mkdir build && cd build
  cmake -DCMAKE_CXX_COMPILER=$COMPILER ..
fi

cmake --build .
ctest -V --output-on-failure
