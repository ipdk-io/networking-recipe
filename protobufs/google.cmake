#
# Copyright 2023 Intel Corporation
# SPDX-License-Identifier: Apache 2.0
#
# Compiles Google RPC protobuf files.
#

set(google_proto_sources
  ${GOOGLE_SOURCE_DIR}/google/rpc/status.proto
  ${GOOGLE_SOURCE_DIR}/google/rpc/code.proto
)

set(google_proto_cpp_sources
  ${CPP_OUT}/google/rpc/status.pb.cc
  ${CPP_OUT}/google/rpc/code.pb.cc
)

set(google_proto_cpp_headers
  ${CPP_OUT}/google/rpc/status.pb.h
  ${CPP_OUT}/google/rpc/code.pb.h
)

# Google protobufs for C++
add_custom_target(google_cpp_out ALL
  COMMAND
    ${HOST_PROTOC_COMMAND}
    ${google_proto_sources}
    --cpp_out ${CPP_OUT}
    -I${GOOGLE_SOURCE_DIR}
  DEPENDS
    ${google_proto_sources}
  BYPRODUCTS
    ${google_proto_cpp_sources}
    ${google_proto_cpp_headers}
  WORKING_DIRECTORY
    ${CMAKE_CURRENT_SOURCE_DIR}
  VERBATIM
)

# Google protobufs for Python
add_custom_target(google_py_out ALL
  COMMAND
    ${HOST_PROTOC_COMMAND}
    ${google_proto_sources}
    --python_out ${PY_OUT}
    --grpc_out ${PY_OUT}
    --plugin=protoc-gen-grpc=${HOST_GRPC_PY_PLUGIN}
    -I${GOOGLE_SOURCE_DIR}
  COMMAND
    find ${PY_OUT}/google -type d | xargs -I FNAME touch FNAME/__init__.py
  WORKING_DIRECTORY
    ${CMAKE_CURRENT_SOURCE_DIR}
  VERBATIM
)

# Google protobufs for Go
if(HOST_PROTOC_GO_PLUGIN)
  add_custom_target(google_go_out ALL
    COMMAND
      ${HOST_PROTOC_COMMAND}
      ${google_proto_sources}
      --go_out=${GO_OUT}
      --go-grpc_out=${GO_OUT}
      -I${GOOGLE_SOURCE_DIR}
    WORKING_DIRECTORY
      ${CMAKE_CURRENT_SOURCE_DIR}
    VERBATIM
  )
else(HOST_PROTOC_GO_PLUGIN)
  add_custom_target(google_go_out ALL
    COMMAND ""
    COMMENT "google_go_out is disabled"
  )
endif(HOST_PROTOC_GO_PLUGIN)
