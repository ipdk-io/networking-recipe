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

####################
# libstratum_proto #
####################

add_library(stratum_proto SHARED
    $<TARGET_OBJECTS:common_proto_o>
    $<TARGET_OBJECTS:common_flow_entry_proto_o>
    $<TARGET_OBJECTS:error_proto_o>
    $<TARGET_OBJECTS:forwarding_pipeline_configs_proto_o>
    $<TARGET_OBJECTS:p4_annotation_proto_o>
    $<TARGET_OBJECTS:p4_control_proto_o>
    $<TARGET_OBJECTS:p4_pipeline_config_proto_o>
    $<TARGET_OBJECTS:p4_table_defs_proto_o>
    $<TARGET_OBJECTS:p4_table_map_proto_o>
    $<TARGET_OBJECTS:phal_db_proto_o>
    $<TARGET_OBJECTS:stratum_tdi_proto_o>
)

target_include_directories(stratum_proto PUBLIC ${PB_OUT_DIR})

target_link_libraries(stratum_proto PUBLIC
    p4runtime_proto
    protobuf::libprotobuf
)

install(TARGETS stratum_proto LIBRARY)

#############################
# common_flow_entry_proto_o #
#############################

add_library(common_flow_entry_proto_o OBJECT
  ${PB_OUT_DIR}/stratum/hal/lib/p4/common_flow_entry.pb.h
  ${PB_OUT_DIR}/stratum/hal/lib/p4/common_flow_entry.pb.cc
)

target_include_directories(common_flow_entry_proto_o PUBLIC ${PB_OUT_DIR})

add_dependencies(common_flow_entry_proto_o
  p4_table_defs_proto_o
  p4_annotation_proto_o
  p4runtime_proto_o
)

#######################################
# forwarding_pipeline_configs_proto_o #
#######################################

add_library(forwarding_pipeline_configs_proto_o OBJECT
  ${PB_OUT_DIR}/stratum/hal/lib/p4/forwarding_pipeline_configs.pb.h
  ${PB_OUT_DIR}/stratum/hal/lib/p4/forwarding_pipeline_configs.pb.cc
)

target_include_directories(forwarding_pipeline_configs_proto_o PUBLIC ${PB_OUT_DIR})

add_dependencies(forwarding_pipeline_configs_proto_o
  p4runtime_proto_o
)

##############################
# p4_pipeline_config_proto_o #
##############################

add_library(p4_pipeline_config_proto_o OBJECT
  ${PB_OUT_DIR}/stratum/hal/lib/p4/p4_pipeline_config.pb.h
  ${PB_OUT_DIR}/stratum/hal/lib/p4/p4_pipeline_config.pb.cc
)

target_include_directories(p4_pipeline_config_proto_o PUBLIC ${PB_OUT_DIR})

add_dependencies(p4_pipeline_config_proto_o
  p4_annotation_proto_o
  p4_control_proto_o
  p4_table_map_proto_o
  p4runtime_proto_o
)

########################
# p4_table_map_proto_o #
########################

add_library(p4_table_map_proto_o OBJECT
  ${PB_OUT_DIR}/stratum/hal/lib/p4/p4_table_map.pb.h
  ${PB_OUT_DIR}/stratum/hal/lib/p4/p4_table_map.pb.cc
)

target_include_directories(p4_table_map_proto_o PUBLIC ${PB_OUT_DIR})

add_dependencies(p4_table_map_proto_o
  common_flow_entry_proto_o
  p4_annotation_proto_o
  p4_table_defs_proto_o
  p4info_proto_o
)

###################
# phal_db_proto_o #
###################

add_library(phal_db_proto_o OBJECT
  ${PB_OUT_DIR}/stratum/hal/lib/phal/db.pb.cc
  ${PB_OUT_DIR}/stratum/hal/lib/phal/db.pb.h
)

target_include_directories(phal_db_proto_o PUBLIC ${PB_OUT_DIR})

add_dependencies(phal_db_proto_o
  common_proto_o
)

#######################
# stratum_tdi_proto_o #
#######################

add_library(stratum_tdi_proto_o OBJECT
  ${PB_OUT_DIR}/stratum/hal/lib/tdi/tdi.pb.h
  ${PB_OUT_DIR}/stratum/hal/lib/tdi/tdi.pb.cc
)

target_include_directories(stratum_tdi_proto_o PUBLIC ${PB_OUT_DIR})

add_dependencies(stratum_tdi_proto_o
  p4info_proto_o
)
