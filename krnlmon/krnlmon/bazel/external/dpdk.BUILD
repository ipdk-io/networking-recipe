# //bazel/external/dpdk.BUILD

# Copyright 2020-present Open Networking Foundation
# Copyright 2022-2024 Intel Corporation
# SPDX-License-Identifier: Apache-2.0

load("@rules_cc//cc:defs.bzl", "cc_library")

package(
    default_visibility = ["//visibility:public"],
)

cc_library(
    name = "sde_libs",
    srcs = glob([
        "dpdk-bin/lib/libbf_switchd_lib.so*",
        "dpdk-bin/lib/libclish.so*",
        "dpdk-bin/lib/libdpdk_infra.so*",
        "dpdk-bin/lib/libdriver.so*",
        "dpdk-bin/lib/libpython3.10.so*",
        "dpdk-bin/lib/x86_64-linux-gnu/*.so*",
    ]),
    linkopts = [
        "-lpthread",
        "-lm",
        "-ldl",
    ],
    deps = [
        "@target_sys",
    ],
)

cc_library(
    name = "sde_hdrs",
    hdrs = glob([
        "dpdk-bin/include/bf_pal/*.h",
        "dpdk-bin/include/bf_rt/**/*.h",
        "dpdk-bin/include/bf_switchd/**/*.h",
        "dpdk-bin/include/bf_types/*.h",
        "dpdk-bin/include/dvm/*.h",
        "dpdk-bin/include/fixed_function/*.h",
        "dpdk-bin/include/lld/*.h",
        "dpdk-bin/include/osdep/*.h",
        "dpdk-bin/include/pipe_mgr/**/*.h",
        "dpdk-bin/include/port_mgr/**/*.h",
    ]),
    strip_include_prefix = "dpdk-bin/include",
)

cc_library(
    name = "sde",
    deps = [
        ":sde_hdrs",
        ":sde_libs",
    ],
)

cc_library(
    name = "tdi",
    srcs = glob([
        "dpdk-bin/lib/libtdi.so*",
        "dpdk-bin/lib/libtdi_json_parser.so*",
        "dpdk-bin/lib/libtdi_pna.so*",
    ]),
   hdrs = glob([
        "dpdk-bin/include/tdi/**/*.h",
        "dpdk-bin/include/tdi/**/*.hpp",
        "dpdk-bin/include/tdi_rt/**/*.h",
        "dpdk-bin/include/tdi_rt/**/*.hpp",
    ]),
    strip_include_prefix = "dpdk-bin/include",
)
