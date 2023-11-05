# Builds external protobufs.
#
# Copyright 2022-2023 Intel Corporation
# SPDX-License-Identifier: Apache 2.0
#

# Version 3.15 is the baseline for P4 Control Plane.
cmake_minimum_required(VERSION 3.15)

include(CMakePrintHelpers)
include(GNUInstallDirs)

project(protobufs LANGUAGES CXX)

if(CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
  set(DEPEND_INSTALL_DIR "$ENV{DEPEND_INSTALL}" CACHE PATH
      "Dependencies install directory")

  if(DEPEND_INSTALL_DIR STREQUAL "")
    message(FATAL_ERROR "DEPEND_INSTALL_DIR (DEPEND_INSTALL) not defined!")
  endif()

  list(APPEND CMAKE_PREFIX_PATH ${DEPEND_INSTALL_DIR})

  include(../cmake/StratumDependencies.cmake)
endif()

option(WITH_GRPC_OUT False "Use separate grpc_out directory")

set(CPP_OUT ${CMAKE_BINARY_DIR}/cpp_out CACHE PATH "C++ protobuf files")
file(MAKE_DIRECTORY ${CPP_OUT})

if(WITH_GRPC_OUT)
  set(GRPC_OUT ${CMAKE_BINARY_DIR}/grpc_out CACHE PATH "C++ gRPC files")
  file(MAKE_DIRECTORY ${GRPC_OUT})
else()
  set(GRPC_OUT ${CPP_OUT})
endif()

set(PY_OUT ${CMAKE_BINARY_DIR}/py_out CACHE PATH "Python protobuf files")
file(MAKE_DIRECTORY ${PY_OUT})

set(GO_OUT ${CMAKE_BINARY_DIR}/go_out CACHE PATH "Go protobuf files")
file(MAKE_DIRECTORY ${GO_OUT})

set(GOOGLE_SOURCE_DIR "../stratum/googleapis")
set(P4RT_PROTO_DIR "../stratum/p4runtime/proto")

include(google.cmake)
include(p4runtime.cmake)

set(_tar_flags -z --owner=ipdk --group=ipdk --sort=name)

add_custom_target(protobuf-tarballs ALL
  COMMAND
    tar -cf cpp_out_protos.tar.gz ${_tar_flags} cpp_out
  COMMAND
    tar -cf py_out_protos.tar.gz ${_tar_flags} py_out
  COMMAND
    tar -cf go_out_protos.tar.gz ${_tar_flags} go_out
  DEPENDS
    google_cpp_out
    google_py_out
    google_go_out
    p4rt_cpp_out
    p4rt_grpc_out
    p4rt_py_out
    p4rt_go_out
  BYPRODUCTS
    cpp_out_protos.tar.gz
    py_out_protos.tar.gz
    go_out_protos.tar.gz
  WORKING_DIRECTORY
    ${CMAKE_CURRENT_BINARY_DIR}
  COMMENT
    "Generating protobuf tarballs"
  VERBATIM
)

install(
  FILES
    ${CMAKE_CURRENT_BINARY_DIR}/cpp_out_protos.tar.gz
    ${CMAKE_CURRENT_BINARY_DIR}/py_out_protos.tar.gz
    ${CMAKE_CURRENT_BINARY_DIR}/go_out_protos.tar.gz
  DESTINATION
    ${CMAKE_INSTALL_DATAROOTDIR}/stratum/proto
)
