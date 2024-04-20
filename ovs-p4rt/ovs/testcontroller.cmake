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

if(TARGET ovs::avx512)
    target_link_libraries(ovs-testcontroller PUBLIC ovs::avx512)
endif()

if(TARGET unbound)
    target_link_libraries(ovs-testcontroller PUBLIC unbound)
endif()

if(TARGET unwind::unwind)
    target_link_libraries(ovs-testcontroller PUBLIC unwind::unwind)
endif()

target_link_libraries(ovs-testcontroller PUBLIC
    absl::strings
    absl::statusor
    absl::flags_private_handle_accessor
    absl::flags
)

target_link_libraries(ovs-testcontroller PUBLIC stratum_static)

target_link_libraries(ovs-testcontroller PUBLIC
    p4_role_config_proto
    p4runtime_proto
    stratum_proto
)

target_link_libraries(ovs-testcontroller PUBLIC sde::target_sys)

install(TARGETS ovs-testcontroller DESTINATION bin)
