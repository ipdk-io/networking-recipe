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

# Set the "ovsp4rt" label on all tests in this directory.
# We can use this with ctest to filter the tests to be run.
set_property(DIRECTORY PROPERTY LABELS ovsp4rt)

#-----------------------------------------------------------------------
# ipv4_test_utils
#-----------------------------------------------------------------------
add_library(ipv4_test_utils STATIC
  ipv4_tunnel_test.h
  p4info_text.h
  table_entry_test.h
  test_main.cc
)

target_link_libraries(ipv4_test_utils PUBLIC
  absl::flags_parse
)

#-----------------------------------------------------------------------
# ipv6_test_utils
#-----------------------------------------------------------------------
add_library(ipv6_test_utils STATIC
  ipv6_tunnel_test.h
  p4info_text.h
  table_entry_test.h
  test_main.cc
)

target_link_libraries(ipv6_test_utils PUBLIC
  absl::flags_parse
)

#-----------------------------------------------------------------------
# encode_host_port_value_test
#-----------------------------------------------------------------------
add_executable(encode_host_port_value_test
  encode_host_port_value_test.cc
  p4info_text.h
)

set_test_properties(encode_host_port_value_test)

target_link_libraries(encode_host_port_value_test PUBLIC
  GTest::gtest_main
)

if(ES2K_TARGET)

#-----------------------------------------------------------------------
# geneve_encap_table_entry_test
#-----------------------------------------------------------------------
add_executable(geneve_encap_table_entry_test
  geneve_encap_table_entry_test.cc
  p4info_text.h
  test_main.cc
)

set_test_properties(geneve_encap_table_entry_test)

target_link_libraries(geneve_encap_table_entry_test PUBLIC
  absl::flags_parse
)

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

#-----------------------------------------------------------------------
# vxlan_encap_v4_vlan_pop_test (ES2K)
#-----------------------------------------------------------------------
add_executable(vxlan_encap_v4_vlan_pop_test
  vxlan_encap_v4_vlan_pop_test.cc
)

set_test_properties(vxlan_encap_v4_vlan_pop_test)

target_link_libraries(vxlan_encap_v4_vlan_pop_test PUBLIC
  ipv4_test_utils
)

endif(ES2K_TARGET)