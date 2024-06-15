# CMake build file for ovs-p4rt/sidecar
#
# Copyright 2022-2024 Intel Corporation
# SPDX-License-Identifier: Apache 2.0
#

set(SIDECAR_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})

#################
# ovsp4rt_utils #
#################

add_library(ovsp4rt_utils STATIC
  ${OVSP4RT_INCLUDE_DIR}/ovsp4rt/ovs-p4rt.h
  ovsp4rt_utils.cc
  ovsp4rt_utils.h
)

target_include_directories(ovsp4rt_utils PUBLIC
  ${OVSP4RT_INCLUDE_DIR}
  ${SIDECAR_SOURCE_DIR}
)

target_link_libraries(ovsp4rt_utils PUBLIC
  p4runtime_proto
)

#################
# ovs_sidecar_o #
#################

add_library(ovs_sidecar_o OBJECT
    ${OVSP4RT_INCLUDE_DIR}/ovsp4rt/ovs-p4rt.h
    ovsp4rt.cc
    ovsp4rt_credentials.cc
    ovsp4rt_credentials.h
    ovsp4rt_session.cc
    ovsp4rt_session.h
)

add_subdirectory(encode)

if(DPDK_TARGET)
    add_subdirectory(dpdk)
elseif(ES2K_TARGET)
    add_subdirectory(es2k)
endif()

target_include_directories(ovs_sidecar_o PUBLIC
    ${OVSP4RT_INCLUDE_DIR}
    ${SIDECAR_SOURCE_DIR}
    ${STRATUM_SOURCE_DIR}
)

target_link_libraries(ovs_sidecar_o PUBLIC
    ovsp4rt_utils
    p4_role_config_proto
    p4runtime_proto
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

set_install_rpath(ovs_sidecar $ORIGIN ${SDE_ELEMENT} ${DEP_ELEMENT})

###########
# testing #
###########

add_subdirectory(testing EXCLUDE_FROM_ALL)
