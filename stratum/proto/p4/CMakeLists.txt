# Builds p4runtime protobuf libraries
#
# Copyright 2022-2024 Intel Corporation
# SPDX-License-Identifier: Apache 2.0
#

set(P4RT_PROTO_FILES
    idpf/p4info.proto
    idpf/p4runtime.proto
    p4/v1/p4data.proto
    p4/v1/p4runtime.proto
    p4/config/v1/p4info.proto
    p4/config/v1/p4types.proto
)

generate_proto_files("${P4RT_PROTO_FILES}" "${P4RUNTIME_SOURCE_DIR}/proto")
generate_grpc_files("p4/v1/p4runtime.proto" "${P4RUNTIME_SOURCE_DIR}/proto")

# p4data.proto
add_library(p4data_proto_o OBJECT
    ${PB_OUT_DIR}/p4/v1/p4data.pb.cc
    ${PB_OUT_DIR}/p4/v1/p4data.pb.h
)

target_include_directories(p4data_proto_o PUBLIC ${PB_OUT_DIR})

# p4info.proto
add_library(p4info_proto_o OBJECT
    ${PB_OUT_DIR}/idpf/p4info.pb.cc
    ${PB_OUT_DIR}/idpf/p4info.pb.h
    ${PB_OUT_DIR}/p4/config/v1/p4info.pb.cc
    ${PB_OUT_DIR}/p4/config/v1/p4info.pb.h
)

add_dependencies(p4info_proto_o p4types_proto_o)

target_include_directories(p4info_proto_o PUBLIC ${PB_OUT_DIR})

# p4types.proto
add_library(p4types_proto_o OBJECT
    ${PB_OUT_DIR}/p4/config/v1/p4types.pb.cc
    ${PB_OUT_DIR}/p4/config/v1/p4types.pb.h
)

target_include_directories(p4types_proto_o PUBLIC ${PB_OUT_DIR})

# p4runtime.proto
add_library(p4runtime_proto_o OBJECT
    ${PB_OUT_DIR}/idpf/p4runtime.pb.cc
    ${PB_OUT_DIR}/idpf/p4runtime.pb.h
    ${PB_OUT_DIR}/p4/v1/p4runtime.pb.cc
    ${PB_OUT_DIR}/p4/v1/p4runtime.pb.h
    ${PB_OUT_DIR}/p4/v1/p4runtime.grpc.pb.cc
    ${PB_OUT_DIR}/p4/v1/p4runtime.grpc.pb.h
)

target_include_directories(p4runtime_proto_o PUBLIC ${PB_OUT_DIR})

add_dependencies(p4runtime_proto_o
    grpc_status_proto_o
    p4data_proto_o
    p4info_proto_o
)

############################
# Build libp4runtime_proto #
############################

add_library(p4runtime_proto SHARED
    $<TARGET_OBJECTS:p4data_proto_o>
    $<TARGET_OBJECTS:p4info_proto_o>
    $<TARGET_OBJECTS:p4runtime_proto_o>
    $<TARGET_OBJECTS:p4types_proto_o>
)

target_link_libraries(p4runtime_proto
    PUBLIC
        absl::synchronization
        gRPC::grpc
        gRPC::grpc++
        grpc_proto
        protobuf::libprotobuf
)

set_install_rpath(p4runtime_proto $ORIGIN ${DEP_ELEMENT})

install(TARGETS p4runtime_proto LIBRARY)
