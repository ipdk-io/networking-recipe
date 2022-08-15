# CMake build file for vswitchp4d
#
# Copyright 2022 Intel Corporation
# SPDX-License-Identifier: Apache 2.0
#
cmake_minimum_required(VERSION 3.5)

# TODO(dfoster): see if we can simplify linkages by creating a pkg-config
# file for libvswitchd.

######################
# Build ovs-vswitchd #
######################

find_file(LIBVSWITCHD
    libvswitchd.a
    PATHS ${OVS_INSTALL_PREFIX}/lib
)
if(LIBVSWITCHD STREQUAL "LIBVSWITCHD-NOTFOUND")
    message(FATAL_ERROR "Cannot find OVS libvswitchd")
endif()

add_library(ovs_sidecar_o OBJECT
    ovs-p4rt.cc
)

target_include_directories(ovs_sidecar_o PRIVATE
    ${OVS_INSTALL_PREFIX}/include
    ${STRATUM_SOURCE_DIR}
)

add_executable(ovs-vswitchd
    $<TARGET_OBJECTS:ovs_sidecar_o>
)

if(DPDK_TARGET)
    target_link_directories(ovs-vswitchd PUBLIC ${DPDK_LIBRARY_DIRS})
endif()

target_link_libraries(ovs-vswitchd
    PRIVATE
        -Wl,--whole-archive
        ${OFPROTO_LINK_LIBRARIES}
        ${LIBOVS_LINK_LIBRARIES}
        ${SFLOW_LINK_LIBRARIES}
        ${LIBVSWITCHD}
        -Wl,--no-whole-archive
    PUBLIC
        atomic
        unwind
        rt m pthread
)

if(WITH_STRATUM)
    target_link_libraries(ovs-vswitchd PUBLIC stratum)
    add_dependencies(ovs-vswitchd stratum)
endif()

install(TARGETS ovs-vswitchd DESTINATION sbin)
