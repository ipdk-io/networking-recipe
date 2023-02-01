# Builds gNMI Protobuf library
#
# Copyright 2022-2023 Intel Corporation
# SPDX-License-Identifier: Apache 2.0
#

########################
# Compile .proto files #
########################

set(GNMI_PROTO_FILES
    gnmi/gnmi.proto
    gnmi/gnmi_ext.proto
)

generate_proto_files("${GNMI_PROTO_FILES}" "${PROTO_PARENT_DIR}")
generate_grpc_files("gnmi/gnmi.proto" "${PROTO_PARENT_DIR}")

#######################
# Build libgnmi_proto #
#######################

add_library(gnmi_proto SHARED
    ${PB_OUT_DIR}/gnmi/gnmi.grpc.pb.cc
    ${PB_OUT_DIR}/gnmi/gnmi.grpc.pb.h
    ${PB_OUT_DIR}/gnmi/gnmi.pb.cc
    ${PB_OUT_DIR}/gnmi/gnmi.pb.h
    ${PB_OUT_DIR}/gnmi/gnmi_ext.pb.cc
    ${PB_OUT_DIR}/gnmi/gnmi_ext.pb.h
)

# Ensure that the header files on which we depend have been generated.
#add_dependencies(gnmi_proto grpc_proto)

target_include_directories(gnmi_proto PRIVATE ${PB_OUT_DIR})

target_link_libraries(gnmi_proto
    PUBLIC
        grpc_proto
        absl::hash
        absl::strings
        absl::synchronization
        absl::time
)

set_install_rpath(gnmi_proto ${DEP_ELEMENT})

install(TARGETS gnmi_proto LIBRARY)
