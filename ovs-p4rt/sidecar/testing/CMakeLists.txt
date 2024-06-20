# CMake build file for ovs-p4rt/sidecar/testing
#
# Copyright 2024 Intel Corporation
# SPDX-License-Identifier: Apache-2.0
#

#=======================================================================
# Manual tests
#=======================================================================

add_executable(ovsp4rt_logging_test EXCLUDE_FROM_ALL
  ovsp4rt_logging_test.cc
)

target_include_directories(ovsp4rt_logging_test PUBLIC
  ${SIDECAR_SOURCE_DIR}
  ${OVSP4RT_INCLUDE_DIR}
)

target_link_libraries(ovsp4rt_logging_test PUBLIC
  ovsp4rt_logging_so
  sde::target_sys
)

#=======================================================================
# Unit tests (common)
#=======================================================================

include(FindGTest)
mark_as_advanced(GTest_DIR)

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

#=======================================================================
# Unit tests (tests)
#=======================================================================

include(unit_test_properties.cmake)

if(ES2K_TARGET)

#-----------------------------------------------------------------------
# prepare_l2_to_tunnel_test
#-----------------------------------------------------------------------
add_executable(prepare_l2_to_tunnel_test
  prepare_l2_to_tunnel_test.cc
)

set_ovsp4rt_unit_test_properties(prepare_l2_to_tunnel_test)

endif(ES2K_TARGET)
