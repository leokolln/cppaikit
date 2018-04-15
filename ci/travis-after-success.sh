#!/usr/bin/env bash

if [ "$IS_COVERAGE_BUILD" == 1 ]; then
  coveralls -E deps -E examples --gcov-options '\-lp'
fi
