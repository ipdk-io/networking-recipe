# CMake build file for ovs-p4rt/sidecar/stubs
#
# Copyright 2024 Intel Corporation
# SPDX-License-Identifier: Apache 2.0
#

#-----------------------------------------------------------------------
# libovsp4rt_stubs.a
#-----------------------------------------------------------------------
add_library(ovsp4rt_stubs STATIC
    ovsp4rt_stubs.cc
)

target_include_directories(ovsp4rt_stubs PUBLIC
    ${OVSP4RT_INCLUDE_DIR}
)

install(
    TARGETS ovsp4rt_stubs
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

set(ovsp4rt_stubs_cfg_file ${CMAKE_CURRENT_BINARY_DIR}/libovsp4rt_stubs.pc)
configure_file(libovsp4rt_stubs.pc.in ${ovsp4rt_stubs_cfg_file} @ONLY)

install(
    FILES ${ovsp4rt_stubs_cfg_file}
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig
)
