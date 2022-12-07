# CMake build file for superproject.
#
# Copyright 2022 Intel Corporation
# SPDX-License-Identifier: Apache 2.0
#

cmake_minimum_required(VERSION 3.5)

project(networking-recipe VERSION 0.1 LANGUAGES C CXX)

include(FindPkgConfig)
include(CMakePrintHelpers)

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

#################
# Build options #
#################

option(WITH_STRATUM     "Build with Stratum" ON)
option(WITH_OVSP4RT     "Build with OVS P4Runtime client" OFF)

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

###########################
# Global compiler options #
###########################

add_compile_options(-D${TARGETFLAG})
if(DPDK_TARGET)
    add_compile_options(-DP4OVS_CHANGES)
endif()

if(DEPEND_INSTALL_DIR)
    include_directories(${DEPEND_INSTALL_DIR}/include)
    link_directories(${DEPEND_INSTALL_DIR}/lib)
endif()

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

#####################
# External packages #
#####################

if(DEPEND_INSTALL_DIR)
    list(APPEND CMAKE_PREFIX_PATH ${DEPEND_INSTALL_DIR})
endif()
list(APPEND CMAKE_PREFIX_PATH ${SDE_INSTALL_DIR})

if(WITH_STRATUM)
    find_package(absl REQUIRED)
endif()

if(DPDK_TARGET)
  pkg_check_modules(DPDK REQUIRED libdpdk)
endif()

##################
# Subdirectories #
##################

if(DPDK_TARGET)
    set(WITH_KRNLMON ON)
    set(WITH_OVSP4RT ON)
endif()

if(WITH_KRNLMON)
    add_subdirectory(krnlmon)
endif()

if(WITH_STRATUM)
    add_subdirectory(stratum)
    add_subdirectory(clients)
endif()

add_subdirectory(infrap4d)
if(WITH_OVSP4RT)
    add_subdirectory(ovs-p4rt)
endif()
