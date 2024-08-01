# CMake build file for ovs-p4rt/sidecar/journal
#
# Copyright 2024 Intel Corporation
# SPDX-License-Identifier: Apache 2.0
#

find_package(GTest)

# Set the "ovsp4rt" label on all tests in this directory.
set_property(DIRECTORY PROPERTY LABELS ovsp4rt)

#-----------------------------------------------------------------------
# Add ovsp4rt_journal to ovsp4rt
#-----------------------------------------------------------------------
target_sources(ovsp4rt PRIVATE
  ovsp4rt_encode.cc
  ovsp4rt_encode.h
  ovsp4rt_journal.cc
  ovsp4rt_journal.h
)

target_include_directories(ovsp4rt PUBLIC
  ${DEPEND_INSTALL_DIR}/include
  ${OVSP4RT_INCLUDE_DIR}
)

target_link_libraries(ovsp4rt PUBLIC
  p4runtime_proto
)

if(BUILD_TESTING)

#-----------------------------------------------------------------------
# encode_addr_test
#-----------------------------------------------------------------------
add_executable(encode_addr_test
  encode_addr_test.cc
  encode_base_test.h
  ovsp4rt_encode.cc
  ovsp4rt_encode.h
  test_main.cc
)

target_include_directories(encode_addr_test PUBLIC
  ${DEPEND_INSTALL_DIR}/include
  ${OVSP4RT_INCLUDE_DIR}
)

target_link_libraries(encode_addr_test PUBLIC
  absl::flags_parse
  GTest::gtest
)

add_test(NAME encode_addr_test COMMAND encode_addr_test)

list(APPEND UNIT_TEST_NAMES encode_addr_test)

# export updated list of unit tests.
set(UNIT_TEST_NAMES "${UNIT_TEST_NAMES}" PARENT_SCOPE)

endif(BUILD_TESTING)
