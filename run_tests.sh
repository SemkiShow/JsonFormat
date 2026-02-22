#!/bin/bash

set -e

cmake -B build -DCMAKE_BUILD_TYPE=RelWithDebInfo -DJSON_FORMAT_BUILD_TESTS=ON
cmake --build build -j$(nproc)
./build/bin/JsonFormatTests
