# Build file for gnmi-ctl.
#
# Copyright 2022-2024 Intel Corporation
# SPDX-License-Identifier: Apache 2.0
#

##################
# Build gnmi-ctl #
##################

add_executable(gnmi-ctl
    gnmi_ctl.cc
    gnmi_ctl_utils.c
    gnmi_ctl_utils.h
)

set_install_rpath(gnmi-ctl ${EXEC_ELEMENT} ${DEP_ELEMENT})

target_link_libraries(gnmi-ctl PRIVATE client_cert_options)

target_link_libraries(gnmi-ctl
    PUBLIC
        stratum_static
        gnmi_proto
        google_rpc_proto
        gflags::gflags_shared
        gRPC::grpc
        gRPC::grpc++
        protobuf::libprotobuf
        pthread
        re2::re2
)

target_include_directories(gnmi-ctl
    PRIVATE
        ${STRATUM_SOURCE_DIR}
        ${PB_OUT_DIR}
)

if(DPDK_TARGET)
    install(TARGETS gnmi-ctl RUNTIME)
endif()
