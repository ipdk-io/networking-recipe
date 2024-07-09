#
# Copyright 2024 Intel Corporation
# SPDX-License-Identifier: Apache-2.0
#

#-----------------------------------------------------------------------
# set_test_properties()
#-----------------------------------------------------------------------
function(set_test_properties TARGET)

  target_include_directories(${TARGET} PUBLIC
    ${OVSP4RT_INCLUDE_DIR}
    ${SIDECAR_SOURCE_DIR}
    ${STRATUM_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
  )

  target_link_libraries(${TARGET} PUBLIC
    GTest::gtest
    ovsp4rt
    p4runtime_proto
    stratum_utils
  )

  if(TEST_COVERAGE)
      target_compile_options(${TARGET} PRIVATE
          -fprofile-arcs
          -ftest-coverage
      )
      target_link_libraries(${TARGET} PUBLIC gcov)
  endif()

  add_test(NAME ${TARGET} COMMAND ${TARGET})

endfunction(set_test_properties)

