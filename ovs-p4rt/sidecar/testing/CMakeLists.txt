# CMake build file for ovs-p4rt/sidecar/testing
#
# Copyright 2024 Intel Corporation
# SPDX-License-Identifier: Apache-2.0
#

add_executable(logging_test EXCLUDE_FROM_ALL
  logging_test.cc
)

target_include_directories(logging_test PUBLIC
  ${SIDECAR_SOURCE_DIR}
  ${OVSP4RT_INCLUDE_DIR}
)

target_link_libraries(logging_test PUBLIC
  ovsp4rt_logging_so
  sde::target_sys
)
