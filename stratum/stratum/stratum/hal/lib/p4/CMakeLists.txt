# Build file for //stratum/hal/lib/p4
#
# Copyright 2022-2024 Intel Corporation
# SPDX-License-Identifier: Apache 2.0
#

add_library(stratum_hal_lib_p4_o OBJECT
    p4_info_manager.cc
    p4_info_manager.h
    utils.cc
    utils.h
)

target_include_directories(stratum_hal_lib_p4_o PRIVATE ${STRATUM_INCLUDES})

add_dependencies(stratum_hal_lib_p4_o
    stratum_proto
    p4runtime_proto
)

############################
# Build p4_control_proto_o #
############################

generate_proto_files(
    "stratum/hal/lib/p4/p4_control.proto"
    "${STRATUM_SOURCE_DIR}"
)

add_library(p4_control_proto_o OBJECT
    ${PB_OUT_DIR}/stratum/hal/lib/p4/p4_control.pb.cc
    ${PB_OUT_DIR}/stratum/hal/lib/p4/p4_control.pb.h
)

add_dependencies(p4_control_proto_o
    p4_annotation_proto_o
    p4_table_defs_proto_o
)

target_include_directories(p4_control_proto_o PUBLIC ${PB_OUT_DIR})
