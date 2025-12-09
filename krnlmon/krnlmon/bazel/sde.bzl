# //bazel/sde.bzl

# Copyright 2024 Intel Corporation
# SPDX-License-Identifier: Apache-2.0

load("//bazel:defs.bzl", "NO_MATCH_ERROR")

TARGET_SDE = select(
    {
        "//flags:dpdk_target": ["@local_dpdk_bin//:sde"],
        "//flags:es2k_target": ["@local_es2k_bin//:sde"],
    },
    no_match_error = NO_MATCH_ERROR,
)

TARGET_SDE_HDRS = select(
    {
        "//flags:dpdk_target": ["@local_dpdk_bin//:sde_hdrs"],
        "//flags:es2k_target": ["@local_es2k_bin//:sde_hdrs"],
    },
    no_match_error = NO_MATCH_ERROR,
)

TARGET_SDE_LIBS = select(
    {
        "//flags:dpdk_target": ["@local_dpdk_bin//:sde_libs"],
        "//flags:es2k_target": ["@local_es2k_bin//:sde_libs"],
    },
    no_match_error = NO_MATCH_ERROR,
)

TARGET_TDI = select(
    {
        "//flags:dpdk_target": ["@local_dpdk_bin//:tdi"],
        "//flags:es2k_target": ["@local_es2k_bin//:tdi"],
    },
    no_match_error = NO_MATCH_ERROR,
)
