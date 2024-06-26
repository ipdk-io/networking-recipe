# CMake build file for ovs-p4rt/sidecar
#
# Copyright 2022-2024 Intel Corporation
# SPDX-License-Identifier: Apache 2.0
#

set(SIDECAR_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})

#################
# ovs_sidecar_o #
#################

add_library(ovs_sidecar_o OBJECT
    ${OVSP4RT_INCLUDE_DIR}/ovsp4rt/ovs-p4rt.h
    ovsp4rt.cc
    ovsp4rt_credentials.cc
    ovsp4rt_credentials.h
    ovsp4rt_private.h
    ovsp4rt_session.cc
    ovsp4rt_session.h
)

add_subdirectory(logging)

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
    p4_role_config_proto
    p4runtime_proto
)

#################
# libovsp4rt.so #
#################

add_library(ovsp4rt SHARED
    $<TARGET_OBJECTS:ovs_sidecar_o>
    $<TARGET_OBJECTS:ovsp4rt_logging_o>
)

target_link_libraries(ovsp4rt PUBLIC
    p4_role_config_proto
    p4runtime_proto
)

target_link_libraries(ovsp4rt PUBLIC
    absl::statusor
    absl::flags_private_handle_accessor
    absl::flags
)

set_install_rpath(ovsp4rt $ORIGIN ${DEP_ELEMENT})

#-----------------------------------------------------------------------
# libovsp4rt_spies.a
#-----------------------------------------------------------------------
add_subdirectory(spies)

#-----------------------------------------------------------------------
# Install
#-----------------------------------------------------------------------
install(
    TARGETS ovsp4rt ovsp4rt_spies
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

install(
    DIRECTORY
        ${OVSP4RT_INCLUDE_DIR}/ovsp4rt
    DESTINATION
        ${CMAKE_INSTALL_INCLUDEDIR}
)

#-----------------------------------------------------------------------
# pkg-config files
#-----------------------------------------------------------------------
set(ovsp4rt_cfg_file ${CMAKE_CURRENT_BINARY_DIR}/libovsp4rt.pc)
configure_file(cmake/libovsp4rt.pc.in ${ovsp4rt_cfg_file} @ONLY)

set(ovsp4rt_spies_cfg_file ${CMAKE_CURRENT_BINARY_DIR}/libovsp4rt_spies.pc)
configure_file(cmake/libovsp4rt_spies.pc.in ${ovsp4rt_spies_cfg_file} @ONLY)

install(
    FILES
        ${ovsp4rt_cfg_file}
        ${ovsp4rt_spies_cfg_file}
    DESTINATION
        ${CMAKE_INSTALL_LIBDIR}/pkgconfig
)

#-----------------------------------------------------------------------
# Testing
#-----------------------------------------------------------------------
if(BUILD_TESTING)
    add_subdirectory(testing)
endif()
