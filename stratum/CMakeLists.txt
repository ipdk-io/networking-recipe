# CMake build file for Stratum code.
#
# Copyright 2022 Intel Corporation
# SPDX-License-Identifier: Apache 2.0

cmake_minimum_required(VERSION 3.5)
project(stratum)

#############################
# Symbolic path definitions #
#############################

set(GOOGLE_SOURCE_DIR
    "${CMAKE_CURRENT_SOURCE_DIR}/googleapis" CACHE PATH
    "Path to Google APIs source directory")

set(P4RUNTIME_SOURCE_DIR
    "${CMAKE_CURRENT_SOURCE_DIR}/p4runtime" CACHE PATH
    "Path to P4Runtime source directory")

set(STRATUM_SOURCE_DIR
    "${CMAKE_CURRENT_SOURCE_DIR}/stratum" CACHE PATH
    "Path to Stratum source directory")

set(PROTO_OUT_DIR "${CMAKE_BINARY_DIR}/pb.out" CACHE PATH
    "Path to generated Protobuf files")
file(MAKE_DIRECTORY ${PROTO_OUT_DIR})

set(STRATUM_INCLUDES
    ${STRATUM_SOURCE_DIR}
    # Protobuf C++ header files.
    ${PROTO_OUT_DIR}
)

#####################
# Compile Protobufs #
#####################

option(INSTALL-PROTO "Install .proto files" OFF)

add_subdirectory(proto)

############################
# Resolve target libraries #
############################

# target_sys
find_library(LIBTARGET_SYS target_sys ${SDE_INSTALL_DIR}/lib)
if(NOT LIBTARGET_SYS)
    message(FATAL_ERROR "Cannot find library: target_sys")
endif()

add_library(target_sys SHARED IMPORTED)
set_property(TARGET target_sys PROPERTY IMPORTED_LOCATION ${LIBTARGET_SYS})

if(TOFINO_TARGET)
    # Tofino driver
    find_library(LIBDRIVER driver ${SDE_INSTALL_DIR}/lib)
    if(NOT LIBDRIVER)
        message(FATAL_ERROR "Cannot find library: driver")
    endif()

    add_library(driver SHARED IMPORTED)
    set_property(TARGET driver PROPERTY IMPORTED_LOCATION ${LIBDRIVER})
elseif(DPDK_TARGET)
    # tdi
    find_library(LIBTDI tdi ${SDE_INSTALL_DIR}/lib)
    if(NOT LIBTDI)
        message(FATAL_ERROR "Cannot find library: tdi")
    endif()

    add_library(tdi SHARED IMPORTED)
    set_property(TARGET tdi PROPERTY IMPORTED_LOCATION ${LIBTDI})

    # tdi_json_parser
    find_library(LIBTDI_JSON_PARSER tdi_json_parser ${SDE_INSTALL_DIR}/lib)
    if(NOT LIBTDI_JSON_PARSER)
        message(FATAL_ERROR "Cannot find library: tdi_json_parser")
    endif()

    add_library(tdi_json_parser SHARED IMPORTED)
    set_property(TARGET tdi_json_parser
                 PROPERTY IMPORTED_LOCATION ${LIBTDI_JSON_PARSER})

    # DPDK driver
    find_library(LIBDRIVER driver ${SDE_INSTALL_DIR}/lib)
    if(NOT LIBDRIVER)
        message(FATAL_ERROR "Cannot find library: driver")
    endif()

    add_library(driver SHARED IMPORTED)
    set_property(TARGET driver PROPERTY IMPORTED_LOCATION ${LIBDRIVER})

    # bf_switchd_lib
    find_library(LIBBF_SWITCHD bf_switchd_lib ${SDE_INSTALL_DIR}/lib)
    if(NOT LIBBF_SWITCHD)
        message(FATAL_ERROR "Cannot find library: bf_switchd_lib")
    endif()

    add_library(bf_switchd_lib SHARED IMPORTED)
    set_property(TARGET bf_switchd_lib
                 PROPERTY IMPORTED_LOCATION ${LIBBF_SWITCHD})

    # target_utils
    find_library(LIBTARGET_UTILS target_utils ${SDE_INSTALL_DIR}/lib)
    if(NOT LIBTARGET_UTILS)
        message(FATAL_ERROR "Cannot find library: target_utils")
    endif()

    add_library(target_utils SHARED IMPORTED)
    set_property(TARGET target_utils
                 PROPERTY IMPORTED_LOCATION ${LIBTARGET_UTILS})
endif()

###################
# Compile Stratum #
###################

add_subdirectory(stratum)
