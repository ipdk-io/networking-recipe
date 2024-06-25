# CMake build file for Stratum code.
#
# Copyright 2022-2024 Intel Corporation
# SPDX-License-Identifier: Apache 2.0
#

# Version 3.15 is the baseline for P4 Control Plane.
cmake_minimum_required(VERSION 3.15)

#############################
# Symbolic path definitions #
#############################

set(GOOGLEAPIS_SOURCE_DIR
    "${CMAKE_CURRENT_SOURCE_DIR}/googleapis" CACHE PATH
    "Path to Google APIs source directory")
mark_as_advanced(GOOGLEAPIS_SOURCE_DIR)

set(P4RUNTIME_SOURCE_DIR
    "${CMAKE_CURRENT_SOURCE_DIR}/p4runtime" CACHE PATH
    "Path to P4Runtime source directory")
mark_as_advanced(P4RUNTIME_SOURCE_DIR)


set(STRATUM_INCLUDES
    ${STRATUM_SOURCE_DIR}
    # Protobuf C++ header files.
    ${PB_OUT_DIR}
)

set(PB_HEADER_INSTALL_DIR ${CMAKE_INSTALL_INCLUDEDIR}/stratum/pb)
set(DOT_PROTO_INSTALL_DIR ${CMAKE_INSTALL_DATAROOTDIR}/stratum/proto)

add_subdirectory(proto)
add_subdirectory(stratum)
