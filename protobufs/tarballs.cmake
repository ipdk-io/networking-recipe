#
# Copyright 2023 Intel Corporation
# SPDX-License-Identifier: Apache 2.0
#
# Builds and installs tarballs.
#

set(tarball_suffix ${CMAKE_PROJECT_VERSION}.tar.gz)
set(tar_flags -z --owner=ipdk --group=ipdk --sort=name)

# C++ tarball
set(cpp_tarball_name p4runtime-cpp-${tarball_suffix})
set(cpp_tarball ${CMAKE_CURRENT_BINARY_DIR}/${cpp_tarball_name})
get_filename_component(cpp_dir ${CPP_OUT} NAME_WE)

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
    ${cpp_tarball}
  WORKING_DIRECTORY
    ${CMAKE_CURRENT_BINARY_DIR}
  COMMENT
    "Generating C++ tarball"
  VERBATIM
)

# Python source tarball is generated when we build the wheel.

# Go tarball
if(GEN_GO_PROTOBUFS)
  set(go_tarball_name p4runtime-go-${tarball_suffix})
  set(go_tarball ${CMAKE_CURRENT_BINARY_DIR}/${go_tarball_name})
  get_filename_component(go_dir ${GO_OUT} NAME_WE)

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
      ${go_tarball}
    WORKING_DIRECTORY
      ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT
      "Generating Go tarball"
    VERBATIM
  )
else()
  set(go_tarball "")
endif()

install(
  FILES
    ${cpp_tarball}
    ${go_tarball}
  DESTINATION
    ${CMAKE_INSTALL_DATAROOTDIR}/p4runtime
)
