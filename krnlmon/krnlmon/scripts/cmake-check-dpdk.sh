#!/bin/bash

echo ""
echo "===== Build DPDK with OVS ====="
rm -fr build install
set -e
cmake -B build -C dpdk.cmake -DWITH_OVSP4RT=ON
cmake --build build -j4 --target install

echo ""
echo "===== Test DPDK with OVS ====="
set +e
(cd build; ctest)

echo ""
echo "===== Build DPDK without OVS ====="
rm -fr build install
set -e
cmake -B build -C dpdk.cmake -DWITH_OVSP4RT=OFF
cmake --build build -j4 --target install

echo ""
echo "===== Test DPDK without OVS ====="
set +e
(cd build; ctest)
