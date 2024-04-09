#
# Copyright 2022-2023 Intel Corporation
# SPDX-License-Identifier: Apache 2.0
#
# Compile p4runtime protobufs for use by clients.
#

if(DEFINED GEN_GO_PROTOBUFS)
  # Don't forward option to external project if it's undefined.
  set(_gen_go_protobufs -DGEN_GO_PROTOBUFS=${GEN_GO_PROTOBUFS})
endif()

ExternalProject_Add(protobufs
  SOURCE_DIR
    ${CMAKE_CURRENT_SOURCE_DIR}/protobufs
  CMAKE_ARGS
    -DDEPEND_INSTALL_DIR=${DEPEND_INSTALL_DIR}
    -DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}
    -DCPP_OUT=${CMAKE_CURRENT_BINARY_DIR}/cpp_out
    -DGO_OUT=${CMAKE_CURRENT_BINARY_DIR}/go_out
    -DPY_OUT=${CMAKE_CURRENT_BINARY_DIR}/py_out
    ${_gen_go_protobufs}
  INSTALL_COMMAND
    ${CMAKE_MAKE_PROGRAM} install
  EXCLUDE_FROM_ALL TRUE
)

unset(_gen_go_protobufs)
