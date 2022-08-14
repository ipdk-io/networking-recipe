# Build file for ovs-clients.
#
# Copyright 2022 Intel Corporation
# SPDX-License-Identifier: Apache 2.0
#
cmake_minimum_required(VERSION 3.5)

set(OVS_P4CTL_OUT ${CMAKE_CURRENT_BINARY_DIR}/ovs-p4ctl)

configure_file(ovs-p4ctl.in ${OVS_P4CTL_OUT} @ONLY)

add_custom_target(ovs-p4ctl ALL DEPENDS ${OVS_P4CTL_OUT})
set_source_files_properties(${OVS_P4CTL_OUT} PROPERTIES GENERATED TRUE)

install(PROGRAMS ${OVS_P4CTL_OUT} DESTINATION bin)
