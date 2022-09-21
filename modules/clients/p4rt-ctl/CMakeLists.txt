# Build file for ovs-clients.
#
# Copyright 2022 Intel Corporation
# SPDX-License-Identifier: Apache 2.0
#
cmake_minimum_required(VERSION 3.5)

set(P4RT_CTL_OUT ${CMAKE_CURRENT_BINARY_DIR}/p4rt-ctl)

configure_file(p4rt-ctl.in ${P4RT_CTL_OUT} @ONLY)

add_custom_target(ovs-p4ctl ALL DEPENDS ${P4RT_CTL_OUT})
set_source_files_properties(${P4RT_CTL_OUT} PROPERTIES GENERATED TRUE)

install(PROGRAMS ${P4RT_CTL_OUT} DESTINATION bin)
