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
        rt m pthread
)

if(TARGET ovs::avx512)
    target_link_libraries(ovs-vswitchd PUBLIC ovs::avx512)
endif()

if(TARGET unbound)
    target_link_libraries(ovs-vswitchd PUBLIC unbound)
endif()

if(TARGET unwind::unwind)
    target_link_libraries(ovs-vswitchd PUBLIC unwind::unwind)
endif()

target_link_libraries(ovs-vswitchd PUBLIC
    absl::strings
    absl::statusor
    absl::flags_private_handle_accessor
    absl::flags
)

target_link_libraries(ovs-vswitchd PUBLIC stratum_static p4_role_config_proto)

target_link_libraries(ovs-vswitchd PUBLIC
    stratum_proto
    p4runtime_proto
)

install(TARGETS ovs-vswitchd DESTINATION sbin)
