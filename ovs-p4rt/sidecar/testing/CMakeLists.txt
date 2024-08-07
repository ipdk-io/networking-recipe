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
# define_ovsp4rt_test()
#-----------------------------------------------------------------------
macro(define_ovsp4rt_test TARGET)
  add_executable(${TARGET}
    ${TARGET}.cc
    p4info_text.h
    test_main.cc
  )

  set_test_properties(${TARGET})

  target_link_libraries(${TARGET} PUBLIC
    absl::flags_parse
  )

  list(APPEND UNIT_TEST_NAMES ${TARGET})
endmacro(define_ovsp4rt_test)

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
# define_ipv4_tunnel_test()
#-----------------------------------------------------------------------
macro(define_ipv4_tunnel_test TARGET)
  add_executable(${TARGET}
    ${TARGET}.cc
  )

  set_test_properties(${TARGET})

  target_link_libraries(${TARGET} PUBLIC
    ipv4_test_utils
  )

  list(APPEND UNIT_TEST_NAMES ${TARGET})
endmacro(define_ipv4_tunnel_test)

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
# define_ipv6_tunnel_test()
#-----------------------------------------------------------------------
macro(define_ipv6_tunnel_test TARGET)
  add_executable(${TARGET}
    ${TARGET}.cc
  )

  set_test_properties(${TARGET})

  target_link_libraries(${TARGET} PUBLIC
    ipv4_test_utils
  )

  list(APPEND UNIT_TEST_NAMES ${TARGET})
endmacro(define_ipv6_tunnel_test)

#-----------------------------------------------------------------------
# encode_host_port_value_test (DPDK, ES2K)
#-----------------------------------------------------------------------
add_executable(encode_host_port_value_test
  encode_host_port_value_test.cc
  p4info_text.h
)

set_test_properties(encode_host_port_value_test)

target_link_libraries(encode_host_port_value_test PUBLIC
  GTest::gtest_main
)

list(APPEND UNIT_TEST_NAMES encode_host_port_value_test)

if(ES2K_TARGET)
#-----------------------------------------------------------------------
# ES2K unit tests
#-----------------------------------------------------------------------
define_ovsp4rt_test(fdb_rx_vlan_entry_test)
define_ovsp4rt_test(fdb_smac_entry_test)
define_ovsp4rt_test(fdb_tx_vlan_entry_test)

define_ipv4_tunnel_test(geneve_encap_v4_table_entry_test)
define_ipv6_tunnel_test(geneve_encap_v6_table_entry_test)
define_ipv4_tunnel_test(geneve_encap_v4_vlan_pop_test)

define_ovsp4rt_test(l2_to_v4_tunnel_test)
define_ovsp4rt_test(l2_to_v6_tunnel_test)

define_ovsp4rt_test(rx_tunnel_v4_table_test)

define_ovsp4rt_test(tunnel_term_v4_table_test)
define_ovsp4rt_test(tunnel_term_v6_table_test)

define_ovsp4rt_test(vxlan_decap_mod_entry_test)

define_ipv4_tunnel_test(vxlan_encap_v4_table_entry_test)
define_ipv6_tunnel_test(vxlan_encap_v6_table_entry_test)
define_ipv4_tunnel_test(vxlan_encap_v4_vlan_pop_test)
define_ipv6_tunnel_test(vxlan_encap_v6_vlan_pop_test)

endif(ES2K_TARGET)

# Export list of unit tests.
set(UNIT_TEST_NAMES "${UNIT_TEST_NAMES}" PARENT_SCOPE)
