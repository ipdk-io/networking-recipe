# Builds stratum protobuf libraries
#
# Copyright 2022-2024 Intel Corporation
# SPDX-License-Identifier: Apache 2.0
#

set(STRATUM_P4_PROTO_FILES
    stratum/hal/lib/p4/forwarding_pipeline_configs.proto
    stratum/hal/lib/phal/db.proto
)

set(STRATUM_BF_PROTO_FILES
    stratum/hal/lib/p4/common_flow_entry.proto
    stratum/hal/lib/p4/p4_table_map.proto
    stratum/hal/lib/p4/p4_pipeline_config.proto
    stratum/hal/lib/tdi/tdi.proto
)

generate_proto_files("${STRATUM_P4_PROTO_FILES}" "${STRATUM_SOURCE_DIR}")
generate_proto_files("${STRATUM_BF_PROTO_FILES}" "${STRATUM_SOURCE_DIR}")

# stratum_proto1_o
add_library(stratum_proto1_o OBJECT
    ${PB_OUT_DIR}/stratum/hal/lib/p4/common_flow_entry.pb.h
    ${PB_OUT_DIR}/stratum/hal/lib/p4/common_flow_entry.pb.cc
    ${PB_OUT_DIR}/stratum/hal/lib/p4/p4_table_map.pb.h
    ${PB_OUT_DIR}/stratum/hal/lib/p4/p4_table_map.pb.cc
    ${PB_OUT_DIR}/stratum/hal/lib/p4/p4_pipeline_config.pb.h
    ${PB_OUT_DIR}/stratum/hal/lib/p4/p4_pipeline_config.pb.cc
    ${PB_OUT_DIR}/stratum/hal/lib/tdi/tdi.pb.h
    ${PB_OUT_DIR}/stratum/hal/lib/tdi/tdi.pb.cc
)

# Ensure that the header files on which we depend have been generated
# before we start building the current library.
add_dependencies(stratum_proto1_o
    common_proto_o
    p4_annotation_proto_o
    p4_control_proto_o
    p4_table_defs_proto_o
    p4runtime_proto_o
)

target_include_directories(stratum_proto1_o PRIVATE ${PB_OUT_DIR})

# stratum_proto2_o
add_library(stratum_proto2_o OBJECT
    ${PB_OUT_DIR}/stratum/hal/lib/p4/forwarding_pipeline_configs.pb.cc
    ${PB_OUT_DIR}/stratum/hal/lib/p4/forwarding_pipeline_configs.pb.h
    ${PB_OUT_DIR}/stratum/hal/lib/phal/db.pb.cc
    ${PB_OUT_DIR}/stratum/hal/lib/phal/db.pb.h
)

# Ensure that the header files on which we depend have been generated
# before we start building the current library.
add_dependencies(stratum_proto2_o
    error_proto_o
    p4runtime_proto_o
    stratum_proto1_o
)

target_include_directories(stratum_proto2_o PRIVATE ${PB_OUT_DIR})

# stratum_proto
add_library(stratum_proto SHARED
    $<TARGET_OBJECTS:common_proto_o>
    $<TARGET_OBJECTS:error_proto_o>
    $<TARGET_OBJECTS:p4_annotation_proto_o>
    $<TARGET_OBJECTS:p4_control_proto_o>
    $<TARGET_OBJECTS:p4_table_defs_proto_o>
    $<TARGET_OBJECTS:stratum_proto1_o>
    $<TARGET_OBJECTS:stratum_proto2_o>
)

target_link_libraries(stratum_proto PUBLIC
    p4runtime_proto
    protobuf::libprotobuf
)

install(TARGETS stratum_proto LIBRARY)
