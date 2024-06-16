# CMake build file for ovs-p4rt/sidecar/testing
#
# Copyright 2024 Intel Corporation
# SPDX-License-Identifier: Apache-2.0
#

#=======================================================================
# Original contents
#=======================================================================

if(FALSE)

add_library(ovsp4rt_logging_o OBJECT
  ${SIDECAR_SOURCE_DIR}/common/ovsp4rt_logutils.cc
  ${SIDECAR_SOURCE_DIR}/common/ovsp4rt_logutils.h
  ${SIDECAR_SOURCE_DIR}/lib/ovsp4rt_logging.cc
  ${SIDECAR_SOURCE_DIR}/lib/ovsp4rt_logging.h
)

target_include_directories(ovsp4rt_logging_o PRIVATE
  ${SIDECAR_SOURCE_DIR}
  ${SDE_INSTALL_DIR}/include
)

add_library(ovsp4rt_logging SHARED $<TARGET_OBJECTS:ovsp4rt_logging_o>)

add_executable(logging_test EXCLUDE_FROM_ALL
  logging_test.cc
)

target_include_directories(logging_test PUBLIC
  ${SIDECAR_SOURCE_DIR}
  ${DEPEND_INSTALL_DIR}/include
  ${SDE_INSTALL_DIR}/include
  ${OVSP4RT_INCLUDE_DIR}
)

target_link_libraries(logging_test PUBLIC
  ovsp4rt_logging
  absl::strings
  sde::target_sys
)

endif(FALSE)

#=======================================================================
# New contents
#=======================================================================

include(FindGTest)
mark_as_advanced(GTest_DIR)

option(TEST_COVERAGE OFF "Measure unit test code coverage")

#-----------------------------------------------------------------------
# p4info_file_path.h
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

#-----------------------------------------------------------------------
# define_sidecar_unit_test
#-----------------------------------------------------------------------
function(define_sidecar_unit_test TARGET)
  target_include_directories(${TARGET} PUBLIC
    ${OVSP4RT_INCLUDE_DIR}
    ${SIDECAR_SOURCE_DIR}
    ${STRATUM_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
  )

  target_link_libraries(${TARGET} PUBLIC
    GTest::gtest
    GTest::gtest_main
    ovs_sidecar
    p4runtime_proto
  )

  if(TEST_COVERAGE)
      target_compile_options(${TARGET} PRIVATE
          -fprofile-arcs
          -ftest-coverage
      )
      target_link_libraries(${TARGET} PUBLIC gcov)
  endif()

  add_test(NAME ${TARGET} COMMAND ${TARGET})
endfunction(define_sidecar_unit_test)

if(ES2K_TARGET)

#-----------------------------------------------------------------------
# prepare_l2_to_tunnel_test
#-----------------------------------------------------------------------
add_executable(prepare_l2_to_tunnel_test
  prepare_l2_to_tunnel_test.cc
)

define_sidecar_unit_test(prepare_l2_to_tunnel_test)

endif(ES2K_TARGET)
