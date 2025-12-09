# //bazel/external/target_sys.BUILD

# Copyright 2020-present Open Networking Foundation
# Copyright 2022-2024 Intel Corporation
# SPDX-License-Identifier: Apache-2.0

load("@rules_cc//cc:defs.bzl", "cc_library")

package(
    default_visibility = ["//visibility:public"],
)

cc_library(
    name = "target_sys",
    srcs = glob(["target-sys/lib/libtarget_sys.so*"]),
    hdrs = glob(["target-sys/include/target-sys/**/*.h"]),
    strip_include_prefix = "target-sys/include",
)
