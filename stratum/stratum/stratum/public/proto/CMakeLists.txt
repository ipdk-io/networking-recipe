# Build file for //stratum/public/proto
#
# Copyright 2024 Intel Corporation
# SPDX-License-Identifier: Apache 2.0
#

##############################
# Build p4_role_config_proto #
##############################

generate_proto_files(
  "stratum/public/proto/p4_role_config.proto"
  "${STRATUM_SOURCE_DIR}"
)

add_library(p4_role_config_proto SHARED
  ${PB_OUT_DIR}/stratum/public/proto/p4_role_config.pb.cc
  ${PB_OUT_DIR}/stratum/public/proto/p4_role_config.pb.h
)

target_include_directories(p4_role_config_proto PUBLIC ${PB_OUT_DIR})

install(TARGETS p4_role_config_proto LIBRARY)

#######################
# Build error_proto_o #
#######################

generate_proto_files(
  "stratum/public/proto/error.proto"
  "${STRATUM_SOURCE_DIR}"
)

add_library(error_proto_o OBJECT
  ${PB_OUT_DIR}/stratum/public/proto/error.pb.cc
  ${PB_OUT_DIR}/stratum/public/proto/error.pb.h
)

target_include_directories(error_proto_o PUBLIC ${PB_OUT_DIR})

###############################
# Build p4_annotation_proto_o #
###############################

generate_proto_files(
  "stratum/public/proto/p4_annotation.proto"
  "${STRATUM_SOURCE_DIR}"
)

add_library(p4_annotation_proto_o OBJECT
  ${PB_OUT_DIR}/stratum/public/proto/p4_annotation.pb.cc
  ${PB_OUT_DIR}/stratum/public/proto/p4_annotation.pb.h
)

add_dependencies(p4_annotation_proto_o p4_table_defs_proto_o)

target_include_directories(p4_annotation_proto_o PUBLIC ${PB_OUT_DIR})

###############################
# Build p4_table_defs_proto_o #
###############################

generate_proto_files(
  "stratum/public/proto/p4_table_defs.proto"
  "${STRATUM_SOURCE_DIR}"
)

add_library(p4_table_defs_proto_o OBJECT
  ${PB_OUT_DIR}/stratum/public/proto/p4_table_defs.pb.cc
  ${PB_OUT_DIR}/stratum/public/proto/p4_table_defs.pb.h
)

target_include_directories(p4_table_defs_proto_o PUBLIC ${PB_OUT_DIR})
