# CMake build file for ovs-vswitchd
#
# Copyright 2022-2024 Intel Corporation
# SPDX-License-Identifier: Apache 2.0
#

add_executable(ovs-vswitchd dummy.cc)

set_install_rpath(ovs-vswitchd ${EXEC_ELEMENT} ${DEP_ELEMENT})

target_link_libraries(ovs-vswitchd
    PRIVATE
        -Wl,--whole-archive
        ovs::vswitchd
        -Wl,--no-whole-archive
        ovs::ofproto
        ovs::openvswitch
        ovs::sflow
    PUBLIC
        atomic
        m
        ovsp4rt
        pthread
        rt
)

set_ovs_target_properties(ovs-vswitchd)

install(TARGETS ovs-vswitchd DESTINATION sbin)
