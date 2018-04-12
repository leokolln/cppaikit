#!/usr/bin/env bash

if [ "$IS_COVERAGE_BUILD" == 1 ]; then
  coveralls --exclude deps examples --gcov-options '\-lp'
fi
