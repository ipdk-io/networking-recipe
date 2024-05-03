# CMake build file for ovs-p4rt/sidecar
#
# Copyright 2022-2024 Intel Corporation
# SPDX-License-Identifier: Apache 2.0
#

add_library(ovs_sidecar_o OBJECT)

add_subdirectory(common)
add_subdirectory(lib)

if(DPDK_TARGET)
    add_subdirectory(dpdk)
elseif(ES2K_TARGET)
    add_subdirectory(es2k)
endif()

target_include_directories(ovs_sidecar_o PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${OVS_INCLUDE_DIR}
    ${STRATUM_SOURCE_DIR}
)

target_link_libraries(ovs_sidecar_o PUBLIC
    absl::strings
    p4_role_config_proto
    p4runtime_proto
    sde::target_sys
)

###############
# ovs_sidecar #
###############

add_library(ovs_sidecar SHARED EXCLUDE_FROM_ALL
    $<TARGET_OBJECTS:ovs_sidecar_o>
)

target_link_libraries(ovs_sidecar PUBLIC
    absl::strings
    p4_role_config_proto
    p4runtime_proto
    sde::target_sys
)

add_dependencies(ovs_sidecar ovs_sidecar_o)
