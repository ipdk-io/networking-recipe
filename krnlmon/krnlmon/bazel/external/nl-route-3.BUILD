# //bazel/external/nl-route-3.BUILD

# Copyright 2024 Intel Corporation
# SPDX-License-Identifier: Apache-2.0

load("@rules_cc//cc:defs.bzl", "cc_import")

cc_import(
    name = "nl-route-3",
    hdrs = glob(["netlink/**/*.h"]),
    includes = ["/usr/include/libnl3"],
    system_provided = 1,
    visibility = ["//visibility:public"],
)
