#
# Copyright 2023 Intel Corporation
# SPDX-License-Identifier: Apache 2.0
#
# Compile p4runtime protobufs for supported languages.
#

# Version 3.15 is the baseline for P4 Control Plane.
cmake_minimum_required(VERSION 3.15)

project(protobufs LANGUAGES CXX)

include(CMakePrintHelpers)
include(GNUInstallDirs)

if(CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
  set(DEPEND_INSTALL_DIR "$ENV{DEPEND_INSTALL}" CACHE PATH
      "Dependencies install directory")

  if(DEPEND_INSTALL_DIR STREQUAL "")
    message(FATAL_ERROR "DEPEND_INSTALL_DIR (DEPEND_INSTALL) not defined!")
  endif()

  list(APPEND CMAKE_PREFIX_PATH ${DEPEND_INSTALL_DIR})

  include(../cmake/StratumDependencies.cmake)
endif()

set(GO_ENABLED TRUE CACHE BOOL "Compile protobufs for Go")
set(GRPC_OUT_ENABLED FALSE CACHE BOOL "Generate grpc_out directory")
set(WHEEL_ENABLED FALSE CACHE BOOL "Generate Python wheel")

cmake_print_variables(GO_ENABLED)
cmake_print_variables(GRPC_OUT_ENABLED)
cmake_print_variables(WHEEL_ENABLED)

set(CPP_OUT ${CMAKE_BINARY_DIR}/cpp_out CACHE PATH "C++ protobuf files")
file(MAKE_DIRECTORY ${CPP_OUT})

if(GRPC_OUT_ENABLED)
  set(GRPC_OUT ${CMAKE_BINARY_DIR}/grpc_out CACHE PATH "C++ gRPC files")
  file(MAKE_DIRECTORY ${GRPC_OUT})
else()
  set(GRPC_OUT ${CPP_OUT})
endif()

set(PY_OUT ${CMAKE_BINARY_DIR}/py_out CACHE PATH "Python protobuf files")
file(MAKE_DIRECTORY ${PY_OUT})

if(GO_ENABLED)
set(GO_OUT ${CMAKE_BINARY_DIR}/go_out CACHE PATH "Go protobuf files")
file(MAKE_DIRECTORY ${GO_OUT})
else(GO_ENABLED)
set(_sink ${GO_OUT}) # suppress cmake warning
endif(GO_ENABLED)

set(GOOGLE_SOURCE_DIR "../stratum/googleapis")
set(P4RT_PROTO_DIR "../stratum/p4runtime/proto")

include(google.cmake)
include(p4runtime.cmake)
include(tarballs.cmake)
