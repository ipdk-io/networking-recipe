# CMake build file for modules directory
#
# Copyright 2022 Intel Corporation
# SPDX-License-Identifier: Apache 2.0

cmake_minimum_required(VERSION 3.5)

# Define PYTHON3 variable.
# We do this before updating CMAKE_PREFIX_PATH, so we find the
# system's interpreter rather than the one in the SDE path.
find_program(PYTHON3 python3)
if(NOT PYTHON3)
    include(FindPython)
    find_package(Python COMPONENTS Interpreter)
    set(PYTHON3 ${Python_EXECUTABLE})
endif()

# Update pkg-config search path.
set(PKG_CONFIG_USE_CMAKE_PREFIX_PATH TRUE)
list(APPEND CMAKE_PREFIX_PATH
    ${OVS_INSTALL_PREFIX}
    ${SDE_INSTALL_PREFIX}
)

#if(DPDK_TARGET)
#    pkg_check_modules(DPDK REQUIRED libdpdk)
#endif()

if(WITH_STRATUM)
    add_subdirectory(clients)
endif()
