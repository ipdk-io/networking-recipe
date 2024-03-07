#
# Copyright 2023 Intel Corporation
# SPDX-License-Identifier: Apache 2.0
#
# Compiles P4Runtime protobuf files.
#

set(p4rt_proto_sources
  ${P4RT_PROTO_DIR}/idpf/p4info.proto
  ${P4RT_PROTO_DIR}/idpf/p4runtime.proto
  ${P4RT_PROTO_DIR}/p4/v1/p4data.proto
  ${P4RT_PROTO_DIR}/p4/v1/p4runtime.proto
  ${P4RT_PROTO_DIR}/p4/config/v1/p4info.proto
  ${P4RT_PROTO_DIR}/p4/config/v1/p4types.proto
)

set(p4rt_proto_cpp_sources
  ${CPP_OUT}/idpf/p4info.pb.cc
  ${CPP_OUT}/idpf/p4runtime.pb.cc
  ${CPP_OUT}/p4/v1/p4data.pb.cc
  ${CPP_OUT}/p4/v1/p4runtime.pb.cc
  ${CPP_OUT}/p4/config/v1/p4info.pb.cc
  ${CPP_OUT}/p4/config/v1/p4types.pb.cc
)

set(p4rt_proto_headers
  ${CPP_OUT}/idpf/p4info.pb.h
  ${CPP_OUT}/idpf/p4runtime.pb.h
  ${CPP_OUT}/p4/v1/p4data.pb.h
  ${CPP_OUT}/p4/v1/p4runtime.pb.h
  ${CPP_OUT}/p4/config/v1/p4info.pb.h
  ${CPP_OUT}/p4/config/v1/p4types.pb.h
)
set(grpc_proto_sources
  p4/v1/p4runtime.proto
)

set(grpc_proto_outputs
  ${CPP_OUT}/p4/v1/p4runtime.pb.grpc.cc
  ${CPP_OUT}/p4/v1/p4runtime.pb.grpc.h
)

set(PROTOFLAGS -I${GOOGLE_SOURCE_DIR} -I${P4RT_PROTO_DIR})

# P4Runtime protobufs for C++
add_custom_target(p4rt_cpp_out ALL
  COMMAND
    ${HOST_PROTOC_COMMAND}
    ${p4rt_proto_sources}
    --cpp_out ${CPP_OUT}
    ${PROTOFLAGS}
  DEPENDS
    google_cpp_out
  BYPRODUCTS
    ${p4rt_proto_cpp_sources}
    ${p4rt_proto_cpp_headers}
  WORKING_DIRECTORY
    ${CMAKE_CURRENT_SOURCE_DIR}
  VERBATIM
)

# P4Runtime gRPC for C++
add_custom_target(p4rt_grpc_out ALL
  COMMAND
    ${HOST_PROTOC_COMMAND}
    ${grpc_proto_sources}
    --grpc_out ${CPP_OUT}
    --plugin=protoc-gen-grpc=${HOST_GRPC_CPP_PLUGIN}
    ${PROTOFLAGS}
  BYPRODUCTS
    ${grpc_proto_outputs}
  WORKING_DIRECTORY
    ${CMAKE_CURRENT_SOURCE_DIR}
  VERBATIM
)

# P4Runtime protobufs for Python
add_custom_target(p4rt_py_out ALL
  COMMAND
    ${HOST_PROTOC_COMMAND}
    ${p4rt_proto_sources}
    --python_out ${PY_OUT}
    --grpc_out ${PY_OUT}
    --plugin=protoc-gen-grpc=${HOST_GRPC_PY_PLUGIN}
    ${PROTOFLAGS}
  COMMAND
    find ${PY_OUT}/p4 -type d -exec touch {}/__init__.py \;
  WORKING_DIRECTORY
    ${CMAKE_CURRENT_SOURCE_DIR}
  VERBATIM
)

# P4Runtime Google protobufs for Go
if(GEN_GO_PROTOBUFS)
  add_custom_target(p4rt_go_out ALL
    COMMAND
      ${HOST_PROTOC_COMMAND}
      ${p4rt_proto_sources}
      --go_out=${GO_OUT}
      --go-grpc_out=${GO_OUT}
      ${PROTOFLAGS}
    WORKING_DIRECTORY
      ${CMAKE_CURRENT_SOURCE_DIR}
    VERBATIM
  )
endif(GEN_GO_PROTOBUFS)
