# modules/clients/gnmi-ctl
#
# Copyright 2022 Intel Corporation
# SPDX-License-Identifier: Apache 2.0
#
cmake_minimum_required(VERSION 3.5)

##################
# Build gnmi-ctl #
##################

add_executable(gnmi-ctl
    gnmi_ctl.cc
    gnmi_ctl_utils.c
    gnmi_ctl_utils.h
)

target_link_libraries(gnmi-ctl
    PUBLIC
        stratum
        gnmi_proto rpc_proto
        grpc protobuf gflags grpc++
        pthread re2
)

target_include_directories(gnmi-ctl
    PRIVATE
        ${STRATUM_SOURCE_DIR}
        ${PROTO_OUT_DIR}
)

add_dependencies(gnmi-ctl
    gnmi_proto
    rpc_proto
)

install(TARGETS gnmi-ctl RUNTIME)
