# Builds google protobuf libraries
#
# Copyright 2022-2024 Intel Corporation
# SPDX-License-Identifier: Apache 2.0
#

set(RPC_PROTO_FILES
    google/rpc/status.proto
    google/rpc/code.proto
)

generate_proto_files("${RPC_PROTO_FILES}" "${GOOGLEAPIS_SOURCE_DIR}")

# code.proto
add_library(google_rpc_code_proto_o OBJECT
    ${PB_OUT_DIR}/google/rpc/code.pb.cc
    ${PB_OUT_DIR}/google/rpc/code.pb.h
)

target_include_directories(google_rpc_code_proto_o PUBLIC ${PB_OUT_DIR})

# status.proto
add_library(google_rpc_status_proto_o OBJECT
    ${PB_OUT_DIR}/google/rpc/status.pb.cc
    ${PB_OUT_DIR}/google/rpc/status.pb.h
)

target_include_directories(google_rpc_status_proto_o PUBLIC ${PB_OUT_DIR})

####################
# google_rpc_proto #
####################

add_library(google_rpc_proto SHARED
    $<TARGET_OBJECTS:google_rpc_code_proto_o>
    $<TARGET_OBJECTS:google_rpc_status_proto_o>
)

target_link_libraries(google_rpc_proto PUBLIC protobuf::libprotobuf)

set_install_rpath(google_rpc_proto ${DEP_ELEMENT})

install(TARGETS google_rpc_proto LIBRARY)
