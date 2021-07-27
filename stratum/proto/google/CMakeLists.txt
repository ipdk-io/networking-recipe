# Builds google protobuf libraries
#
# Copyright 2022-2024 Intel Corporation
# SPDX-License-Identifier: Apache 2.0
#

set(GRPC_PROTO_FILES
    google/rpc/status.proto
    google/rpc/code.proto
)

generate_proto_files("${GRPC_PROTO_FILES}" "${GOOGLEAPIS_SOURCE_DIR}")

# code.proto
add_library(grpc_code_proto_o OBJECT
    ${PB_OUT_DIR}/google/rpc/code.pb.cc
    ${PB_OUT_DIR}/google/rpc/code.pb.h
)

target_include_directories(grpc_code_proto_o PUBLIC ${PB_OUT_DIR})

# status.proto
add_library(grpc_status_proto_o OBJECT
    ${PB_OUT_DIR}/google/rpc/status.pb.cc
    ${PB_OUT_DIR}/google/rpc/status.pb.h
)

target_include_directories(grpc_status_proto_o PUBLIC ${PB_OUT_DIR})

#######################
# Build libgrpc_proto #
#######################

add_library(grpc_proto SHARED
    $<TARGET_OBJECTS:grpc_code_proto_o>
    $<TARGET_OBJECTS:grpc_status_proto_o>
)

target_link_libraries(grpc_proto PUBLIC protobuf::libprotobuf)

set_install_rpath(grpc_proto ${DEP_ELEMENT})

install(TARGETS grpc_proto LIBRARY)
