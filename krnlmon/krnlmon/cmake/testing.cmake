# Switchlink unit tests
#
# Copyright 2023-2024 Intel Corporation
# SPDX-License-Identifier: Apache 2.0

option(TEST_COVERAGE OFF "Measure unit test code coverage")

include(FindGTest)
mark_as_advanced(GTest_DIR)

####################
# define_unit_test #
####################

function(define_unit_test test_name)
    target_link_libraries(${test_name} PUBLIC
        GTest::gtest
        GTest::gtest_main
        PkgConfig::nl3
        sde::target_sys
    )

    target_link_directories(${test_name} PUBLIC ${DRIVER_SDK_DIRS})

    target_include_directories(${test_name} PRIVATE
        ${SDE_INSTALL_DIR}/include/target-sys
    )

    if(TEST_COVERAGE)
        target_compile_options(${test_name} PRIVATE
            -fprofile-arcs
            -ftest-coverage
        )
        target_link_libraries(${test_name} PUBLIC gcov)
    endif()

    add_test(NAME ${test_name} COMMAND ${test_name})
endfunction()

################
# krnlmon-test #
################

if(TEST_COVERAGE)
  set(test_options -T test -T coverage)
endif()

set(test_targets
  switchlink_link_test
  switchlink_address_test
  switchlink_neighbor_test
  switchlink_route_test
)

if(DPDK_TARGET)
  list(APPEND test_targets switchsde_dpdk_test)
elseif(ES2K_TARGET)
  list(APPEND test_targets switchsde_es2k_test)
endif()

# On-demand target to build and run the krnlmon tests with a
# minimum of configuration.
add_custom_target(krnlmon-test
  COMMAND
    ctest ${test_options}
    --output-on-failure
  DEPENDS
    ${test_targets}
  WORKING_DIRECTORY
    ${CMAKE_BINARY_DIR}
)

set_target_properties(krnlmon-test PROPERTIES EXCLUDE_FROM_ALL TRUE)

#######################
# build-krnlmon-tests #
#######################

add_custom_target(build-krnlmon-tests
  DEPENDS ${test_targets}
)

unset(test_options)
unset(test_targets)

####################
# krnlmon-coverage #
####################

add_custom_target(krnlmon-coverage
  COMMAND
    lcov --capture --directory ${CMAKE_BINARY_DIR}
    --output-file krnlmon.info
    --exclude '/opt/deps/*'
    --exclude '/usr/include/*'
    --exclude '9/**'
  COMMAND
    genhtml krnlmon.info --output-directory coverage
  WORKING_DIRECTORY
    ${CMAKE_BINARY_DIR}/Testing
)

set_target_properties(krnlmon-coverage PROPERTIES EXCLUDE_FROM_ALL TRUE)
