# CMake build file for ovs-testcontroller
#
# Copyright 2022-2024 Intel Corporation
# SPDX-License-Identifier: Apache 2.0
#

add_executable(ovs-testcontroller
    $<TARGET_OBJECTS:ovs_sidecar_o>
)

set_install_rpath(ovs-testcontroller ${EXEC_ELEMENT} ${DEP_ELEMENT})

target_link_libraries(ovs-testcontroller
    PRIVATE
        -Wl,--whole-archive
        ovs::testcontroller
        -Wl,--no-whole-archive
        ovs::ofproto
        ovs::openvswitch
        ovs::sflow
        ovs::vswitchd
    PUBLIC
        atomic
        rt
)

set_ovs_target_properties(ovs-testcontroller)

install(TARGETS ovs-testcontroller DESTINATION bin)
