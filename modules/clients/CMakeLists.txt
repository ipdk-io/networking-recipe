# modules/clients
#
# Copyright 2022 Intel Corporation
# SPDX-License-Identifier: Apache 2.0
#
cmake_minimum_required(VERSION 3.5)

add_subdirectory(p4rt-ctl)

# TODO: Eliminate dependencies on libovs, stratum, and dpdk.
# TODO: Find out what properties can be made transitive.

##################
# Build gnmi_cli #
##################

add_executable(gnmi_cli
    ${STRATUM_SOURCE_DIR}/stratum/tools/gnmi/gnmi_cli.cc
)

target_link_libraries(gnmi_cli
    PUBLIC
        stratum
        gnmi_proto rpc_proto
        grpc protobuf gflags grpc++
        pthread re2
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

# Suppress "warning: attribute ignored" on ABSL_MUST_USE_RESULT [[nodiscard]]
target_compile_options(tdi_pipeline_builder PRIVATE -Wno-attributes)

target_include_directories(tdi_pipeline_builder
    PRIVATE
        ${STRATUM_SOURCE_DIR}
        ${PROTO_OUT_DIR}
)

if(DPDK_TARGET)
    target_link_directories(tdi_pipeline_builder PRIVATE ${DPDK_LIBRARY_DIRS})
    target_link_options(tdi_pipeline_builder PRIVATE ${DPDK_LDFLAGS_OTHER})
endif()

target_link_libraries(tdi_pipeline_builder PUBLIC stratum_proto p4v1_proto)
target_link_libraries(tdi_pipeline_builder PUBLIC stratum)

target_link_libraries(tdi_pipeline_builder PUBLIC
    protobuf gflags glog grpc grpc++
)

add_dependencies(tdi_pipeline_builder
    stratum
    p4v1_proto
    stratum_proto
)

install(TARGETS tdi_pipeline_builder RUNTIME)
