#
# Copyright 2023 Intel Corporation
# SPDX-License-Identifier: Apache 2.0
#
# Builds and installs tarballs.
#

set(tarball_suffix ${CMAKE_PROJECT_VERSION}.tar.gz)
set(cpp_tarball_name p4runtime-cpp-${tarball_suffix})
set(py_tarball_name p4runtime-py-${tarball_suffix})
set(go_tarball_name p4runtime-go-${tarball_suffix})

get_filename_component(cpp_dir ${CPP_OUT} NAME_WE)
get_filename_component(py_dir ${PY_OUT} NAME_WE)
get_filename_component(go_dir ${GO_OUT} NAME_WE)

set(tar_flags -z --owner=ipdk --group=ipdk --sort=name)

# C++ tarball
add_custom_target(cpp-tarball ALL
  COMMAND
    tar -cf ${cpp_tarball_name}
    ${tar_flags}
    -C ${CPP_OUT}/..
    ${cpp_dir}
  DEPENDS
    google_cpp_out
    p4rt_cpp_out
    p4rt_grpc_out
  BYPRODUCTS
    ${CMAKE_CURRENT_BINARY_DIR}/${cpp_tarball_name}
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
    ${tar_flags}
    -C ${PY_OUT}/..
    ${py_dir}
  DEPENDS
    google_py_out
    p4rt_py_out
  BYPRODUCTS
    ${CMAKE_CURRENT_BINARY_DIR}/${py_tarball_name}
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
      ${tar_flags}
      -C ${GO_OUT}/..
      ${go_dir}
    DEPENDS
      google_go_out
      p4rt_go_out
    BYPRODUCTS
      ${CMAKE_CURRENT_BINARY_DIR}/${go_tarball_name}
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
    ${CMAKE_INSTALL_DATAROOTDIR}/p4runtime
)
