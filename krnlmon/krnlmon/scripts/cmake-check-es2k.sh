#!/bin/bash

echo ""
echo "===== Build ES2K with OVS ====="
rm -fr build install
set -e
cmake -B build -C es2k.cmake -DWITH_OVSP4RT=ON
cmake --build build -j4 --target install

echo ""
echo "===== Test ES2K with OVS ====="
set +e
(cd build; ctest)

echo ""
echo "===== Build ES2K without OVS ====="
rm -fr build install
set -e
cmake -B build -C es2k.cmake -DWITH_OVSP4RT=OFF
cmake --build build -j4 --target install

echo ""
echo "===== Test ES2K without OVS ====="
set +e
(cd build; ctest)
