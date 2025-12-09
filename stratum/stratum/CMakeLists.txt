# CMake build file for stratum project
#
# Copyright 2022-2025 Intel Corporation
# SPDX-License-Identifier: Apache 2.0

# CMake version 3.16 is the baseline for P4 Control Plane
cmake_minimum_required(VERSION 3.16)

project(stratum LANGUAGES CXX)

if(CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
    # SDE_INSTALL_DIR
    set(SDE_INSTALL_DIR "$ENV{SDE_INSTALL}" CACHE PATH
        "SDE install directory")

    if(NOT EXISTS "${SDE_INSTALL_DIR}")
        message(FATAL_ERROR "SDE_INSTALL_DIR (SDE_INSTALL) not defined!")
    endif()

    # DEPEND_INSTALL_DIR
    set(DEPEND_INSTALL_DIR "$ENV{DEPEND_INSTALL}" CACHE PATH
        "Dependencies install directory")

    if(NOT EXISTS "${DEPEND_INSTALL_DIR}")
        message(FATAL_ERROR "DEPEND_INSTALL_DIR (DEPEND_INSTALL) not defined!")
    endif()

    list(APPEND CMAKE_PREFIX_PATH ${DEPEND_INSTALL_DIR})
endif()

# Suppress "warning: attribute ignored" on ABSL_MUST_USE_RESULT [[nodiscard]]
add_compile_options(-Wno-attributes)

add_subdirectory(stratum)

###########################
# add_stratum_object_libs #
###########################

function(add_stratum_object_libs _TGT)
    target_sources(${_TGT} PRIVATE
        $<TARGET_OBJECTS:stratum_glue_o>
        $<TARGET_OBJECTS:stratum_glue_status_o>
        $<TARGET_OBJECTS:stratum_hal_lib_common_o>
        $<TARGET_OBJECTS:stratum_hal_lib_p4_o>
        $<TARGET_OBJECTS:stratum_hal_lib_phal_o>
        $<TARGET_OBJECTS:stratum_lib_o>
        $<TARGET_OBJECTS:stratum_lib_p4runtime_o>
        $<TARGET_OBJECTS:stratum_lib_security_o>
        $<TARGET_OBJECTS:stratum_lib_utils_o>
        $<TARGET_OBJECTS:stratum_main_o>
        $<TARGET_OBJECTS:stratum_public_lib_o>
        $<TARGET_OBJECTS:stratum_tdi_common_o>
        $<TARGET_OBJECTS:stratum_tdi_target_o>
        $<TARGET_OBJECTS:stratum_yang_parse_tree_o>
    )
endfunction(add_stratum_object_libs)

#########################
# add_stratum_libraries #
#########################

# Abseil libraries
function(add_stratum_abseil_libs _TGT)
    target_link_libraries(${_TGT} PUBLIC
        absl::strings
        absl::synchronization
        absl::graphcycles_internal
        absl::stacktrace
        absl::symbolize
        absl::malloc_internal
        absl::debugging_internal
        absl::demangle_internal
        absl::time
        absl::strings_internal
        absl::throw_delegate
        absl::base
        absl::spinlock_wait
        absl::int128
        absl::raw_logging_internal
        absl::log_severity
        absl::civil_time
        absl::time_zone
        absl::status
        absl::cord
        absl::exponential_biased
        absl::str_format_internal
        absl::hash
        absl::raw_hash_set
        absl::city
        absl::bad_optional_access
        absl::bad_variant_access
    )
    if(absl_VERSION VERSION_GREATER_EQUAL 20230125)
        target_link_libraries(${_TGT} PUBLIC absl::log_internal_check_op)
    endif()
endfunction()

# Google libraries
function(add_stratum_google_libs _TGT)
    target_link_libraries(${_TGT} PUBLIC
        gflags::gflags_shared
        glog::glog
        gRPC::gpr
        gRPC::grpc
        gRPC::grpc++
        protobuf::libprotobuf
    )
endfunction()

# Protobuf libraries
function(add_stratum_proto_libs _TGT)
    target_link_libraries(${_TGT} PUBLIC
        openconfig_proto
        stratum_proto
        gnmi_proto
        google_rpc_proto
        p4runtime_proto
    )
endfunction()

function(add_stratum_libraries _TGT)
    add_stratum_abseil_libs(${_TGT})
    add_stratum_google_libs(${_TGT})
    add_stratum_proto_libs(${_TGT})
    target_link_libraries(${_TGT} PUBLIC pthread)
endfunction()

#####################
# libstratum_static #
#####################

add_library(stratum_static STATIC)
add_stratum_object_libs(stratum_static)
add_stratum_libraries(stratum_static)

target_include_directories(stratum_static PUBLIC ${STRATUM_INCLUDES})

#####################
# libstratum_shared #
#####################

add_library(stratum_shared SHARED)
add_stratum_object_libs(stratum_shared)
add_stratum_libraries(stratum_shared)

target_include_directories(stratum_shared PUBLIC ${STRATUM_INCLUDES})
