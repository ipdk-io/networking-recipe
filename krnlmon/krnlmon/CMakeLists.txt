# CMake build file for Kernel Monitor
#
# Copyright 2022-2025 Intel Corporation
# SPDX-License-Identifier: Apache 2.0
#

# CMake version 3.16 is the baseline for P4 Control Plane
cmake_minimum_required(VERSION 3.16)

project(krnlmon VERSION 2.0 LANGUAGES C CXX)

include(CMakePrintHelpers)
include(FindPkgConfig)
include(cmake/iwyu.cmake)

if(CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
  include(cmake/standalone.cmake)
endif()

if(SAI_SOURCE_DIR STREQUAL "")
  message(FATAL_ERROR "SAI_SOURCE_DIR not defined!")
endif()

set(SAI_INCLUDE_DIR ${SAI_SOURCE_DIR}/inc)
if(NOT EXISTS ${SAI_INCLUDE_DIR}/sai.h)
  message(FATAL_ERROR "sai.h not found")
endif()

# Find netlink libraries
pkg_check_modules(nl3 REQUIRED IMPORTED_TARGET libnl-3.0)
pkg_check_modules(nl3-route REQUIRED IMPORTED_TARGET libnl-route-3.0)
include_directories(${nl3_INCLUDE_DIRS})

add_compile_options(-Werror)

include_directories(.)

# Set the "krnlmon" label on all tests in this directory.
set_property(DIRECTORY PROPERTY LABELS krnlmon)

if(BUILD_TESTING)
  include(cmake/testing.cmake)
endif()

add_subdirectory(switchapi)
add_subdirectory(switchlink)
add_subdirectory(switchsai)
add_subdirectory(switchsde)
add_subdirectory(switchutils)

##############
# libkrnlmon #
##############

add_library(krnlmon SHARED
  $<TARGET_OBJECTS:switchapi_o>
  $<TARGET_OBJECTS:switchlink_o>
  $<TARGET_OBJECTS:switchlink_sai_o>
  $<TARGET_OBJECTS:switchsai_o>
  $<TARGET_OBJECTS:switchsde_o>
  $<TARGET_OBJECTS:switchutils_o>
  krnlmon_main.cc
  krnlmon_main.h
)

# Required for Ninja.
set_target_properties(switchutils_o PROPERTIES LINKER_LANGUAGE "C")

target_link_libraries(krnlmon PkgConfig::nl3 PkgConfig::nl3-route)
target_link_libraries(krnlmon absl::synchronization)

set_install_rpath(krnlmon ${DEP_ELEMENT})

#################
# libkrnlmon.pc #
#################

# TODO: Look into using configure_package_config_file()
configure_file(libkrnlmon.pc.in
               ${CMAKE_CURRENT_BINARY_DIR}/libkrnlmon.pc @ONLY)

install(TARGETS krnlmon LIBRARY)

install(
  FILES
    ${CMAKE_CURRENT_BINARY_DIR}/libkrnlmon.pc
  DESTINATION
    ${CMAKE_INSTALL_LIBDIR}/pkgconfig
)

#################
# dummy_krnlmon #
#################

add_executable(dummy_krnlmon EXCLUDE_FROM_ALL
  dummy_main.cc
)

target_link_libraries(dummy_krnlmon PUBLIC krnlmon)

if(DPDK_TARGET)
  add_dpdk_target_libraries(dummy_krnlmon)
elseif(ES2K_TARGET)
  add_es2k_target_libraries(dummy_krnlmon)
endif()
