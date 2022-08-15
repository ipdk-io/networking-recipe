# CMake build file for superproject.
#
# Copyright 2022 Intel Corporation
# SPDX-License-Identifier: Apache 2.0
#

cmake_minimum_required(VERSION 3.5)

project(ovs-with-p4 VERSION 0.1 LANGUAGES C CXX)

include(CMakePrintHelpers)

set(LT_CURRENT "0" CACHE STRING "Current version number for linker symbols")

#############################
# Symbolic path definitions #
#############################

set(DEPEND_INSTALL_PREFIX "" CACHE PATH "Dependencies install directory")

set(OVS_INSTALL_PREFIX "${CMAKE_SOURCE_DIR}/ovs/install" CACHE PATH
    "OVS install directory")

set(OVS_SOURCE_DIR "${CMAKE_SOURCE_DIR}/ovs/ovs" CACHE PATH
    "OVS source directory")

set(SDE_INSTALL_PREFIX "/opt/sde" CACHE PATH "SDE install directory")

option(WITH_OPENCONFIG  "Build with OpenConfig" OFF)
option(WITH_STRATUM     "Build with Stratum" OFF)

############################
# Target selection options #
############################

option(TOFINO_TARGET    "Build for Tofino target" OFF)
option(MEV_TARGET       "Build for MEV target" OFF)
option(DPDK_TARGET      "Build for DPDK target" OFF)

# Translate selected option to TARGETFLAG string.
# - Use precedence to deal with multiple selections.
# - Ensure that individual target options are consistent
#   so we can use them to control the build.
# - Default to DPDK_TARGET.
if(TOFINO_TARGET)
    set(TARGETFLAG TOFINO_TARGET)
    set(DPDK_TARGET OFF)
    set(MEV_TARGET OFF)
elseif(MEV_TARGET)
    set(TARGETFLAG MEV_TARGET)
    set(DPDK_TARGET OFF)
    set(TOFINO_TARGET OFF)
else()
    set(TARGETFLAG DPDK_TARGET)
    set(DPDK_TARGET ON)
    set(MEV_TARGET OFF)
    set(TOFINO_TARGET OFF)
endif()

if(DPDK_TARGET)
    set(SOFT_TARGET ON)
endif()

###########################
# Global compiler options #
###########################

add_compile_options(-D${TARGETFLAG})
add_compile_options(-DIPDK_TARGET)
add_compile_options(-DTDI_TARGET)
add_compile_options(-DP4OVS_CHANGES)

if(DPDK_TARGET)
    add_compile_options(-DSOFT_TARGET)
endif()

if(DEPEND_INSTALL_PREFIX)
  include_directories(${DEPEND_INSTALL_PREFIX}/include)
  link_directories(${DEPEND_INSTALL_PREFIX}/lib)
endif()

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

if(WITH_STRATUM)
  add_subdirectory(stratum)
endif()
add_subdirectory(modules)
