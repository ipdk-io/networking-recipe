# CMake build file for ovs-vswitchd
#
# Copyright 2022-2024 Intel Corporation
# SPDX-License-Identifier: Apache 2.0
#

add_executable(ovs-vswitchd
    $<TARGET_OBJECTS:ovs_sidecar_o>
)

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
        pthread
        rt
)

set_ovs_target_properties(ovs-vswitchd)

target_link_libraries(ovs-vswitchd PUBLIC sde::target_sys)

install(TARGETS ovs-vswitchd DESTINATION sbin)
