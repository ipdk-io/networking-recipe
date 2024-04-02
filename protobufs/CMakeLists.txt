#
# Copyright 2023-2024 Intel Corporation
# SPDX-License-Identifier: Apache 2.0
#
# Compile p4runtime protobufs for supported languages.
#

# Version 3.15 is the baseline for P4 Control Plane.
cmake_minimum_required(VERSION 3.15)

project(protobufs VERSION 2024.3.0 LANGUAGES CXX)

include(CMakePrintHelpers)
include(GNUInstallDirs)

set(DEPEND_INSTALL_DIR "$ENV{DEPEND_INSTALL}" CACHE PATH
    "Dependencies install directory")

if(DEPEND_INSTALL_DIR STREQUAL "")
  message(FATAL_ERROR "DEPEND_INSTALL_DIR (DEPEND_INSTALL) not defined!")
endif()

list(APPEND CMAKE_PREFIX_PATH ${DEPEND_INSTALL_DIR})

# Add networking-recipe/cmake to the search path for the package file.
get_filename_component(_path "../cmake" REALPATH)
list(APPEND CMAKE_MODULE_PATH ${_path})
unset(_path)

# Find the host Protobuf compiler.
find_package(HostProtoc)

set(GEN_PY_WHEEL TRUE CACHE BOOL "Generate Python wheel")

# Generation of the Go protobuf files may be enabled/disabled explicitly
# through the GEN_GO_PROTOBUFS option. If the option is undefined,
# generation is contingent on whether the Go plugins are installed.
if(NOT DEFINED GEN_GO_PROTOBUFS)
  if(NOT HOST_PROTOC_GO_PLUGIN OR NOT HOST_GRPC_GO_PLUGIN)
    message(WARNING
      "Plugin missing - p4runtime protobufs for Go will not be generated.")
    set(GEN_GO_PROTOBUFS FALSE CACHE BOOL "Disable Go protobuf generation")
  else()
    set(GEN_GO_PROTOBUFS TRUE CACHE BOOL "Enable Go protobuf generation")
  endif()
elseif(NOT GEN_GO_PROTOBUFS)
  message(STATUS "Go protobuf generation disabled")
endif()

# Directory for generated C++ files.
set(CPP_OUT ${CMAKE_BINARY_DIR}/cpp_out CACHE PATH "C++ protobuf files")
file(MAKE_DIRECTORY ${CPP_OUT})

# Directory for generated Python files.
set(PY_OUT ${CMAKE_BINARY_DIR}/py_out CACHE PATH "Python protobuf files")
file(MAKE_DIRECTORY ${PY_OUT})

# Directory for generated Go files.
if(GEN_GO_PROTOBUFS)
  set(GO_OUT ${CMAKE_BINARY_DIR}/go_out CACHE PATH "Go protobuf files")
  file(MAKE_DIRECTORY ${GO_OUT})
endif()

# Where to find the .proto files.
set(GOOGLE_SOURCE_DIR "../stratum/googleapis")
set(P4RT_PROTO_DIR "../stratum/p4runtime/proto")

include(google.cmake)
include(p4runtime.cmake)
include(tarballs.cmake)
if(GEN_PY_WHEEL)
  include(wheel.cmake)
endif()
