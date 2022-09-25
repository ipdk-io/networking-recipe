# CMake build file for superproject.
#
# Copyright 2022 Intel Corporation
# SPDX-License-Identifier: Apache 2.0
#

cmake_minimum_required(VERSION 3.5)

project(networking-recipe VERSION 0.1 LANGUAGES C CXX)

include(CMakePrintHelpers)

set(LT_CURRENT "0" CACHE STRING "Current version number for linker symbols")

#############################
# Symbolic path definitions #
#############################

set(DEPEND_INSTALL_PREFIX "" CACHE PATH "Dependencies install directory")

set(OVS_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}" CACHE PATH
    "OVS install directory")

set(OVS_SOURCE_DIR "${CMAKE_SOURCE_DIR}/ovs/ovs" CACHE PATH
    "OVS source directory")

set(SDE_INSTALL_PREFIX "/opt/sde" CACHE PATH "SDE install directory")

#################
# Build options #
#################

option(WITH_STRATUM     "Build with Stratum" ON)
option(WITH_OPENCONFIG  "Build with OpenConfig" ${WITH_STRATUM})

############################
# Target selection options #
############################

option(TOFINO_TARGET    "Build for Tofino target" OFF)
option(DPDK_TARGET      "Build for DPDK target" OFF)
option(IPDK_TARGET      "Build Stratum for IPDK" OFF)

# Translate selected option to TARGETFLAG string.
# - Use precedence to deal with multiple selections.
# - Ensure that individual target options are consistent
#   so we can use them to control the build.
# - Default to TOFINO_TARGET.
if(TOFINO_TARGET)
    set(TARGETFLAG TOFINO_TARGET)
    set(DPDK_TARGET OFF)
elseif(DPDK_TARGET)
    set(TARGETFLAG DPDK_TARGET)
else()
    set(TARGETFLAG TOFINO_TARGET)
endif()

message(NOTICE "Building ${TARGETFLAG}")

###########################
# Global compiler options #
###########################

add_compile_options(-D${TARGETFLAG})
add_compile_options(-DP4OVS_CHANGES)

if(DEPEND_INSTALL_PREFIX)
  include_directories(${DEPEND_INSTALL_PREFIX}/include)
  link_directories(${DEPEND_INSTALL_PREFIX}/lib)
endif()

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

if(WITH_STRATUM)
  add_subdirectory(stratum)
endif()
add_subdirectory(infrap4d)
add_subdirectory(modules)
