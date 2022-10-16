# modules/clients/gnmi_cli
#
# Copyright 2022 Intel Corporation
# SPDX-License-Identifier: Apache 2.0
#
cmake_minimum_required(VERSION 3.5)

##################
# Build gnmi_cli #
##################

if(DPDK_TARGET)
    set(GNMI_CLI_SOURCES
        gnmi_cli.cc
        gnmi_cli_utils.c
        gnmi_cli_utils.h
    )
else()
    set(GNMI_CLI_SOURCES
        ${STRATUM_SOURCE_DIR}/stratum/tools/gnmi/gnmi_cli.cc
    )

endif()

add_executable(gnmi_cli ${GNMI_CLI_SOURCES})

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
