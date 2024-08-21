# CMake build file for ovs-p4rt/sidecar/tests/es2k
#
# Copyright 2024 Intel Corporation
# SPDX-License-Identifier: Apache-2.0
#

#-----------------------------------------------------------------------
# dpdk_ovsp4rt_test
#-----------------------------------------------------------------------
add_library(dpdk_ovsp4rt_test INTERFACE
  base_table_test.h
  p4info_helper.h
  p4info_text.h
)

target_include_directories(dpdk_ovsp4rt_test INTERFACE
  ${SIDECAR_SOURCE_DIR}
  ${STRATUM_SOURCE_DIR}
)

target_link_libraries(dpdk_ovsp4rt_test INTERFACE
  absl::flags_parse
  p4runtime_proto
  stratum_utils
)

#-----------------------------------------------------------------------
# define_dpdk_test()
#-----------------------------------------------------------------------
macro(define_dpdk_test TARGET)
  add_executable(${TARGET}
    ${TARGET}.cc
    test_main.cc
  )

  set_test_properties(${TARGET})

  target_link_libraries(${TARGET} PUBLIC
    dpdk_ovsp4rt_test
  )

  list(APPEND UNIT_TEST_NAMES ${TARGET})
endmacro(define_dpdk_test)

#-----------------------------------------------------------------------
# DPDK unit tests
#-----------------------------------------------------------------------
define_dpdk_test(dpdk_fdb_rx_vlan_test)
define_dpdk_test(dpdk_fdb_tx_vlan_test)

define_dpdk_test(dpdk_fdb_tx_geneve_test)
define_dpdk_test(dpdk_fdb_tx_vxlan_test)

# Export list of unit tests.
set(UNIT_TEST_NAMES "${UNIT_TEST_NAMES}" PARENT_SCOPE)
