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

#-----------------------------------------------------------------------
# Google RPC (gRPC).
#-----------------------------------------------------------------------
find_package(gRPC CONFIG REQUIRED)
mark_as_advanced(gRPC_DIR)
mark_as_advanced(c-ares_DIR)

message(STATUS "Found gRPC version ${gRPC_VERSION}")

#-----------------------------------------------------------------------
# Protobuf compiler.
# Runs on the development system.
#-----------------------------------------------------------------------
find_program(HOST_PROTOC_COMMAND "protoc" NO_CMAKE_FIND_ROOT_PATH)
mark_as_advanced(HOST_PROTOC_COMMAND)

if(HOST_PROTOC_COMMAND)
  message(STATUS "Found protoc: ${HOST_PROTOC_COMMAND}")
else()
  message(FATAL_ERROR "protoc not found")
endif()

#-----------------------------------------------------------------------
# gRPC c++ plugin for the Protobuf compiler.
# Runs on the development (host) system.
#-----------------------------------------------------------------------
find_program(HOST_GRPC_CPP_PLUGIN "grpc_cpp_plugin" NO_CMAKE_FIND_ROOT_PATH)
mark_as_advanced(HOST_GRPC_CPP_PLUGIN)

if(HOST_GRPC_CPP_PLUGIN)
  message(STATUS "Found grpc_cpp_plugin: ${HOST_GRPC_CPP_PLUGIN}")
else()
  message(FATAL_ERROR "grpc_cpp_plugin not found")
endif()

#-----------------------------------------------------------------------
# gRPC Python plugin for the Protobuf compiler.
# Runs on the development (host) system.
#-----------------------------------------------------------------------
find_program(HOST_GRPC_PY_PLUGIN "grpc_python_plugin" NO_CMAKE_FIND_ROOT_PATH)
mark_as_advanced(HOST_GRPC_PY_PLUGIN)

if(HOST_GRPC_PY_PLUGIN)
  message(STATUS "Found grpc_py_plugin: ${HOST_GRPC_PY_PLUGIN}")
endif()

#-----------------------------------------------------------------------
# Go plugin for the Protobuf compiler.
# Runs on the development (host) system.
#-----------------------------------------------------------------------
find_program(HOST_PROTOC_GO_PLUGIN "protoc-gen-go" NO_CMAKE_FIND_ROOT_PATH)
mark_as_advanced(HOST_PROTOC_GO_PLUGIN)

if(HOST_PROTOC_GO_PLUGIN)
  # TODO: protoc-gen-go --version to determine version
  # protoc-gen-go v1.28.1
  message(STATUS "Found protoc_go_plugin: ${HOST_PROTOC_GO_PLUGIN}")
endif()

#-----------------------------------------------------------------------
# Go gRPC plugin for the Protobuf compiler.
# Runs on the development (host) system.
#-----------------------------------------------------------------------
find_program(HOST_GRPC_GO_PLUGIN "protoc-gen-go-grpc" NO_CMAKE_FIND_ROOT_PATH)
mark_as_advanced(HOST_GRPC_GO_PLUGIN)

if(HOST_GRPC_GO_PLUGIN)
  # TODO: protoc-gen-go-grpc --version to determine version
  # protoc-gen-go-grpc 1.2.0
  message(STATUS "Found grpc_go_plugin: ${HOST_GRPC_GO_PLUGIN}")
endif()

#-----------------------------------------------------------------------
# SSL/TLS library (OpenSSL).
#-----------------------------------------------------------------------
find_package(OpenSSL REQUIRED)
mark_as_advanced(OpenSSL_DIR)

set(WITH_OPENSSL TRUE)
