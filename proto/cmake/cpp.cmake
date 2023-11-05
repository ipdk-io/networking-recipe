#
# Copyright 2023 Intel Corporation
# SPDX-License-Identifier: Apache 2.0
#
# Compile external protobufs for C++.
#

option(WITH_GRPC_OUT FALSE "Generate C++ gRPC files in GRPC_OUT")

set(CPP_OUT ${CMAKE_BINARY_DIR}/cpp_out)
file(MAKE_DIRECTORY ${CPP_OUT})

if(WITH_GRPC_OUT)
  set(GRPC_OUT ${CMAKE_BINARY_DIR}/grpc_out)
  file(MAKE_DIRECTORY ${GRPC_OUT})
else()
  set(GRPC_OUT ${CPP_OUT})
endif()

#-----------------------------------------------------------------------
# Figure out what files we will be generating so we can declare the
# byproducts of the build.
#-----------------------------------------------------------------------
# p4runtime
list(TRANSFORM P4RT_PROTO_FILES
  PREPEND "${CPP_OUT}/"
  OUTPUT_VARIABLE _p4rt_list
)
list(TRANSFORM _p4rt_list
  REPLACE "\.proto$" ".h"
  OUTPUT_VARIABLE _p4rt_headers
)
list(TRANSFORM _p4rt_list
  REPLACE "\.proto$" ".cc"
  OUTPUT_VARIABLE _p4rt_sources
)

# googleapis
list(TRANSFORM GOOGLE_PROTO_FILES
  PREPEND "${CPP_OUT}/"
  OUTPUT_VARIABLE _google_list
)
list(TRANSFORM _google_list
  REPLACE "\.proto$" ".h"
  OUTPUT_VARIABLE _google_headers
)
list(TRANSFORM _google_list
  REPLACE "\.proto$" ".cc"
  OUTPUT_VARIABLE _google_sources
)

#-----------------------------------------------------------------------
# Compile protobufs for C++
#-----------------------------------------------------------------------
add_custom_target(cpp_out ALL
    ${HOST_PROTOC_COMMAND}
    ${P4RT_PROTOS}
    ${GOOGLE_PROTOS}
    --cpp_out ${CPP_OUT}
    ${PROTOFLAGS}
  BYPRODUCTS
    ${_p4rt_headers}
    ${_p4rt_sources}
    ${_google_headers}
    ${_google_sources}
  WORKING_DIRECTORY
    ${CMAKE_CURRENT_SOURCE_DIR}
  COMMENT
    "Generating cpp_out"
  VERBATIM
)

#-----------------------------------------------------------------------
# Compile gRPC services for C++
#-----------------------------------------------------------------------
list(TRANSFORM GRPC_PROTO_FILES
  PREPEND "${GRPC_OUT}/"
  OUTPUT_VARIABLE _grpc_list
)
list(TRANSFORM _grpc_list
  REPLACE "\.proto$" ".grpc.h"
  OUTPUT_VARIABLE _grpc_headers
)
list(TRANSFORM _grpc_list
  REPLACE "\.proto$" ".grpc.cc"
  OUTPUT_VARIABLE _grpc_sources
)

add_custom_target(grpc_out ALL
    ${HOST_PROTOC_COMMAND}
    ${GRPC_PROTOS}
    --grpc_out ${GRPC_OUT}
    --plugin=protoc-gen-grpc=${HOST_GRPC_CPP_PLUGIN}
    ${PROTOFLAGS}
  BYPRODUCTS
    ${_grpc_headers}
    ${_grpc_sources}
  WORKING_DIRECTORY
    ${CMAKE_CURRENT_SOURCE_DIR}
  COMMENT
    "Generating grpc_out"
  VERBATIM
)
