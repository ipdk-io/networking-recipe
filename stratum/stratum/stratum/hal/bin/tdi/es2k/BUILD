# stratum/hal/bin/tdi/es2k/BUILD

# Copyright 2018 Google LLC
# Copyright 2018-present Open Networking Foundation
# Copyright 2022-2023 Intel Corporation
# SPDX-License-Identifier: Apache-2.0

load("//bazel:rules.bzl", "HOST_ARCHES", "stratum_cc_binary")

licenses(["notice"])  # Apache v2

package(
    default_visibility = ["//visibility:public"],
)

stratum_es2k_common_deps = [
    "//stratum/glue:init_google",
    "//stratum/glue:logging",
    "//stratum/hal/lib/phal:phal_sim",
    "//stratum/hal/lib/phal",
    "//stratum/lib/security:auth_policy_checker",
    "//stratum/lib/security:credentials_manager",
    "@com_github_google_glog//:glog",
]

stratum_cc_binary(
    name = "stratum_es2k",
    srcs = [
        "es2k_main.cc",
        "main.cc",
    ],
    arches = HOST_ARCHES,
    data = [
        "es2k_port_config.pb.txt",
        "es2k_skip_p4.conf",
    ],
    deps = [
        "//stratum/hal/bin/tdi:main",
        "//stratum/hal/lib/tdi/es2k:es2k_chassis_manager",
        "//stratum/hal/lib/tdi/es2k:es2k_hal",
        "//stratum/hal/lib/tdi/es2k:es2k_node",
        "//stratum/hal/lib/tdi/es2k:es2k_port_manager",
        "//stratum/hal/lib/tdi/es2k:es2k_sde_meter",
        "//stratum/hal/lib/tdi/es2k:es2k_sde_utils",
        "//stratum/hal/lib/tdi/es2k:es2k_sde_wrapper",
        "//stratum/hal/lib/tdi/es2k:es2k_switch",
        "//stratum/hal/lib/tdi:tdi_action_profile_manager",
        "//stratum/hal/lib/tdi:tdi_counter_manager",
        "//stratum/hal/lib/tdi:tdi_pre_manager",
        "//stratum/hal/lib/tdi:tdi_sde_wrapper",
        "//stratum/hal/lib/tdi:tdi_table_manager",
    ] + stratum_es2k_common_deps,
)
