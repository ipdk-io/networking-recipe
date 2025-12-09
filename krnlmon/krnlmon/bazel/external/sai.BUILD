# //bazel/external/sai.BUILD

# Copyright 2024 Intel Corporation
# SPDX-License-Identifier: Apache-2.0

load("@rules_cc//cc:defs.bzl", "cc_library")

package(
    default_visibility = ["//visibility:public"],
)

cc_library(
    name = "sai_hdrs",
    hdrs = glob(["inc/*.h"]),
    strip_include_prefix = "inc/",
)
