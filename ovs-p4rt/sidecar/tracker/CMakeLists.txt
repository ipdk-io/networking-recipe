# CMake build file for ovs-p4rt/sidecar/tracker
#
# Copyright 2024 Intel Corporation
# SPDX-License-Identifier: Apache 2.0
#

#-----------------------------------------------------------------------
# ovsp4rt_tracker.a
#-----------------------------------------------------------------------
add_library(ovsp4rt_tracker STATIC
  ovsp4rt_encode.cc
  ovsp4rt_encode.h
  ovsp4rt_tracker.cc
  ovsp4rt_tracker.h
)

target_include_directories(ovsp4rt_tracker PUBLIC
  ${DEPEND_INSTALL_DIR}/include
  ${OVSP4RT_INCLUDE_DIR}
)

target_link_libraries(ovsp4rt_tracker PUBLIC
  p4runtime_proto
)

if(BUILD_TESTING)

find_package(GTest)

add_executable(encode_addr_test
  encode_addr_test.cc
  ovsp4rt_encode.cc
  ovsp4rt_encode.h
  test_main.cc
)

target_include_directories(encode_addr_test PUBLIC
  ${DEPEND_INSTALL_DIR}/include
  ${OVSP4RT_INCLUDE_DIR}
)

target_link_libraries(encode_addr_test PUBLIC
  GTest::gtest
  absl::flags_parse
)

add_test(NAME encode_addr_test COMMAND encode_addr_test)

endif(BUILD_TESTING)
