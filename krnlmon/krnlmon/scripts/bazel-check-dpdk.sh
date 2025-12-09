#!/bin/bash

echo ""
echo "===== Build DPDK with OVS ====="
bazel build --config dpdk --//flags:ovs //:dummy_krnlmon

echo ""
echo "===== Test DPDK with OVS ====="
bazel test --config dpdk --//flags:ovs //switchlink:all //switchsde:all

echo ""
echo "===== Build DPDK without OVS ====="
bazel build --config dpdk --//flags:ovs=false //:dummy_krnlmon

echo ""
echo "===== Test DPDK without OVS ====="
bazel test --config dpdk --//flags:ovs=false //switchlink:all //switchsde:all
