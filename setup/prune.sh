#!/bin/bash
#
# Copyright 2023 Intel Corporation
# SPDX-License-Identifier: Apache 2.0
#
# Removes unnecessary third-party packages.
#

# duplicate packages
rm -fr grpc/third_party/abseil-cpp
rm -fr grpc/third_party/bloaty/abseil-cpp
rm -fr grpc/third_party/bloaty/third_party/abseil-cpp
rm -fr grpc/third_party/bloaty/third_party/protobuf
rm -fr grpc/third_party/bloaty/third_party/zlib
rm -fr grpc/third_party/cares
rm -fr grpc/third_party/protobuf
rm -fr grpc/third_party/zlib

# unused packages
rm -fr grpc/third_party/boringssl-with-bazel
