# CMake build file for ovs-p4rt/sidecar
#
# Copyright 2022-2024 Intel Corporation
# SPDX-License-Identifier: Apache 2.0
#

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

add_subdirectory(lib)

if(DPDK_TARGET)
    add_subdirectory(dpdk)
elseif(ES2K_TARGET)
    add_subdirectory(es2k)
endif()

target_include_directories(ovs_sidecar_o PUBLIC
    ${OVSP4RT_INCLUDE_DIR}
    ${STRATUM_SOURCE_DIR}
)

target_link_libraries(ovs_sidecar_o PUBLIC
    p4_role_config_proto
    p4runtime_proto
    sde::target_sys
)

#################
# libovsp4rt.so #
#################

add_library(ovsp4rt SHARED
    $<TARGET_OBJECTS:ovs_sidecar_o>
)

target_link_libraries(ovsp4rt PRIVATE stratum_static)

target_link_libraries(ovsp4rt PUBLIC
    p4_role_config_proto
    stratum_proto
    p4runtime_proto
)

target_link_libraries(ovsp4rt PUBLIC
    absl::strings
    absl::statusor
    absl::flags_private_handle_accessor
    absl::flags
)

add_dependencies(ovsp4rt ovs_sidecar_o)

set_install_rpath(ovsp4rt $ORIGIN ${DEP_ELEMENT})

install(
    TARGETS ovsp4rt
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

install(
    DIRECTORY
        ${OVSP4RT_INCLUDE_DIR}/ovsp4rt
    DESTINATION
        ${CMAKE_INSTALL_INCLUDEDIR}
)

set_install_rpath(ovs_sidecar $ORIGIN ${SDE_ELEMENT} ${DEP_ELEMENT})

#################
# libovsp4rt.pc #
#################

set(pkg_config_file ${CMAKE_CURRENT_BINARY_DIR}/libovsp4rt.pc)

configure_file(libovsp4rt.pc.in ${pkg_config_file} @ONLY)

install(
  FILES ${pkg_config_file}
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig
)

###########
# testing #
###########

set(SIDECAR_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})

add_subdirectory(testing EXCLUDE_FROM_ALL)
