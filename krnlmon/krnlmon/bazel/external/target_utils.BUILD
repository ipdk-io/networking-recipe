# //bazel/external/target_utils.BUILD

# Copyright 2020-present Open Networking Foundation
# Copyright 2022-2024 Intel Corporation
# SPDX-License-Identifier: Apache-2.0

load("@rules_cc//cc:defs.bzl", "cc_library")

package(
    default_visibility = ["//visibility:public"],
)

cc_library(
    name = "target_utils",
    srcs = glob(["target-utils/lib/libtarget_utils.so*"]),
    hdrs = glob(["target-utils/include/target-utils/**/*.h"]),
    linkopts = [
        "-lpthread",
        "-lm",
        "-ldl",
    ],
    strip_include_prefix = "target-utils/include",
    deps = ["@target_sys"],
)

cc_library(
    name = "cjson",
    srcs = glob(["target-utils/lib/libtarget_utils.so*"]),
    hdrs = glob([
        "target-utils/include/target-utils/third-party/cJSON/*.h",
    ]),
    strip_include_prefix = "target-utils/include/target-utils/third-party/",
)

cc_library(
    name = "judy",
    srcs = glob(["target-utils/lib/libtarget_utils.so*"]),
    hdrs = glob([
        "target-utils/include/target-utils/third-party/judy-1.0.5/src/*.h",
    ]),
    strip_include_prefix = "target-utils/include/target-utils/third-party/",
)

cc_library(
    name = "tommyds",
    srcs = glob(["target-utils/lib/libtarget_utils.so*"]),
    hdrs = glob([
        "target-utils/include/target-utils/third-party/tommyds/tommyds/*.h",
    ]),
    strip_include_prefix = "target-utils/include/target-utils/third-party/tommyds",
)

cc_library(
    name = "xxhash",
    srcs = glob(["target-utils/lib/libtarget_utils.so*"]),
    hdrs = glob([
        "target-utils/include/target-utils/third-party/xxHash/xxHash/*.h",
    ]),
    strip_include_prefix = "target-utils/include/target-utils/third-party/xxHash",
)
