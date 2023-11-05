#
# Copyright 2023 Intel Corporation
# SPDX-License-Identifier: Apache 2.0
#
# Compile external protobufs for Python.
#

set(PY_OUT ${CMAKE_BINARY_DIR}/py_out)

file(MAKE_DIRECTORY ${PY_OUT})

#-----------------------------------------------------------------------
# Figure out what files we will be generating so we can declare them
# as byproducts of the build.
#-----------------------------------------------------------------------
# p4runtime
list(TRANSFORM P4RT_PROTO_FILES
  PREPEND "${PY_OUT}/"
  OUTPUT_VARIABLE _p4rt_list
)
list(TRANSFORM _p4rt_list
  REPLACE "\.proto$" "_pb2.py"
  OUTPUT_VARIABLE _p4rt_sources
)
list(TRANSFORM _google_list
  REPLACE "\.proto$" "_pb2.py"
  OUTPUT_VARIABLE _google_sources
)

# googleapis
list(TRANSFORM GOOGLE_PROTO_FILES
  PREPEND "${PY_OUT}/"
  OUTPUT_VARIABLE _google_list
)
list(TRANSFORM _p4rt_list
  REPLACE "\.proto$" "_pb2_grpc.py"
  OUTPUT_VARIABLE _p4rt_grpc_sources
)
list(TRANSFORM _google_list
  REPLACE "\.proto$" "_pb2_grpc.py"
  OUTPUT_VARIABLE _google_grpc_sources
)

#-----------------------------------------------------------------------
# Generate Python files
#-----------------------------------------------------------------------
add_custom_target(py_out ALL
    ${HOST_PROTOC_COMMAND}
    ${P4RT_PROTOS}
    ${GOOGLE_PROTOS}
    --python_out ${PY_OUT}
    ${PROTOFLAGS}
    --grpc_out ${PY_OUT}
    --plugin=protoc-gen-grpc=${HOST_GRPC_PY_PLUGIN}
  BYPRODUCTS
    ${_p4rt_sources}
    ${_google_sources}
    ${_p4rt_grpc_sources}
    ${_google_grpc_sources}
  WORKING_DIRECTORY
    ${CMAKE_CURRENT_SOURCE_DIR}
  COMMENT
    "Generating py_out"
  VERBATIM
)
