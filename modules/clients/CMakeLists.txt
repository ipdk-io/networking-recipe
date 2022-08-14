# p4ovs/clients
#
# Copyright 2022 Intel Corporation
# SPDX-License-Identifier: Apache 2.0
#
cmake_minimum_required(VERSION 3.5)

# TODO: Eliminate dependencies on libovs, stratum, and dpdk.
# TODO: Find out what properties can be made transitive.

##################
# Build gnmi_cli #
##################

add_executable(gnmi_cli
    gnmi_cli.cc
    client_parse_key_value.c
    client_parse_key_value.h
    client_strzcpy.c
    client_strzcpy.h
)

target_link_libraries(gnmi_cli
    PUBLIC
        grpc protobuf gflags grpc++
        atomic pthread unwind
        absl_synchronization
        gnmi_proto rpc_proto
)

if(HAVE_POSIX_AIO)
    target_link_libraries(gnmi_cli PUBLIC rt)
endif()

target_include_directories(gnmi_cli
    PRIVATE
        ${STRATUM_SOURCE_DIR}
        ${PROTO_OUT_DIR}
)

add_dependencies(gnmi_cli
    gnmi_proto
    rpc_proto
)

install(TARGETS gnmi_cli RUNTIME)

##############################
# Build tdi_pipeline_builder #
##############################

add_executable(tdi_pipeline_builder
    ${STRATUM_SOURCE_DIR}/stratum/hal/bin/tdi/tdi_pipeline_builder.cc
)

target_include_directories(tdi_pipeline_builder
    PRIVATE
        ${STRATUM_SOURCE_DIR}
        ${PROTO_OUT_DIR}
#       ${OVS_INSTALL_PREFIX}/include
)

if(DPDK_TARGET)
    target_link_directories(tdi_pipeline_builder PRIVATE ${DPDK_LIBRARY_DIRS})
    target_link_options(tdi_pipeline_builder PRIVATE ${DPDK_LDFLAGS_OTHER})
endif()

target_link_libraries(tdi_pipeline_builder
    PRIVATE
        stratum
        protobuf gflags glog grpc++
        absl_strings
)

add_dependencies(tdi_pipeline_builder
    stratum
    p4v1_proto
    stratum_proto
)

install(TARGETS tdi_pipeline_builder RUNTIME)
