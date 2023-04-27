# CMake build file for superproject.
#
# Copyright 2022 Intel Corporation
# SPDX-License-Identifier: Apache 2.0
#

cmake_minimum_required(VERSION 3.5)

option(KLOCWORK "Configure for Klocwork scan" OFF)

# CMAKE_BUILD_TYPE must be set before the first project() command.
if(KLOCWORK)
  set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Type of build to perform")
else()
  set(CMAKE_BUILD_TYPE "RelWithDebInfo" CACHE STRING "Type of build to perform")
endif()

project(networking-recipe VERSION 23.01 LANGUAGES C CXX)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

include(CMakePrintHelpers)
include(CTest)
include(FindPkgConfig)
include(GNUInstallDirs)

cmake_print_variables(CMAKE_SYSROOT)
cmake_print_variables(CMAKE_TOOLCHAIN_FILE)
cmake_print_variables(CMAKE_STAGING_PREFIX)

#############################
# Symbolic path definitions #
#############################

set(DEPEND_INSTALL_DIR "$ENV{DEPEND_INSTALL}" CACHE PATH
    "Dependencies install directory")

set(OVS_INSTALL_DIR "${CMAKE_INSTALL_PREFIX}" CACHE PATH
    "OVS install directory")

set(OVS_SOURCE_DIR "${CMAKE_SOURCE_DIR}/ovs/ovs" CACHE PATH
    "OVS source directory")

set(SDE_INSTALL_DIR "$ENV{SDE_INSTALL}" CACHE PATH
    "SDE install directory")

set(HOST_DEPEND_DIR "" CACHE PATH "Host dependencies install directory")

set(PB_OUT_DIR "${CMAKE_BINARY_DIR}/pb.out" CACHE PATH
    "Path to generated Protobuf files")
file(MAKE_DIRECTORY ${PB_OUT_DIR})

############################
# Target selection options #
############################

option(DPDK_TARGET      "Build for DPDK target" OFF)
option(TOFINO_TARGET    "Build for Tofino target" OFF)
option(ES2K_TARGET      "Build for ES2000 target" OFF)

# Translate target option to TARGETFLAG string.
# - Use precedence to deal with multiple selections.
# - Ensure that individual target options are consistent
#   so we can use them to control the build.
# - Default to DPDK_TARGET.
set(_target_action "Building")
set(_default_target "DPDK_TARGET")
if(TOFINO_TARGET)
    set(TARGETFLAG TOFINO_TARGET)
    set(DPDK_TARGET OFF)
    set(ES2K_TARGET OFF)
elseif(DPDK_TARGET)
    set(TARGETFLAG DPDK_TARGET)
    set(ES2K_TARGET OFF)
    set(TOFINO_TARGET OFF)
elseif(ES2K_TARGET)
    set(TARGETFLAG ES2K_TARGET)
    set(DPDK_TARGET OFF)
    set(TOFINO_TARGET OFF)
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

option(SET_RPATH    "Set RPATH in libraries and executables" OFF)

option(WITH_KRNLMON "Enable Kernel Monitor support" ON)
option(WITH_OVSP4RT "Enable OVS support" ON)

if(TOFINO_TARGET)
    set(WITH_KRNLMON OFF)
    set(WITH_OVSP4RT OFF)
endif()

cmake_print_variables(WITH_KRNLMON)
cmake_print_variables(WITH_OVSP4RT)

###########################
# Global compiler options #
###########################

add_compile_options(-D${TARGETFLAG})

if(NOT DEPEND_INSTALL_DIR STREQUAL "")
    include_directories(${DEPEND_INSTALL_DIR}/include)
    link_directories(${DEPEND_INSTALL_DIR}/lib)
    if(EXISTS ${DEPEND_INSTALL_DIR}/lib64)
        link_directories(${DEPEND_INSTALL_DIR}/lib64)
    endif()
endif()

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

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

    if(NOT OVS_INSTALL_DIR STREQUAL "")
        list(APPEND CMAKE_FIND_ROOT_PATH ${OVS_INSTALL_DIR})
    endif()
else()
    # CMAKE_PREFIX_PATH specifies directories of HOST platform files
    # to be searched by the various find_xxxx() commands.

    if(NOT DEPEND_INSTALL_DIR STREQUAL "")
        list(APPEND CMAKE_PREFIX_PATH ${DEPEND_INSTALL_DIR})
    endif()

    list(APPEND CMAKE_PREFIX_PATH ${SDE_INSTALL_DIR})

    if(NOT OVS_INSTALL_DIR STREQUAL "")
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

find_package(absl REQUIRED)

if(DPDK_TARGET)
    include(dpdk-driver)
elseif(ES2K_TARGET)
    include(es2k-driver)
elseif(TOFINO_TARGET)
    include(tofino-driver)
endif()

########################
# strip_sysroot_prefix #
########################

function(strip_sysroot_prefix _IN _OUT)
    string(REGEX REPLACE "^${CMAKE_SYSROOT}(.+)\$" "\\1" _out "${_IN}")
    set(${_OUT} ${_out} PARENT_SCOPE)
endfunction()

##################
# RPATH elements #
##################

# TODO:
# If the SDE or DEPEND install tree is combined with the
# RECIPE install tree (which we might want to do for a runtime
# system), we can omit the SDE or DEPEND element in favor of
# the ORIGIN element.
#
# The ORIGIN element is equivalent to ${CMAKE_PREFIX_PATH}/lib or
# ${CMAKE_PREFIX_PATH}/lib64.
#
# The elements will need to be expanded to include /lib64 paths where
# appropriate.
#
# We shouldn't need an element for ${SDE_INSTALL_DIR}/lib/x86_64-linux-gnu.
# It's only used for DPDK libraries, which are transitive dependencies of
# the SDE libraries (specifically dpdk_infra.so). In the unlikely event
# that we do need to include the DPDK library directory in the RPATH, we
# can use DPDK_LIBRARY_DIRS, which is defined by pkg_check_modules(DPDK).

# SDE_ELEMENT
strip_sysroot_prefix(${SDE_INSTALL_DIR} _sde_stem)
if(EXISTS ${SDE_INSTALL_DIR}/lib)
    list(APPEND SDE_ELEMENT ${_sde_stem}/lib)
endif()
if(EXISTS ${SDE_INSTALL_DIR}/lib64)
    list(APPEND SDE_ELEMENT ${_sde_stem}/lib64)
endif()
unset(_sde_stem)

# DEP_ELEMENT
strip_sysroot_prefix(${DEPEND_INSTALL_DIR} _dep_stem)
if(NOT DEPEND_INSTALL_DIR STREQUAL "")
    if(EXISTS ${DEPEND_INSTALL_DIR}/lib)
        list(APPEND DEP_ELEMENT ${_dep_stem}/lib)
    endif()
    if(EXISTS ${DEPEND_INSTALL_DIR}/lib64)
        list(APPEND DEP_ELEMENT ${_dep_stem}/lib64)
    endif()
endif()
unset(_dep_stem)

# EXEC_ELEMENT
if(EXISTS ${CMAKE_INSTALL_PREFIX}/lib OR CMAKE_INSTALL_LIBDIR STREQUAL "lib")
    list(APPEND EXEC_ELEMENT $ORIGIN/../lib)
endif()
if(EXISTS ${CMAKE_INSTALL_PREFIX}/lib64 OR CMAKE_INSTALL_LIBDIR STREQUAL "lib64")
    list(APPEND EXEC_ELEMENT $ORIGIN/../lib64)
endif()

###################
# RPATH functions #
###################

# Constructs an RPATH (RUNPATH) value from a list of component paths.
function(define_rpath VAR)
    set(_rpath ${ARGN})
    list(REMOVE_DUPLICATES _rpath)
    list(JOIN _rpath ":" _rpath)
    set(${VAR} ${_rpath} PARENT_SCOPE)
endfunction()

# Constructs an RPATH (RUNPATH) value and sets the INSTALL_RPATH property
# of the specified target.
function(set_install_rpath TARGET)
    if(SET_RPATH)
        define_rpath(_rpath ${ARGN})
        set_target_properties(${TARGET} PROPERTIES INSTALL_RPATH ${_rpath})
        # message(STATUS "${TARGET}::RPATH=\"${_rpath}\"")
    endif()
endfunction()

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
