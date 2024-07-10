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

if(ES2K_TARGET)

#-----------------------------------------------------------------------
# prepare_l2_to_tunnel_test
#-----------------------------------------------------------------------
add_executable(prepare_l2_to_tunnel_test
  prepare_l2_to_tunnel_test.cc
  p4info_text.h
  test_main.cc
)

set_test_properties(prepare_l2_to_tunnel_test)

target_link_libraries(prepare_l2_to_tunnel_test PUBLIC
  absl::flags_parse
)

endif(ES2K_TARGET)
