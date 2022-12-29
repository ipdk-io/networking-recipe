# CMake build file for ovs-p4rt
#
# Copyright 2022 Intel Corporation
# SPDX-License-Identifier: Apache 2.0
#
cmake_minimum_required(VERSION 3.5)

include(CheckLibraryExists)

set(PROTO_INCLUDES
    # Protobuf C++ header files.
    ${PROTO_OUT_DIR}
)

###################
# Use OVS package #
###################

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

include(CheckBinutilsAVX512)
include(CheckCCompilerFlag)

check_library_exists(rt aio_write /usr/lib/x86_64-linux-gnu HAVE_POSIX_AIO)
check_c_compiler_flag(-mavx512f HAVE_AVX512F)
check_binutils_avx512(HAVE_LD_AVX512_GOOD)

# Generate config.h file and add it to the include path.
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/config.h.in"
               "${CMAKE_CURRENT_BINARY_DIR}/config.h")

# Update pkg-config search path.
set(PKG_CONFIG_USE_CMAKE_PREFIX_PATH TRUE)

pkg_check_modules(LIBOVS REQUIRED libopenvswitch)
pkg_check_modules(OFPROTO REQUIRED libofproto)
pkg_check_modules(SFLOW REQUIRED libsflow)

message(STATUS "Checking for library 'libvswitchd'")
find_library(LIBVSWITCHD vswitchd)
if(LIBVSWITCHD STREQUAL "LIBVSWITCHD-NOTFOUND")
    message(FATAL_ERROR "Cannot find OVS libvswitchd")
endif()
message(STATUS "  Found ${LIBVSWITCHD}")

message(STATUS "Checking for library 'libtestcontroller'")
find_library(LIBTESTCONTROLLER testcontroller)
if(NOT LIBTESTCONTROLLER)
  message(FATAL_ERROR "Cannot find OVS libtestcontroller")
endif()
message(STATUS "  Found ${LIBTESTCONTROLLER}")

#################
# ovs_sidecar_o #
#################

add_library(ovs_sidecar_o OBJECT
    ovs_p4rt.cc
    ovs_p4rt_session.cc
    ovs_p4rt_session.h
    p4_name_mapping.h
)

target_include_directories(ovs_sidecar_o PRIVATE
    ${OVS_INSTALL_DIR}/include
    ${CMAKE_CURRENT_BINARY_DIR}
)

target_include_directories(ovs_sidecar_o PRIVATE ${PROTO_INCLUDES})

add_dependencies(ovs_sidecar_o
    stratum_proto
    p4runtime_proto
)

################
# ovs-vswitchd #
################

add_executable(ovs-vswitchd
    $<TARGET_OBJECTS:ovs_sidecar_o>
)

target_link_libraries(ovs-vswitchd
    PRIVATE
        -Wl,--whole-archive
        ${LIBVSWITCHD}
        -Wl,--no-whole-archive
        ${OFPROTO_LINK_LIBRARIES}
        ${LIBOVS_LINK_LIBRARIES}
        ${SFLOW_LINK_LIBRARIES}
    PUBLIC
        atomic
        unwind
        rt m pthread
)

set_install_rpath(ovs-vswitchd ${EXEC_ELEMENT} ${DEP_ELEMENT})

target_link_libraries(ovs-vswitchd PUBLIC
    absl::strings
    absl::statusor
    absl::flags_private_handle_accessor
    absl::flags
)

target_link_libraries(ovs-vswitchd PUBLIC stratum)
add_dependencies(ovs-vswitchd stratum)

target_link_libraries(ovs-vswitchd PUBLIC
    stratum_proto
    p4runtime_proto
)

install(TARGETS ovs-vswitchd DESTINATION sbin)

######################
# ovs-testcontroller #
######################

add_executable(ovs-testcontroller
    $<TARGET_OBJECTS:ovs_sidecar_o>
)

set_install_rpath(ovs-testcontroller ${EXEC_ELEMENT} ${DEP_ELEMENT})

target_link_libraries(ovs-testcontroller
    PRIVATE
        -Wl,--whole-archive
        ${LIBTESTCONTROLLER}
        -Wl,--no-whole-archive
        ${OFPROTO_LINK_LIBRARIES}
        ${LIBOVS_LINK_LIBRARIES}
        ${SFLOW_LINK_LIBRARIES}
        ${LIBVSWITCHD}
    PUBLIC
        atomic
        rt
        unwind
)

target_link_libraries(ovs-testcontroller PUBLIC
    absl::strings
    absl::statusor
    absl::flags_private_handle_accessor
    absl::flags
)

target_link_libraries(ovs-testcontroller PUBLIC stratum)

target_link_libraries(ovs-testcontroller PUBLIC
    stratum_proto
    p4runtime_proto
)

install(TARGETS ovs-testcontroller DESTINATION bin)
