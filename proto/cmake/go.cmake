#
# Copyright 2023 Intel Corporation
# SPDX-License-Identifier: Apache 2.0
#
# Compile external protobufs for Go.
#

set(GO_OUT ${CMAKE_BINARY_DIR}/go_out)

file(MAKE_DIRECTORY ${GO_OUT})

# Directory prefix for p4runtime files.
set(P4RT_GO_OUT_PREFIX ${GO_OUT}/github.com/p4lang/p4runtime/go)

# Directory prefix for generated google rpc files.
set(GOOGLE_GO_OUT_PREFIX ${GO_OUT}/google.golang.org/genproto/googleapis)

#-----------------------------------------------------------------------
# Figure out what files we will be generating so we can declare them
# as byproducts of the build.
#-----------------------------------------------------------------------
# p4runtime
list(TRANSFORM P4RT_PROTO_FILES
  PREPEND "${P4RT_GO_OUT_PREFIX}/"
  OUTPUT_VARIABLE _p4rt_list
)
list(TRANSFORM _p4rt_list
  REPLACE "\.proto$" ".pb.go"
  OUTPUT_VARIABLE _p4rt_sources
)

# google rpc
list(TRANSFORM GOOGLE_PROTO_FILES
  PREPEND "${GOOGLE_GO_OUT_PREFIX}/"
  OUTPUT_VARIABLE _google_list
)
list(TRANSFORM _google_list
  REPLACE "googleapis/google/" "googleapis/"
  OUTPUT_VARIABLE _google_sources
)
list(TRANSFORM _google_sources
  REPLACE "\.proto$" ".pb.go"
  OUTPUT_VARIABLE _google_sources
)

#-----------------------------------------------------------------------
# Generate Go files.
#-----------------------------------------------------------------------
add_custom_target(go_out ALL
    ${HOST_PROTOC}
    ${P4RT_PROTOS}
    ${GOOGLE_PROTOS}
    --go_out=${GO_OUT}
    --go-grpc_out=${GO_OUT}
    ${PROTOFLAGS}
  BYPRODUCTS
    ${_p4rt_sources}
    ${_google_sources}
    ${P4RT_GO_OUT_PREFIX}/p4/v1/p4runtime_grpc.pb.go
  WORKING_DIRECTORY
    ${CMAKE_CURRENT_SOURCE_DIR}
  COMMENT
    "Generating go_out"
  VERBATIM
)
