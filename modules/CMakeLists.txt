# CMake build file for modules directory
#
# Copyright 2022 Intel Corporation
# SPDX-License-Identifier: Apache 2.0

cmake_minimum_required(VERSION 3.5)

set(SAI_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/SAI" CACHE PATH
    "SAI source directory")

include(CheckLibraryExists)
include(FindPkgConfig)

# -------------------------------------------------------------------- #

# We should be able to move this logic to ovs-addons/CMakeLists.txt
# once krnlmon is no longer dependent on OVS.

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

include(CheckBinutilsAVX512)
include(CheckCCompilerFlag)

check_library_exists(rt aio_write /usr/lib/x86_64-linux-gnu HAVE_POSIX_AIO)
check_c_compiler_flag(-mavx512f HAVE_AVX512F)
check_binutils_avx512(HAVE_LD_AVX512_GOOD)

# Generate config.h file and add it to the include path.
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/config.h.in"
               "${CMAKE_CURRENT_BINARY_DIR}/config.h")
include_directories(${CMAKE_CURRENT_BINARY_DIR})

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

# Find external libraries and define linker options.
pkg_check_modules(LIBOVS REQUIRED libopenvswitch)
pkg_check_modules(OFPROTO REQUIRED libofproto)
pkg_check_modules(SFLOW REQUIRED libsflow)

# -------------------------------------------------------------------- #

if(DPDK_TARGET)
    pkg_check_modules(DPDK REQUIRED libdpdk)
endif()

if(WITH_STRATUM)
  add_subdirectory(clients)
endif()
add_subdirectory(krnlmon)
add_subdirectory(ovs-addons)
