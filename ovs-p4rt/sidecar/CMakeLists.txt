# CMake build file for ovs-p4rt/sidecar
#
# Copyright 2022-2024 Intel Corporation
# SPDX-License-Identifier: Apache 2.0
#

add_library(ovs_sidecar_o OBJECT
    ${OVSP4RT_INCLUDE_DIR}/ovsp4rt/ovs-p4rt.h
)

target_include_directories(ovs_sidecar_o
    PUBLIC
        ${OVSP4RT_INCLUDE_DIR}
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${STRATUM_SOURCE_DIR}
)

add_subdirectory(common)

if(DPDK_TARGET)
    add_subdirectory(dpdk)
elseif(ES2K_TARGET)
    add_subdirectory(es2k)
endif()

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

install(
    DIRECTORY
        ${OVSP4RT_INCLUDE_DIR}/ovsp4rt
    DESTINATION
        ${CMAKE_INSTALL_INCLUDEDIR}
)

###########
# testing #
###########

set(SIDECAR_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})

add_subdirectory(testing EXCLUDE_FROM_ALL)
