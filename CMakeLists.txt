# CMake build file for superproject.
#
# Copyright 2022 Intel Corporation
# SPDX-License-Identifier: Apache 2.0
#

cmake_minimum_required(VERSION 3.5)

project(networking-recipe VERSION 0.1 LANGUAGES C CXX)

include(FindPkgConfig)
include(CMakePrintHelpers)

# Default: Release with Debug Info
set(CMAKE_BUILD_TYPE "RelWithDebInfo")

#############################
# Symbolic path definitions #
#############################

set(DEPEND_INSTALL_DIR "" CACHE PATH "Dependencies install directory")

set(OVS_INSTALL_DIR "${CMAKE_INSTALL_PREFIX}" CACHE PATH
    "OVS install directory")

set(OVS_SOURCE_DIR "${CMAKE_SOURCE_DIR}/ovs/ovs" CACHE PATH
    "OVS source directory")

set(SDE_INSTALL_DIR "/opt/sde" CACHE PATH "SDE install directory")

set(PROTO_OUT_DIR "${CMAKE_BINARY_DIR}/pb.out" CACHE PATH
    "Path to generated Protobuf files")
file(MAKE_DIRECTORY ${PROTO_OUT_DIR})

############################
# Target selection options #
############################

option(DPDK_TARGET      "Build for DPDK target" OFF)
option(TOFINO_TARGET    "Build for Tofino target" OFF)

# Translate target option to TARGETFLAG string.
# - Use precedence to deal with multiple selections.
# - Ensure that individual target options are consistent
#   so we can use them to control the build.
# - Default to TOFINO_TARGET.
set(_target_action "Building")
set(_default_target "DPDK_TARGET")
if(TOFINO_TARGET)
    set(TARGETFLAG TOFINO_TARGET)
    set(DPDK_TARGET OFF)
elseif(DPDK_TARGET)
    set(TARGETFLAG DPDK_TARGET)
else()
    set(TARGETFLAG ${_default_target})
    set(${_default_target} ON)
    set(_target_action "Defaulting to")
endif()

message(NOTICE "${_target_action} ${TARGETFLAG}")

unset(_default_target)
unset(_target_action)

###################
# Feature toggles #
###################

if(DPDK_TARGET)
    set(WITH_KRNLMON ON)
    set(WITH_OVSP4RT ON)
elseif(TOFINO_TARGET)
    set(WITH_KRNLMON OFF)
    set(WITH_OVSP4RT OFF)
endif()

cmake_print_variables(WITH_KRNLMON)
cmake_print_variables(WITH_OVSP4RT)

###########################
# Global compiler options #
###########################

add_compile_options(-D${TARGETFLAG})

if(DEPEND_INSTALL_DIR)
    include_directories(${DEPEND_INSTALL_DIR}/include)
    link_directories(${DEPEND_INSTALL_DIR}/lib)
endif()

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

#####################
# CMAKE_PREFIX_PATH #
#####################

# CMAKE_PREFIX_PATH specifies the directories to be searched by
# find_file(), find_library(), find_path(), and find_program().

if(NOT DEPEND_INSTALL_DIR STREQUAL "")
    list(APPEND CMAKE_PREFIX_PATH ${DEPEND_INSTALL_DIR})
endif()

list(APPEND CMAKE_PREFIX_PATH ${SDE_INSTALL_DIR})

if(NOT OVS_INSTALL_DIR STREQUAL "")
    list(APPEND CMAKE_PREFIX_PATH ${OVS_INSTALL_DIR})
endif()

list(REMOVE_DUPLICATES CMAKE_PREFIX_PATH)
cmake_print_variables(CMAKE_PREFIX_PATH)

#####################
# External packages #
#####################

find_package(absl REQUIRED)

if(DPDK_TARGET)
    pkg_check_modules(DPDK REQUIRED libdpdk)
endif()

##################
# Subdirectories #
##################

if(WITH_KRNLMON)
    add_subdirectory(krnlmon)
endif()

add_subdirectory(stratum)
add_subdirectory(infrap4d)
add_subdirectory(clients)

if(WITH_OVSP4RT)
    add_subdirectory(ovs-p4rt)
endif()
