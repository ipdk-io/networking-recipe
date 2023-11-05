# Builds external protobufs.
#
# Copyright 2022-2023 Intel Corporation
# SPDX-License-Identifier: Apache 2.0
#

# Version 3.15 is the baseline for P4 Control Plane.
cmake_minimum_required(VERSION 3.15)

include(CMakePrintHelpers)

project(external-protobufs LANGUAGES CXX)

# Support standalone builds during development.
if(CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
  set(DEPEND_INSTALL_DIR "$ENV{DEPEND_INSTALL}" CACHE PATH
      "Dependencies install directory")

  if(DEPEND_INSTALL_DIR STREQUAL "")
    message(FATAL_ERROR "DEPEND_INSTALL_DIR (DEPEND_INSTALL) not defined!")
  endif()

  list(APPEND CMAKE_PREFIX_PATH ${DEPEND_INSTALL_DIR})

  include(../cmake/StratumDependencies.cmake)
endif()

# TODO: Move these two submodules from stratum/ to proto/.
set(GOOGLE_PROTO_DIR "../stratum/googleapis")
set(P4RT_PROTO_DIR "../stratum/p4runtime/proto")

set(PROTOFLAGS -I${GOOGLE_PROTO_DIR} -I${P4RT_PROTO_DIR})

# p4runtime protobuf files
set(P4RT_PROTO_FILES
  p4/v1/p4data.proto
  p4/v1/p4runtime.proto
  p4/config/v1/p4info.proto
  p4/config/v1/p4types.proto
)

list(TRANSFORM P4RT_PROTO_FILES
  PREPEND "${P4RT_PROTO_DIR}/"
  OUTPUT_VARIABLE P4RT_PROTOS
)

# google rpc files
set(GOOGLE_PROTO_FILES
  google/rpc/status.proto
  google/rpc/code.proto
)

list(TRANSFORM GOOGLE_PROTO_FILES
  PREPEND "${GOOGLE_PROTO_DIR}/"
  OUTPUT_VARIABLE GOOGLE_PROTOS
)

# grpc files
set(GRPC_PROTO_FILES
  p4/v1/p4runtime.proto
)

list(TRANSFORM GRPC_PROTO_FILES
  PREPEND "${P4RT_PROTO_DIR}/"
  OUTPUT_VARIABLE GRPC_PROTOS
)

include(cmake/cpp.cmake)
include(cmake/python.cmake)
include(cmake/go.cmake)
