# CMake build file for ovs-p4rt/sidecar/testing
#
# Copyright 2024 Intel Corporation
# SPDX-License-Identifier: Apache-2.0
#

#=======================================================================
# Unit tests
#=======================================================================

include(FindGTest)
mark_as_advanced(GTest_DIR)

include(set_test_properties.cmake)

option(TEST_COVERAGE OFF "Measure unit test code coverage")

#-----------------------------------------------------------------------
# p4info_file_path.h - header file specifying the path to p4Info.txt
#-----------------------------------------------------------------------
set(P4INFO_FILE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/p4Info.txt")

if(NOT EXISTS "${P4INFO_FILE_PATH}")
  message(WARNING "p4Info.txt not found: OVSP4RT unit tests will not be built")
  return()
endif()

configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/p4info_file_path.h.in
  ${CMAKE_CURRENT_BINARY_DIR}/p4info_file_path.h
  @ONLY
)

if(ES2K_TARGET)

#-----------------------------------------------------------------------
# prepare_l2_to_tunnel_test
#-----------------------------------------------------------------------
add_executable(prepare_l2_to_tunnel_test
  prepare_l2_to_tunnel_test.cc
  test_main.cc
)

set_test_properties(prepare_l2_to_tunnel_test)

target_link_libraries(prepare_l2_to_tunnel_test PUBLIC
  absl::flags_parse
)

endif(ES2K_TARGET)
