# Import host Protobuf compiler.
#
# Copyright 2022-2023 Intel Corporation
# SPDX-License-Identifier: Apache 2.0
#

include_guard(GLOBAL)

#-----------------------------------------------------------------------
# Protobuf compiler.
# Runs on the development system.
#-----------------------------------------------------------------------
find_program(HOST_PROTOC_COMMAND "protoc" NO_CMAKE_FIND_ROOT_PATH)
mark_as_advanced(HOST_PROTOC_COMMAND)

if(HOST_PROTOC_COMMAND)
  execute_process(
    COMMAND ${HOST_PROTOC_COMMAND} --version
    OUTPUT_VARIABLE protoc_output
  )
  string(STRIP "${protoc_output}" protoc_output)
  string(REGEX REPLACE
    "^libprotoc +([0-9.]+)$" "\\1" PROTOC_VERSION ${protoc_output})
  message(STATUS "Found protoc: ${HOST_PROTOC_COMMAND} (version found \"${PROTOC_VERSION}\")")
  unset(protoc_output)
else()
  message(FATAL_ERROR "protoc not found")
endif()

#-----------------------------------------------------------------------
# gRPC c++ plugin for the Protobuf compiler.
# Runs on the development (host) system.
#-----------------------------------------------------------------------
find_program(HOST_GRPC_CPP_PLUGIN "grpc_cpp_plugin" NO_CMAKE_FIND_ROOT_PATH)
if(HOST_GRPC_CPP_PLUGIN)
  message(STATUS "Found grpc_cpp_plugin")
else()
  message(FATAL_ERROR "grpc_cpp_plugin not found")
endif()
mark_as_advanced(HOST_GRPC_CPP_PLUGIN)

#-----------------------------------------------------------------------
# gRPC Python plugin for the Protobuf compiler.
# Runs on the development (host) system.
#-----------------------------------------------------------------------
find_program(HOST_GRPC_PY_PLUGIN "grpc_python_plugin" NO_CMAKE_FIND_ROOT_PATH)
if(HOST_GRPC_PY_PLUGIN)
  message(STATUS "Found grpc_python_plugin")
else()
  message(WARNING "grpc_python_plugin not found")
endif()
mark_as_advanced(HOST_GRPC_PY_PLUGIN)

#-----------------------------------------------------------------------
# Go plugin for the Protobuf compiler.
# Runs on the development (host) system.
#-----------------------------------------------------------------------
find_program(HOST_PROTOC_GO_PLUGIN "protoc-gen-go" NO_CMAKE_FIND_ROOT_PATH)
mark_as_advanced(HOST_PROTOC_GO_PLUGIN)

if(HOST_PROTOC_GO_PLUGIN)
  execute_process(
    COMMAND ${HOST_PROTOC_GO_PLUGIN} --version
    OUTPUT_VARIABLE plugin_output
  )
  string(STRIP "${plugin_output}" plugin_output)
  message(STATUS "Found ${plugin_output}")
  unset(plugin_output)
else()
  message(STATUS "protoc-gen-go not found")
endif()

#-----------------------------------------------------------------------
# Go gRPC plugin for the Protobuf compiler.
# Runs on the development (host) system.
#-----------------------------------------------------------------------
find_program(HOST_GRPC_GO_PLUGIN "protoc-gen-go-grpc" NO_CMAKE_FIND_ROOT_PATH)
mark_as_advanced(HOST_GRPC_GO_PLUGIN)

if(HOST_GRPC_GO_PLUGIN)
  execute_process(
    COMMAND ${HOST_GRPC_GO_PLUGIN} --version
    OUTPUT_VARIABLE plugin_output
  )
  string(STRIP "${plugin_output}" plugin_output)
  message(STATUS "Found ${plugin_output}")
  unset(plugin_output)
else()
  message(STATUS "protoc-gen-go-grpc plugin not found")
endif()
