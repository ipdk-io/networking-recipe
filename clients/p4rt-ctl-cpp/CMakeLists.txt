# CMake build file for p4rt_perf_test
#
# Copyright 2023-2024 Intel Corporation
# SPDX-License-Identifier: Apache 2.0
#

include(CheckLibraryExists)

set(PROTO_INCLUDES
    # Protobuf C++ header files.
    ${PB_OUT_DIR}
)

#################
# p4rt_perf_test #
#################

add_library(p4rt_perf_test_o OBJECT
    p4rt_perf.cc
    p4rt_perf_session.cc
    p4rt_perf_session.h
    p4rt_perf_tls_credentials.cc
    p4rt_perf_tls_credentials.h
    p4rt_perf_simple_l2_demo.cc
    p4rt_perf_util.cc
)

target_compile_options(p4rt_perf_test_o PRIVATE -O3)
target_include_directories(p4rt_perf_test_o PRIVATE
    ${CMAKE_CURRENT_BINARY_DIR}
)

target_include_directories(p4rt_perf_test_o PRIVATE ${PROTO_INCLUDES})

add_dependencies(p4rt_perf_test_o
    stratum_proto
    p4runtime_proto
)


add_executable(p4rt_perf_test
    $<TARGET_OBJECTS:p4rt_perf_test_o>
)

target_link_libraries(p4rt_perf_test PUBLIC
    absl::strings
    absl::statusor
    absl::flags_private_handle_accessor
    absl::flags
)

target_link_libraries(p4rt_perf_test PUBLIC stratum_static)

target_link_libraries(p4rt_perf_test PUBLIC
    stratum_proto
    p4runtime_proto
)

install(TARGETS p4rt_perf_test DESTINATION bin)
