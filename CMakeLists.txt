# CMake build file for superproject.
#
# Copyright 2022-2025 Intel Corporation
# SPDX-License-Identifier: Apache 2.0
#

# Version 3.15 is the baseline for P4 Control Plane.
cmake_minimum_required(VERSION 3.15)

project(networking-recipe VERSION 3.4.0 LANGUAGES C CXX)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

include(CMakePrintHelpers)
include(CompilerSettings)
include(CTest)
include(ExternalProject)
include(FindPkgConfig)
include(GNUInstallDirs)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "RelWithDebInfo" CACHE STRING "Choose the build type"
      FORCE)
endif()

set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS
    "Debug;MinSizeRel;Release;RelWithDebInfo")

#cmake_print_variables(CMAKE_STAGING_PREFIX)
cmake_print_variables(CMAKE_SYSROOT)
cmake_print_variables(CMAKE_TOOLCHAIN_FILE)

#############################
# Symbolic path definitions #
#############################

set(DEPEND_INSTALL_DIR "$ENV{DEPEND_INSTALL}" CACHE PATH
    "Dependencies install directory")

set(OVS_INSTALL_DIR "$ENV{OVS_INSTALL}" CACHE PATH
    "OVS install directory")

set(SDE_INSTALL_DIR "$ENV{SDE_INSTALL}" CACHE PATH
    "SDE install directory")

if(SDE_INSTALL_DIR STREQUAL "")
    message(FATAL_ERROR "SDE_INSTALL_DIR (SDE_INSTALL) not defined!")
elseif(DEPEND_INSTALL_DIR STREQUAL "")
    message(FATAL_ERROR "DEPEND_INSTALL_DIR (DEPEND_INSTALL) not defined!")
endif()

set(HOST_DEPEND_DIR "" CACHE PATH "Host dependencies install directory")

set(PB_OUT_DIR "${CMAKE_BINARY_DIR}/pb.out" CACHE PATH
    "Path to generated Protobuf files")
mark_as_advanced(PB_OUT_DIR)
file(MAKE_DIRECTORY ${PB_OUT_DIR})

set(STRATUM_SOURCE_DIR
    "${CMAKE_CURRENT_SOURCE_DIR}/stratum/stratum" CACHE PATH
    "Path to Stratum source directory")
mark_as_advanced(STRATUM_SOURCE_DIR)

###################
# Feature toggles #
###################

option(IPDK_ROLE_FIX "Enable IPDK role config changes" ON)
option(SET_RPATH    "Set RPATH in libraries and executables" OFF)
option(WITH_KRNLMON "Enable Kernel Monitor support" ON)
option(WITH_OVSP4RT "Enable OVS support" ON)

############################
# Target selection options #
############################

include(SelectTdiTarget)

if(TOFINO_TARGET)
    set(WITH_KRNLMON OFF)
    set(WITH_OVSP4RT OFF)
endif()

##################
# OVS build mode #
##################

set(P4OVS_MODE "p4ovs" CACHE STRING "P4OVS build mode")
string(TOLOWER "${P4OVS_MODE}" P4OVS_MODE)

if(NOT WITH_OVSP4RT OR P4OVS_MODE STREQUAL "none")
    unset(WITH_OVSP4RT)
    set(WITH_OVSP4RT OFF CACHE STRING "" FORCE)
    set(LEGACY_P4OVS OFF)
elseif(P4OVS_MODE STREQUAL "p4ovs")
    set(LEGACY_P4OVS ON)
else()
    set(LEGACY_P4OVS OFF)
endif()

if(LEGACY_P4OVS AND OVS_INSTALL_DIR STREQUAL "")
    message(FATAL_ERROR "OVS_INSTALL_DIR (OVS_INSTALL) not defined!")
endif()

############################
# Global compiler settings #
############################

set_basic_compiler_options()
set_legacy_security_options()
#set_extended_security_options()

add_compile_options(-D${TARGETFLAG})

if(NOT DEPEND_INSTALL_DIR STREQUAL "")
    include_directories(${DEPEND_INSTALL_DIR}/include)
    link_directories(${DEPEND_INSTALL_DIR}/lib)
    if(EXISTS ${DEPEND_INSTALL_DIR}/lib64)
        link_directories(${DEPEND_INSTALL_DIR}/lib64)
    endif()
endif()

############################
# find_xxxx() search paths #
############################

if(CMAKE_CROSSCOMPILING)
    # CMAKE_FIND_ROOT_PATH specifies directories of TARGET platform
    # files to be searched by the various find_xxxx() commands. It is
    # not normally searched by find_program(), since the executables
    # we're interested are primarily HOST programs.

    if(NOT DEPEND_INSTALL_DIR STREQUAL "")
        list(APPEND CMAKE_FIND_ROOT_PATH ${DEPEND_INSTALL_DIR})
    endif()

    list(APPEND CMAKE_FIND_ROOT_PATH ${SDE_INSTALL_DIR})

    if(LEGACY_P4OVS AND NOT OVS_INSTALL_DIR STREQUAL "")
        list(APPEND CMAKE_FIND_ROOT_PATH ${OVS_INSTALL_DIR})
    endif()
else()
    # CMAKE_PREFIX_PATH specifies directories of HOST platform files
    # to be searched by the various find_xxxx() commands.

    if(NOT DEPEND_INSTALL_DIR STREQUAL "")
        list(APPEND CMAKE_PREFIX_PATH ${DEPEND_INSTALL_DIR})
    endif()

    list(APPEND CMAKE_PREFIX_PATH ${SDE_INSTALL_DIR})

    if(LEGACY_P4OVS AND NOT OVS_INSTALL_DIR STREQUAL "")
        list(APPEND CMAKE_PREFIX_PATH ${OVS_INSTALL_DIR})
    endif()
endif()

if(NOT HOST_DEPEND_DIR STREQUAL "")
    list(APPEND CMAKE_PREFIX_PATH ${HOST_DEPEND_DIR})
endif()

list(REMOVE_DUPLICATES CMAKE_FIND_ROOT_PATH)
list(REMOVE_DUPLICATES CMAKE_PREFIX_PATH)

cmake_print_variables(CMAKE_FIND_ROOT_PATH)
cmake_print_variables(CMAKE_PREFIX_PATH)

#####################
# External packages #
#####################

# Stratum dependencies for the target system.
include(StratumDependencies)

# Protobuf compiler for the development system.
find_package(HostProtoc)

# TLS libraries.
find_package(OpenSSL REQUIRED)
mark_as_advanced(OpenSSL_DIR)

# P4 SDE for the target system.
if(DPDK_TARGET)
    find_package(DpdkDriver)
elseif(ES2K_TARGET)
    find_package(Es2kDriver)
elseif(TOFINO_TARGET)
    find_package(TofinoDriver)
endif()

# Open vSwitch.
if(LEGACY_P4OVS)
    find_package(OVS)
endif()

#####################
# set_install_rpath #
#####################

include(SetInstallRpath)

#####################
# Protobufs project #
#####################

include(P4RuntimeProtobufs)

##################
# Subdirectories #
##################

if(WITH_KRNLMON)
    add_subdirectory(krnlmon)
endif()

if(WITH_OVSP4RT)
    add_subdirectory(ovs-p4rt)
endif()

add_subdirectory(stratum)
add_subdirectory(infrap4d)
add_subdirectory(clients)
add_subdirectory(scripts)
