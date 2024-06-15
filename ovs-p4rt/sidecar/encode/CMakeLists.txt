# CMake build file for ovs-p4rt/sidecar/encode
#
# Copyright 2024 Intel Corporation
# SPDX-License-Identifier: Apache 2.0
#

include(FindGTest)
mark_as_advanced(GTest_DIR)

function(define_encode_test TARGET)
  target_include_directories(${TARGET} PUBLIC
    ${OVSP4RT_INCLUDE_DIR}
    ${SIDECAR_SOURCE_DIR}
  )

  target_link_libraries(${TARGET} PUBLIC
    GTest::gtest
    GTest::gtest_main
    ovsp4rt_utils
    p4runtime_proto
    stratum_static
  )
endfunction(define_encode_test)

if(ES2K_TARGET)

target_sources(ovs_sidecar_o PRIVATE
  PrepareL2ToTunnel.cc
  PrepareL2ToTunnel.h
)

add_executable(prepare_l2_to_tunnel_test
  prepare_l2_to_tunnel_test.cc
  PrepareL2ToTunnel.cc
  PrepareL2ToTunnel.h
)

define_encode_test(prepare_l2_to_tunnel_test)

endif(ES2K_TARGET)
