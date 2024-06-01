#!/bin/bash

rm -fr build install

# ${_OVS_BLD} ${_OVS_DIR} ${_TOOLCHAIN_FILE}
cmake -S . -B build -DCMAKE_INSTALL_PREFIX=install -DP4OVS=ON
cmake --build build -j6 -- V=0
