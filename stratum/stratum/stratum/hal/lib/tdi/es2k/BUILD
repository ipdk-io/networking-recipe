# stratum/hal/lib/tdi/es2k/BUILD

# Copyright 2018 Google LLC
# Copyright 2018-present Open Networking Foundation
# Copyright 2022-2024 Intel Corporation
# SPDX-License-Identifier: Apache-2.0

load(
    "//bazel:rules.bzl",
    "HOST_ARCHES",
    "STRATUM_INTERNAL",
    "stratum_cc_library",
    "stratum_cc_test",
)

licenses(["notice"])  # Apache v2

package(
    default_visibility = STRATUM_INTERNAL,
)

stratum_cc_library(
    name = "es2k_hal",
    srcs = ["es2k_hal.cc"],
    hdrs = ["es2k_hal.h"],
    deps = [
        "//stratum/glue:logging",
        "//stratum/hal/lib/common:common_cc_proto",
        "//stratum/hal/lib/common:config_monitoring_service",
        "//stratum/hal/lib/common:error_buffer",
        "//stratum/hal/lib/common:p4_service",
        "//stratum/hal/lib/common:switch_interface",
        "//stratum/hal/lib/common:target_options",
        "//stratum/lib:constants",
        "//stratum/lib:macros",
        "//stratum/lib:utils",
        "//stratum/lib/security:auth_policy_checker",
        "//stratum/lib/security:credentials_manager",
        "@com_github_google_glog//:glog",
        "@com_github_grpc_grpc//:grpc++",
        "@com_google_absl//absl/base:core_headers",
        "@com_google_absl//absl/container:flat_hash_map",
        "@com_google_absl//absl/memory",
        "@com_google_absl//absl/strings",
        "@com_google_absl//absl/synchronization",
    ],
)

stratum_cc_library(
    name = "es2k_node",
    srcs = ["es2k_node.cc"],
    hdrs = ["es2k_node.h"],
    deps = [
        "//stratum/hal/lib/tdi:tdi_node",
        "@com_github_gflags_gflags//:gflags",
    ],
)

stratum_cc_library(
    name = "es2k_node_mock",
    testonly = 1,
    hdrs = ["es2k_node_mock.h"],
    arches = HOST_ARCHES,
    deps = [
        ":es2k_node",
        "@com_google_googletest//:gtest",
    ],
)

stratum_cc_library(
    name = "es2k_sde_utils",
    srcs = ["es2k_sde_utils.cc"],
    hdrs = [
        "//stratum/hal/lib/tdi:tdi_sde_utils.h",
    ],
    deps = [
        "@local_es2k_bin//:es2k_sde",
    ],
)

stratum_cc_library(
    name = "es2k_sde_wrapper",
    srcs = [
        "es2k_sde_packetio.cc",
        "es2k_sde_table_data.cc",
        "es2k_sde_wrapper.cc",
    ],
    hdrs = ["es2k_sde_wrapper.h"],
    deps = [
        ":es2k_port_manager",
        "//stratum/glue:integral_types",
        "//stratum/glue:logging",
        "//stratum/glue/gtl:map_util",
        "//stratum/glue/status",
        "//stratum/glue/status:statusor",
        "//stratum/hal/lib/tdi:tdi_constants",
        "//stratum/hal/lib/tdi:tdi_sde_headers",
        "//stratum/lib:constants",
        "//stratum/lib:utils",
        "@com_google_absl//absl/base:core_headers",
        "@com_google_absl//absl/cleanup",
        "@local_es2k_bin//:es2k_sde",
    ],
)

stratum_cc_library(
    name = "es2k_sde_meter",
    srcs = ["es2k_sde_meter.cc"],
    deps = [
        ":es2k_sde_wrapper",
        "//stratum/glue:integral_types",
        "//stratum/glue/status",
        "//stratum/hal/lib/tdi:tdi_sde_headers",
        "@local_es2k_bin//:es2k_sde",
    ],
)

stratum_cc_library(
    name = "es2k_port_manager",
    srcs = ["es2k_port_manager.cc"],
    hdrs = ["es2k_port_manager.h"],
    deps = [
        "//stratum/glue:integral_types",
        "//stratum/glue:logging",
        "//stratum/glue/status",
        "//stratum/glue/status:statusor",
        "//stratum/hal/lib/common:common_cc_proto",
        "//stratum/hal/lib/tdi:tdi_port_manager",
        "//stratum/hal/lib/tdi:tdi_sde_common",
        "//stratum/hal/lib/tdi:tdi_status",
        "//stratum/lib/channel",
        "@com_google_absl//absl/container:flat_hash_map",
        "@com_google_absl//absl/memory",
        "@com_google_absl//absl/synchronization",
        "@local_es2k_bin//:es2k_hdrs",
    ],
)

stratum_cc_library(
    name = "es2k_virtual_port_manager",
    srcs = ["es2k_virtual_port_manager.cc"],
    hdrs = ["es2k_virtual_port_manager.h"],
    deps = [
        "//stratum/glue:integral_types",
        "//stratum/glue:logging",
        "//stratum/glue/status",
        "//stratum/glue/status:statusor",
        "//stratum/hal/lib/common:common_cc_proto",
        "//stratum/hal/lib/tdi:tdi_fixed_function_manager",
        "//stratum/hal/lib/tdi:tdi_sde_common",
        "//stratum/hal/lib/tdi:tdi_status",
        "//stratum/lib/channel",
        "@com_google_absl//absl/container:flat_hash_map",
        "@com_google_absl//absl/memory",
        "@com_google_absl//absl/synchronization",
    ],
)

stratum_cc_library(
    name = "es2k_port_manager_dummy",
    testonly = True,
    srcs = ["es2k_port_manager_dummy.cc"],
    hdrs = ["es2k_port_manager.h"],
    deps = [
        "//stratum/glue:integral_types",
        "//stratum/glue:logging",
        "//stratum/glue/status",
        "//stratum/glue/status:statusor",
        "//stratum/hal/lib/common:common_cc_proto",
        "//stratum/hal/lib/tdi:tdi_sde_headers",
        "//stratum/lib/channel",
        "@com_google_absl//absl/container:flat_hash_map",
        "@com_google_absl//absl/synchronization",
    ],
)

stratum_cc_library(
    name = "es2k_port_manager_interface",
    hdrs = ["es2k_port_manager.h"],
    deps = [
        "//stratum/hal/lib/tdi:tdi_port_manager",
    ],
)

stratum_cc_library(
    name = "es2k_port_manager_mock",
    testonly = True,
    hdrs = ["es2k_port_manager_mock.h"],
    deps = [
        ":es2k_port_manager_dummy",
        "@com_google_googletest//:gtest",
    ],
)

stratum_cc_library(
    name = "es2k_switch",
    srcs = ["es2k_switch.cc"],
    hdrs = ["es2k_switch.h"],
    deps = [
        ":es2k_node",
        "//stratum/glue:integral_types",
        "//stratum/glue:logging",
        "//stratum/glue/gtl:map_util",
        "//stratum/glue/status:status_macros",
        "//stratum/hal/lib/common:switch_interface",
        "//stratum/hal/lib/tdi:tdi_global_vars",
        "//stratum/hal/lib/tdi:tdi_ipsec_manager",
        "//stratum/hal/lib/tdi/es2k:es2k_chassis_manager",
        "//stratum/lib:constants",
        "//stratum/lib:macros",
        "@com_google_absl//absl/container:flat_hash_map",
        "@com_google_googleapis//google/rpc:status_cc_proto",
    ],
)

stratum_cc_library(
    name = "es2k_chassis_manager",
    srcs = ["es2k_chassis_manager.cc"],
    hdrs = ["es2k_chassis_manager.h"],
    deps = [
        ":es2k_port_manager_interface",
        "//stratum/glue:integral_types",
        "//stratum/glue:logging",
        "//stratum/glue/gtl:map_util",
        "//stratum/glue/status",
        "//stratum/glue/status:statusor",
        "//stratum/hal/lib/common:common_cc_proto",
        "//stratum/hal/lib/common:constants",
        "//stratum/hal/lib/common:phal_interface",
        "//stratum/hal/lib/common:switch_interface",
        "//stratum/hal/lib/common:utils",
        "//stratum/hal/lib/common:writer_interface",
        "//stratum/hal/lib/tdi:tdi_global_vars",
        "//stratum/hal/lib/tdi:tdi_sde_headers",
        "//stratum/lib:constants",
        "//stratum/lib:macros",
        "//stratum/lib:utils",
        "//stratum/public/lib:error",
        "@com_github_google_glog//:glog",
        "@com_google_absl//absl/base:core_headers",
        "@com_google_absl//absl/memory",
        "@com_google_absl//absl/synchronization",
        "@com_google_absl//absl/types:optional",
        "@com_google_protobuf//:protobuf",
    ],
)

stratum_cc_library(
    name = "es2k_chassis_manager_mock",
    testonly = 1,
    hdrs = ["es2k_chassis_manager_mock.h"],
    arches = HOST_ARCHES,
    deps = [
        ":es2k_chassis_manager",
        "@com_google_googletest//:gtest",
    ],
)

stratum_cc_test(
    name = "es2k_chassis_manager_test",
    srcs = ["es2k_chassis_manager_test.cc"],
    deps = [
        ":es2k_chassis_manager",
        ":es2k_port_manager_mock",
        ":test_main",
        "//stratum/glue:integral_types",
        "//stratum/glue/gtl:map_util",
        "//stratum/glue/status",
        "//stratum/glue/status:status_test_util",
        "//stratum/glue/status:statusor",
        "//stratum/hal/lib/common:common_cc_proto",
        "//stratum/hal/lib/common:constants",
        "//stratum/hal/lib/common:phal_mock",
        "//stratum/hal/lib/common:utils",
        "//stratum/hal/lib/common:writer_interface",
        "//stratum/hal/lib/common:writer_mock",
        "//stratum/hal/lib/tdi:tdi_global_vars",
        "//stratum/hal/lib/tdi:tdi_sde_interface",
        "//stratum/lib:utils",
        "//stratum/lib/test_utils:matchers",
        "@com_google_absl//absl/strings",
        "@com_google_absl//absl/synchronization",
        "@com_google_absl//absl/time",
        "@com_google_googletest//:gtest",
        "@local_es2k_bin//:target_sys",
    ],
)

stratum_cc_test(
    name = "es2k_hal_test",
    srcs = ["es2k_hal_test.cc"],
    deps = [
        ":es2k_hal",
        ":test_main",
        "//stratum/glue/net_util:ports",
        "//stratum/glue/status:status_test_util",
        "//stratum/hal/lib/common:switch_mock",
        "//stratum/lib:utils",
        "//stratum/lib/security:auth_policy_checker_mock",
        "//stratum/lib/test_utils:matchers",
        "//stratum/public/lib:error",
        "@com_github_google_glog//:glog",
        "@com_google_absl//absl/strings",
        "@com_google_googletest//:gtest",
    ],
)

stratum_cc_test(
    name = "es2k_switch_test",
    srcs = ["es2k_switch_test.cc"],
    deps = [
        ":es2k_chassis_manager_mock",
        ":es2k_node_mock",
        ":es2k_switch",
        ":test_main",
        "//stratum/glue/status:status_test_util",
        "//stratum/hal/lib/common:switch_interface",
        "//stratum/hal/lib/common:writer_mock",
        "//stratum/hal/lib/p4:p4_table_mapper_mock",
        "//stratum/hal/lib/tdi:tdi_global_vars",
        "//stratum/hal/lib/tdi:tdi_ipsec_manager_mock",
        "//stratum/hal/lib/tdi:tdi_sde_mock",
        "//stratum/lib:utils",
        "//stratum/lib/channel:channel_mock",
        "//stratum/lib/test_utils:matchers",
        "@com_google_absl//absl/memory",
        "@com_google_googletest//:gtest",
    ],
)

stratum_cc_test(
    name = "es2k_tdi_status_test",
    srcs = ["es2k_tdi_status_test.cc"],
    deps = [
        ":test_main",
        "//stratum/hal/lib/tdi:tdi_status",
        "@com_google_googletest//:gtest",
        "@local_es2k_bin//:es2k_hdrs",
    ],
)

stratum_cc_library(
    name = "test_main",
    testonly = 1,
    srcs = ["test_main.cc"],
    deps = [
        "//stratum/glue:init_google",
        "//stratum/glue:logging",
        "@com_github_google_glog//:glog",
        "@com_google_googletest//:gtest",
    ],
)
