# modules/clients
#
# Copyright 2022 Intel Corporation
# SPDX-License-Identifier: Apache 2.0
#
cmake_minimum_required(VERSION 3.5)

add_subdirectory(gnmi-ctl)
add_subdirectory(p4rt-ctl)

##################
# Build gnmi_cli #
##################

add_executable(gnmi_cli
    ${STRATUM_SOURCE_DIR}/stratum/tools/gnmi/gnmi_cli.cc
)

target_link_libraries(gnmi_cli
    PUBLIC
        stratum
        stratum_proto
        p4v1_proto
        gnmi_proto rpc_proto
        grpc protobuf gflags glog grpc++
        pthread re2
)

set_target_properties(gnmi_cli PROPERTIES
    INSTALL_RPATH ${EXEC_ORIGIN_SDE_AND_DEPS}
    INSTALL_RPATH_USE_LINK_PATH FALSE
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

if(NOT DPDK_TARGET)
    install(TARGETS gnmi_cli RUNTIME)
endif()

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

target_link_libraries(tdi_pipeline_builder PRIVATE stratum)

target_link_libraries(tdi_pipeline_builder PUBLIC
    stratum_proto
    p4v1_proto
    protobuf gflags glog grpc grpc++
)

set_target_properties(tdi_pipeline_builder PROPERTIES
    INSTALL_RPATH ${EXEC_ORIGIN_SDE_AND_DEPS}
    INSTALL_RPATH_USE_LINK_PATH FALSE
)

add_dependencies(tdi_pipeline_builder stratum p4v1_proto stratum_proto)

install(TARGETS tdi_pipeline_builder RUNTIME)
