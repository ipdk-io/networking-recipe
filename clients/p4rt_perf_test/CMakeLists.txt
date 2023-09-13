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

set(SRC_FILES
    p4rt_perf_main.cc
    p4rt_perf_session.cc
    p4rt_perf_session.h
    p4rt_perf_simple_l2_demo.cc
    p4rt_perf_simple_l2_demo.h
    p4rt_perf_tls_credentials.cc
    p4rt_perf_tls_credentials.h
    p4rt_perf_test.h
    p4rt_perf_util.cc
    p4rt_perf_util.h
)

add_executable(p4rt_perf_test ${SRC_FILES})

target_compile_options(p4rt_perf_test PRIVATE -O3)

target_include_directories(p4rt_perf_test PRIVATE ${PROTO_INCLUDES})

add_dependencies(p4rt_perf_test
    stratum_proto
    p4runtime_proto
)

target_link_libraries(p4rt_perf_test PUBLIC
    absl::flags_private_handle_accessor
    absl::flags
    absl::statusor
    absl::strings
    p4runtime_proto
    stratum_static
    stratum_proto
)

install(TARGETS p4rt_perf_test DESTINATION bin)
