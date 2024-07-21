# Build file for sgnmi_cli.
#
# Copyright 2023-2024 Intel Corporation
# SPDX-License-Identifier: Apache 2.0
#

###################
# Build sgnmi_cli #
###################

add_executable(sgnmi_cli
    sgnmi_cli.cc
)

set_install_rpath(sgnmi_cli ${EXEC_ELEMENT} ${DEP_ELEMENT})

target_link_libraries(sgnmi_cli PRIVATE client_cert_options)

target_link_libraries(sgnmi_cli
    PUBLIC
        stratum_static
        gnmi_proto
        google_rpc_proto
        gflags::gflags_shared
        gRPC::grpc
        gRPC::grpc++
        gRPC::re2
        protobuf::libprotobuf
        pthread
)

if(HAVE_POSIX_AIO)
    target_link_libraries(sgnmi_cli PUBLIC rt)
endif()

target_include_directories(sgnmi_cli
    PRIVATE
        ${STRATUM_SOURCE_DIR}
        ${PB_OUT_DIR}
)

install(TARGETS sgnmi_cli RUNTIME)
