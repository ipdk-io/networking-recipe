#!/bin/bash

echo ""
echo "===== Build ES2K with OVS ====="
bazel build --config es2k --//flags:ovs //:dummy_krnlmon

echo ""
echo "===== Test ES2K with OVS ====="
bazel test --config es2k --//flags:ovs //switchlink:all //switchsde:all

echo ""
echo "===== Build ES2K without OVS ====="
bazel build --config es2k --//flags:ovs=false //:dummy_krnlmon

echo ""
echo "===== Test ES2K without OVS ====="
bazel test --config es2k --//flags:ovs=false //switchlink:all //switchsde:all
