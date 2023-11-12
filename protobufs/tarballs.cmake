#
# Copyright 2023 Intel Corporation
# SPDX-License-Identifier: Apache 2.0
#
# Builds and installs tarballs.
#

set(cpp_tarball_name p4runtime-cpp-0.0.0.tar.gz)
set(py_tarball_name p4runtime-py-0.0.0.tar.gz)
set(go_tarball_name p4runtime-go-0.0.0.tar.gz)

set(_tar_flags -z --owner=ipdk --group=ipdk --sort=name)

# C++ tarball
add_custom_target(cpp-tarball ALL
  COMMAND
    tar -cf ${cpp_tarball_name}
    ${_tar_flags}
    -C ${CPP_OUT}/..
    cpp_out
  DEPENDS
    google_cpp_out
    p4rt_cpp_out
    p4rt_grpc_out
  BYPRODUCTS
    ${cpp_tarball_name}
  WORKING_DIRECTORY
    ${CMAKE_CURRENT_BINARY_DIR}
  COMMENT
    "Generating C++ tarball"
  VERBATIM
)

# Python tarball
add_custom_target(py-tarball ALL
  COMMAND
    tar -cf ${py_tarball_name}
    ${_tar_flags}
    -C ${PY_OUT}/..
    py_out
  DEPENDS
    google_py_out
    p4rt_py_out
  BYPRODUCTS
    ${py_tarball_name}
  WORKING_DIRECTORY
    ${CMAKE_CURRENT_BINARY_DIR}
  COMMENT
    "Generating Python tarball"
  VERBATIM
)

# Go tarball
if(GEN_GO_PROTOBUFS)
  add_custom_target(go-tarball ALL
    COMMAND
      tar -cf ${go_tarball_name}
      ${_tar_flags}
      -C ${GO_OUT}/..
      go_out
    DEPENDS
      google_go_out
      p4rt_go_out
    BYPRODUCTS
      ${go_tarball_name}
    WORKING_DIRECTORY
      ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT
      "Generating Go tarball"
    VERBATIM
  )
  set(go_tarball ${CMAKE_CURRENT_BINARY_DIR}/${go_tarball_name})
else(GEN_GO_PROTOBUFS)
  add_custom_target(go-tarball ALL
    COMMAND ""
    COMMENT "go-tarball is disabled"
  )
  set(go_tarball "")
endif(GEN_GO_PROTOBUFS)

install(
  FILES
    ${CMAKE_CURRENT_BINARY_DIR}/${cpp_tarball_name}
    ${CMAKE_CURRENT_BINARY_DIR}/${py_tarball_name}
    ${go_tarball}
  DESTINATION
    ${CMAKE_INSTALL_DATAROOTDIR}/stratum/protobufs
)
