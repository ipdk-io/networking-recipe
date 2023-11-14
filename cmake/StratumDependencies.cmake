# Import Stratum dependencies.
#
# Copyright 2022-2023 Intel Corporation
# SPDX-License-Identifier: Apache 2.0
#

include_guard(GLOBAL)

#-----------------------------------------------------------------------
# Google C++ libraries (Abseil).
#-----------------------------------------------------------------------
find_package(absl CONFIG REQUIRED)
mark_as_advanced(absl_DIR)

message(STATUS "Found Abseil version ${absl_VERSION}")

if(absl_VERSION VERSION_GREATER_EQUAL "20230125")
  add_compile_definitions(ABSL_LEGACY_THREAD_ANNOTATIONS)
endif()

#-----------------------------------------------------------------------
# Google command-line flags library (gflags).
#-----------------------------------------------------------------------
# By default, gflags does not namespace its targets.
set(GFLAGS_USE_TARGET_NAMESPACE TRUE)

find_package(gflags CONFIG REQUIRED)
mark_as_advanced(gflags_DIR)

#-----------------------------------------------------------------------
# Google logging library (glog).
#-----------------------------------------------------------------------
find_package(glog CONFIG REQUIRED)
mark_as_advanced(glog_DIR)

#-----------------------------------------------------------------------
# Google Protocol Buffers (protobuf).
# We must import the Protobuf package before gRPC.
#-----------------------------------------------------------------------
find_package(Protobuf CONFIG REQUIRED)
mark_as_advanced(Protobuf_DIR)

message(STATUS "Found Protobuf version ${Protobuf_VERSION}")

#-----------------------------------------------------------------------
# Google RPC (gRPC).
#-----------------------------------------------------------------------
find_package(gRPC CONFIG REQUIRED)
mark_as_advanced(gRPC_DIR)
mark_as_advanced(c-ares_DIR)

message(STATUS "Found gRPC version ${gRPC_VERSION}")
