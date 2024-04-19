# CMake build file for ovs-p4rt
#
# Copyright 2022-2024 Intel Corporation
# SPDX-License-Identifier: Apache 2.0
#

include(CheckLibraryExists)

set(PROTO_INCLUDES
    # Protobuf C++ header files.
    ${PB_OUT_DIR}
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

find_package(Unbound)
find_package(Unwind)

if(DPDK_TARGET)
    add_subdirectory(dpdk)
elseif(ES2K_TARGET)
    add_subdirectory(es2k)
endif()

#################
# ovs_sidecar_o #
#################

add_library(ovs_sidecar_o OBJECT
    ovsp4rt.cc
    ovsp4rt_session.cc
    ovsp4rt_session.h
    ovsp4rt_credentials.cc
    ovsp4rt_credentials.h
    $<TARGET_OBJECTS:ovsp4rt_p4_mapping_o>
)

# Required for Ninja.
set_target_properties(ovsp4rt_p4_mapping_o PROPERTIES LINKER_LANGUAGE "C")

target_include_directories(ovs_sidecar_o PRIVATE
    ${OVS_INSTALL_DIR}/include
    ${CMAKE_CURRENT_BINARY_DIR}
)

target_include_directories(ovs_sidecar_o PRIVATE ${STRATUM_SOURCE_DIR})

target_include_directories(ovs_sidecar_o PRIVATE ${PROTO_INCLUDES})

add_dependencies(ovs_sidecar_o
    stratum_proto
    p4runtime_proto
)

target_link_libraries(ovs_sidecar_o PUBLIC stratum_static p4_role_config_proto)

################
# ovs-vswitchd #
################

add_executable(ovs-vswitchd
    $<TARGET_OBJECTS:ovs_sidecar_o>
)

set_install_rpath(ovs-vswitchd ${EXEC_ELEMENT} ${DEP_ELEMENT})

target_link_libraries(ovs-vswitchd
    PRIVATE
        -Wl,--whole-archive
        ovs::vswitchd
        -Wl,--no-whole-archive
        ovs::ofproto
        ovs::openvswitch
        ovs::sflow
    PUBLIC
        atomic
        rt m pthread
)

if(TARGET ovs::avx512)
    target_link_libraries(ovs-vswitchd PUBLIC ovs::avx512)
endif()

if(TARGET unbound)
    target_link_libraries(ovs-vswitchd PUBLIC unbound)
endif()

if(TARGET unwind::unwind)
    target_link_libraries(ovs-vswitchd PUBLIC unwind::unwind)
endif()

target_link_libraries(ovs-vswitchd PUBLIC
    absl::strings
    absl::statusor
    absl::flags_private_handle_accessor
    absl::flags
)

target_link_libraries(ovs-vswitchd PUBLIC stratum_static p4_role_config_proto)

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
        ovs::testcontroller
        -Wl,--no-whole-archive
        ovs::ofproto
        ovs::openvswitch
        ovs::sflow
        ovs::vswitchd
    PUBLIC
        atomic
        rt
)

if(TARGET ovs::avx512)
    target_link_libraries(ovs-testcontroller PUBLIC ovs::avx512)
endif()

if(TARGET unbound)
    target_link_libraries(ovs-testcontroller PUBLIC unbound)
endif()

if(TARGET unwind::unwind)
    target_link_libraries(ovs-testcontroller PUBLIC unwind::unwind)
endif()

target_link_libraries(ovs-testcontroller PUBLIC
    absl::strings
    absl::statusor
    absl::flags_private_handle_accessor
    absl::flags
)

target_link_libraries(ovs-testcontroller PUBLIC stratum_static)

target_link_libraries(ovs-testcontroller PUBLIC
    p4_role_config_proto
    p4runtime_proto
    stratum_proto
)

install(TARGETS ovs-testcontroller DESTINATION bin)
