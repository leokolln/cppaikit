#!/usr/bin/env bash

if [ "$TRAVIS_OS_NAME" == "linux" ] && [ "$COMPILER" == "g++-7" ]; then
  IS_COVERAGE_BUILD=1
else
  IS_COVERAGE_BUILD=0
fi

export IS_COVERAGE_BUILD
