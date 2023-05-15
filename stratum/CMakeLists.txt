# CMake build file for Stratum code.
#
# Copyright 2022-2023 Intel Corporation
# SPDX-License-Identifier: Apache 2.0
#

cmake_minimum_required(VERSION 3.12)

project(stratum)

#############################
# Symbolic path definitions #
#############################

set(GOOGLEAPIS_SOURCE_DIR
    "${CMAKE_CURRENT_SOURCE_DIR}/googleapis" CACHE PATH
    "Path to Google APIs source directory")

set(P4RUNTIME_SOURCE_DIR
    "${CMAKE_CURRENT_SOURCE_DIR}/p4runtime" CACHE PATH
    "Path to P4Runtime source directory")

set(STRATUM_SOURCE_DIR
    "${CMAKE_CURRENT_SOURCE_DIR}/stratum" CACHE PATH
    "Path to Stratum source directory")

set(PB_OUT_DIR "${CMAKE_BINARY_DIR}/pb.out" CACHE PATH
    "Path to generated Protobuf files")
file(MAKE_DIRECTORY ${PB_OUT_DIR})

set(STRATUM_INCLUDES
    ${STRATUM_SOURCE_DIR}
    # Protobuf C++ header files.
    ${PB_OUT_DIR}
)

#####################
# Compile Protobufs #
#####################

option(INSTALL_PROTO_FILES "Install .proto files" OFF)

add_subdirectory(proto)

# Suppress "warning: attribute ignored" on ABSL_MUST_USE_RESULT [[nodiscard]]
add_compile_options(-Wno-attributes)

##################
# stratum_glue_o #
##################

add_library(stratum_glue_o OBJECT
    ${STRATUM_SOURCE_DIR}/stratum/glue/gtl/cleanup.h
    ${STRATUM_SOURCE_DIR}/stratum/glue/integral_types.h
    ${STRATUM_SOURCE_DIR}/stratum/glue/logging.cc
    ${STRATUM_SOURCE_DIR}/stratum/glue/logging.h
    ${STRATUM_SOURCE_DIR}/stratum/glue/status/posix_error_space.cc
    ${STRATUM_SOURCE_DIR}/stratum/glue/status/posix_error_space.h
    ${STRATUM_SOURCE_DIR}/stratum/glue/status/status.cc
    ${STRATUM_SOURCE_DIR}/stratum/glue/status/status.h
    ${STRATUM_SOURCE_DIR}/stratum/glue/status/status_macros.cc
    ${STRATUM_SOURCE_DIR}/stratum/glue/status/status_macros.h
    ${STRATUM_SOURCE_DIR}/stratum/glue/status/statusor.cc
    ${STRATUM_SOURCE_DIR}/stratum/glue/status/statusor.h
)

target_include_directories(stratum_glue_o PRIVATE ${STRATUM_INCLUDES})

#################
# stratum_lib_o #
#################

add_library(stratum_lib_o OBJECT
    ${STRATUM_SOURCE_DIR}/stratum/lib/timer_daemon.cc
    ${STRATUM_SOURCE_DIR}/stratum/lib/timer_daemon.h
    ${STRATUM_SOURCE_DIR}/stratum/lib/channel/channel.h
    ${STRATUM_SOURCE_DIR}/stratum/lib/channel/channel_internal.h
    ${STRATUM_SOURCE_DIR}/stratum/lib/macros.h
    ${STRATUM_SOURCE_DIR}/stratum/lib/security/auth_policy_checker.cc
    ${STRATUM_SOURCE_DIR}/stratum/lib/security/auth_policy_checker.h
    ${STRATUM_SOURCE_DIR}/stratum/lib/security/credentials_manager.cc
    ${STRATUM_SOURCE_DIR}/stratum/lib/security/credentials_manager.h
    ${STRATUM_SOURCE_DIR}/stratum/lib/utils.cc
    ${STRATUM_SOURCE_DIR}/stratum/lib/utils.h
    ${STRATUM_SOURCE_DIR}/stratum/public/lib/error.cc
    ${STRATUM_SOURCE_DIR}/stratum/public/lib/error.h
)

target_include_directories(stratum_lib_o PRIVATE ${STRATUM_INCLUDES})

add_dependencies(stratum_lib_o stratum_proto gnmi_proto)

############################
# stratum_hal_lib_common_o #
############################

add_library(stratum_hal_lib_common_o OBJECT
    ${STRATUM_SOURCE_DIR}/stratum/hal/lib/common/channel_writer_wrapper.h
    ${STRATUM_SOURCE_DIR}/stratum/hal/lib/common/config_monitoring_service.cc
    ${STRATUM_SOURCE_DIR}/stratum/hal/lib/common/config_monitoring_service.h
    ${STRATUM_SOURCE_DIR}/stratum/hal/lib/common/error_buffer.cc
    ${STRATUM_SOURCE_DIR}/stratum/hal/lib/common/error_buffer.h
    ${STRATUM_SOURCE_DIR}/stratum/hal/lib/common/gnmi_publisher.cc
    ${STRATUM_SOURCE_DIR}/stratum/hal/lib/common/gnmi_publisher.h
    ${STRATUM_SOURCE_DIR}/stratum/hal/lib/common/openconfig_converter.cc
    ${STRATUM_SOURCE_DIR}/stratum/hal/lib/common/openconfig_converter.h
    ${STRATUM_SOURCE_DIR}/stratum/hal/lib/common/p4_service.cc
    ${STRATUM_SOURCE_DIR}/stratum/hal/lib/common/p4_service.h
    ${STRATUM_SOURCE_DIR}/stratum/hal/lib/common/server_writer_wrapper.h
    ${STRATUM_SOURCE_DIR}/stratum/hal/lib/common/utils.cc
    ${STRATUM_SOURCE_DIR}/stratum/hal/lib/common/utils.h
    ${STRATUM_SOURCE_DIR}/stratum/hal/lib/common/writer_interface.h
)

set_source_files_properties(
    ${STRATUM_SOURCE_DIR}/stratum/hal/lib/common/config_monitoring_service.cc
    PROPERTIES COMPILE_OPTIONS "-Wno-deprecated-declarations"
)

target_include_directories(stratum_hal_lib_common_o PRIVATE
    ${STRATUM_INCLUDES}
    ${SDE_INSTALL_DIR}/include
)

add_dependencies(stratum_hal_lib_common_o
    gnmi_proto
    openconfig_proto
    stratum_proto
)

##########################
# stratum_hal_lib_phal_o #
##########################

add_library(stratum_hal_lib_phal_o OBJECT
    ${STRATUM_SOURCE_DIR}/stratum/hal/lib/phal/phal_sim.cc
    ${STRATUM_SOURCE_DIR}/stratum/hal/lib/phal/phal_sim.h
)

target_include_directories(stratum_hal_lib_phal_o PRIVATE ${STRATUM_INCLUDES})

add_dependencies(stratum_hal_lib_phal_o stratum_proto)

########################
# stratum_hal_lib_p4_o #
########################

add_library(stratum_hal_lib_p4_o OBJECT
    ${STRATUM_SOURCE_DIR}/stratum/hal/lib/p4/p4_info_manager.cc
    ${STRATUM_SOURCE_DIR}/stratum/hal/lib/p4/p4_info_manager.h
    ${STRATUM_SOURCE_DIR}/stratum/hal/lib/p4/utils.cc
    ${STRATUM_SOURCE_DIR}/stratum/hal/lib/p4/utils.h
)

target_include_directories(stratum_hal_lib_p4_o PRIVATE ${STRATUM_INCLUDES})

add_dependencies(stratum_hal_lib_p4_o
    stratum_proto
    p4runtime_proto
)

#############################
# stratum_yang_parse_tree_o #
#############################

set(STRATUM_TDI_LIB_DIR  ${STRATUM_SOURCE_DIR}/stratum/hal/lib/tdi)
set(STRATUM_YANG_LIB_DIR ${STRATUM_SOURCE_DIR}/stratum/hal/lib/yang)

add_library(stratum_yang_parse_tree_o OBJECT
    ${STRATUM_YANG_LIB_DIR}/yang_parse_tree.cc
    ${STRATUM_YANG_LIB_DIR}/yang_parse_tree.h
    ${STRATUM_YANG_LIB_DIR}/yang_parse_tree_chassis.cc
    ${STRATUM_YANG_LIB_DIR}/yang_parse_tree_component.cc
    ${STRATUM_YANG_LIB_DIR}/yang_parse_tree_component.h
    ${STRATUM_YANG_LIB_DIR}/yang_parse_tree_helpers.cc
    ${STRATUM_YANG_LIB_DIR}/yang_parse_tree_helpers.h
    ${STRATUM_YANG_LIB_DIR}/yang_parse_tree_interface.cc
    ${STRATUM_YANG_LIB_DIR}/yang_parse_tree_interface.h
    ${STRATUM_YANG_LIB_DIR}/yang_parse_tree_ipsec.cc
    ${STRATUM_YANG_LIB_DIR}/yang_parse_tree_node.cc
    ${STRATUM_YANG_LIB_DIR}/yang_parse_tree_optical.cc
    ${STRATUM_YANG_LIB_DIR}/yang_parse_tree_paths.cc
    ${STRATUM_YANG_LIB_DIR}/yang_parse_tree_paths.h
    ${STRATUM_YANG_LIB_DIR}/yang_parse_tree_singleton.cc
    ${STRATUM_YANG_LIB_DIR}/yang_parse_tree_system.cc
)

if(DPDK_TARGET)
    target_sources(stratum_yang_parse_tree_o PRIVATE
        ${STRATUM_TDI_LIB_DIR}/dpdk/dpdk_add_subtree_interface.cc
        ${STRATUM_TDI_LIB_DIR}/dpdk/dpdk_parse_tree_interface.cc
        ${STRATUM_TDI_LIB_DIR}/dpdk/dpdk_parse_tree_interface.h
    )
else()
    target_sources(stratum_yang_parse_tree_o PRIVATE
        ${STRATUM_YANG_LIB_DIR}/yang_add_subtree_interface.cc
    )
endif()

target_include_directories(stratum_yang_parse_tree_o PRIVATE
    ${STRATUM_INCLUDES}
)

add_dependencies(stratum_yang_parse_tree_o
    gnmi_proto
    stratum_proto
)

########################
# stratum_tdi_common_o #
########################

add_library(stratum_tdi_common_o OBJECT
    ${STRATUM_SOURCE_DIR}/stratum/hal/lib/tdi/tdi_action_profile_manager.cc
    ${STRATUM_SOURCE_DIR}/stratum/hal/lib/tdi/tdi_action_profile_manager.h
    ${STRATUM_SOURCE_DIR}/stratum/hal/lib/tdi/tdi_counter_manager.cc
    ${STRATUM_SOURCE_DIR}/stratum/hal/lib/tdi/tdi_counter_manager.h
    ${STRATUM_SOURCE_DIR}/stratum/hal/lib/tdi/tdi_id_mapper.cc
    ${STRATUM_SOURCE_DIR}/stratum/hal/lib/tdi/tdi_id_mapper.h
    ${STRATUM_SOURCE_DIR}/stratum/hal/lib/tdi/tdi_lut_manager.cc
    ${STRATUM_SOURCE_DIR}/stratum/hal/lib/tdi/tdi_lut_manager.h
    ${STRATUM_SOURCE_DIR}/stratum/hal/lib/tdi/tdi_node.cc
    ${STRATUM_SOURCE_DIR}/stratum/hal/lib/tdi/tdi_node.h
    ${STRATUM_SOURCE_DIR}/stratum/hal/lib/tdi/tdi_packetio_manager.cc
    ${STRATUM_SOURCE_DIR}/stratum/hal/lib/tdi/tdi_packetio_manager.h
    ${STRATUM_SOURCE_DIR}/stratum/hal/lib/tdi/tdi_pipeline_utils.cc
    ${STRATUM_SOURCE_DIR}/stratum/hal/lib/tdi/tdi_pipeline_utils.h
    ${STRATUM_SOURCE_DIR}/stratum/hal/lib/tdi/tdi_port_manager.h
    ${STRATUM_SOURCE_DIR}/stratum/hal/lib/tdi/tdi_pre_manager.cc
    ${STRATUM_SOURCE_DIR}/stratum/hal/lib/tdi/tdi_pre_manager.h
    ${STRATUM_SOURCE_DIR}/stratum/hal/lib/tdi/tdi_sde_action_profile.cc
    ${STRATUM_SOURCE_DIR}/stratum/hal/lib/tdi/tdi_sde_clone_session.cc
    ${STRATUM_SOURCE_DIR}/stratum/hal/lib/tdi/tdi_sde_common.h
    ${STRATUM_SOURCE_DIR}/stratum/hal/lib/tdi/tdi_sde_counter.cc
    ${STRATUM_SOURCE_DIR}/stratum/hal/lib/tdi/tdi_sde_flags.cc
    ${STRATUM_SOURCE_DIR}/stratum/hal/lib/tdi/tdi_sde_flags.h
    ${STRATUM_SOURCE_DIR}/stratum/hal/lib/tdi/tdi_sde_helpers.cc
    ${STRATUM_SOURCE_DIR}/stratum/hal/lib/tdi/tdi_sde_helpers.h
    ${STRATUM_SOURCE_DIR}/stratum/hal/lib/tdi/tdi_sde_interface.h
    ${STRATUM_SOURCE_DIR}/stratum/hal/lib/tdi/tdi_sde_meter.cc
    ${STRATUM_SOURCE_DIR}/stratum/hal/lib/tdi/tdi_sde_multicast.cc
    ${STRATUM_SOURCE_DIR}/stratum/hal/lib/tdi/tdi_sde_register.cc
    ${STRATUM_SOURCE_DIR}/stratum/hal/lib/tdi/tdi_sde_table_data.cc
    ${STRATUM_SOURCE_DIR}/stratum/hal/lib/tdi/tdi_sde_table_entry.cc
    ${STRATUM_SOURCE_DIR}/stratum/hal/lib/tdi/tdi_sde_table_key.cc
    ${STRATUM_SOURCE_DIR}/stratum/hal/lib/tdi/tdi_sde_utils.h
    ${STRATUM_SOURCE_DIR}/stratum/hal/lib/tdi/tdi_sde_wrapper.cc
    ${STRATUM_SOURCE_DIR}/stratum/hal/lib/tdi/tdi_sde_wrapper.h
    ${STRATUM_SOURCE_DIR}/stratum/hal/lib/tdi/tdi_table_manager.cc
    ${STRATUM_SOURCE_DIR}/stratum/hal/lib/tdi/tdi_table_manager.h
    ${STRATUM_SOURCE_DIR}/stratum/hal/lib/tdi/utils.cc
    ${STRATUM_SOURCE_DIR}/stratum/hal/lib/tdi/utils.h
)

if(ES2K_TARGET)
    target_sources(stratum_tdi_common_o PRIVATE
        ${STRATUM_SOURCE_DIR}/stratum/hal/lib/tdi/tdi_fixed_function_manager.cc
        ${STRATUM_SOURCE_DIR}/stratum/hal/lib/tdi/tdi_fixed_function_manager.h
        ${STRATUM_SOURCE_DIR}/stratum/hal/lib/tdi/tdi_ipsec_manager.cc
        ${STRATUM_SOURCE_DIR}/stratum/hal/lib/tdi/tdi_ipsec_manager.h
    )
endif()

target_include_directories(stratum_tdi_common_o PRIVATE
    ${STRATUM_INCLUDES}
    ${SDE_INSTALL_DIR}/include
)

add_dependencies(stratum_tdi_common_o stratum_proto)

########################
# stratum_tdi_target_o #
########################

add_library(stratum_tdi_target_o OBJECT)

if(TOFINO_TARGET)
    # Tofino files
    target_sources(stratum_tdi_target_o PRIVATE
        ${STRATUM_TDI_LIB_DIR}/tofino/tofino_chassis_manager.cc
        ${STRATUM_TDI_LIB_DIR}/tofino/tofino_chassis_manager.h
        ${STRATUM_TDI_LIB_DIR}/tofino/tofino_hal.cc
        ${STRATUM_TDI_LIB_DIR}/tofino/tofino_hal.h
        ${STRATUM_TDI_LIB_DIR}/tofino/tofino_port_manager.cc
        ${STRATUM_TDI_LIB_DIR}/tofino/tofino_port_manager.h
        ${STRATUM_TDI_LIB_DIR}/tofino/tofino_sde_target.cc
        ${STRATUM_TDI_LIB_DIR}/tofino/tofino_sde_utils.cc
        ${STRATUM_TDI_LIB_DIR}/tofino/tofino_switch.cc
        ${STRATUM_TDI_LIB_DIR}/tofino/tofino_switch.h
    )
elseif(DPDK_TARGET)
    # DPDK files
    target_sources(stratum_tdi_target_o PRIVATE
        ${STRATUM_TDI_LIB_DIR}/dpdk/dpdk_chassis_manager.cc
        ${STRATUM_TDI_LIB_DIR}/dpdk/dpdk_chassis_manager.h
        ${STRATUM_TDI_LIB_DIR}/dpdk/dpdk_hal.cc
        ${STRATUM_TDI_LIB_DIR}/dpdk/dpdk_hal.h
        ${STRATUM_TDI_LIB_DIR}/dpdk/dpdk_port_config.cc
        ${STRATUM_TDI_LIB_DIR}/dpdk/dpdk_port_config.h
        ${STRATUM_TDI_LIB_DIR}/dpdk/dpdk_port_manager.cc
        ${STRATUM_TDI_LIB_DIR}/dpdk/dpdk_port_manager.h
        ${STRATUM_TDI_LIB_DIR}/dpdk/dpdk_sde_target.cc
        ${STRATUM_TDI_LIB_DIR}/dpdk/dpdk_sde_utils.cc
        ${STRATUM_TDI_LIB_DIR}/dpdk/dpdk_switch.cc
        ${STRATUM_TDI_LIB_DIR}/dpdk/dpdk_switch.h
    )
elseif(ES2K_TARGET)
    # ES2K files
    target_sources(stratum_tdi_target_o PRIVATE
        ${STRATUM_TDI_LIB_DIR}/es2k/es2k_chassis_manager.cc
        ${STRATUM_TDI_LIB_DIR}/es2k/es2k_chassis_manager.h
        ${STRATUM_TDI_LIB_DIR}/es2k/es2k_hal.cc
        ${STRATUM_TDI_LIB_DIR}/es2k/es2k_hal.h
        ${STRATUM_TDI_LIB_DIR}/es2k/es2k_node.cc
        ${STRATUM_TDI_LIB_DIR}/es2k/es2k_node.h
        ${STRATUM_TDI_LIB_DIR}/es2k/es2k_port_manager.cc
        ${STRATUM_TDI_LIB_DIR}/es2k/es2k_port_manager.h
        ${STRATUM_TDI_LIB_DIR}/es2k/es2k_sde_target.cc
        ${STRATUM_TDI_LIB_DIR}/es2k/es2k_sde_utils.cc
        ${STRATUM_TDI_LIB_DIR}/es2k/es2k_switch.cc
        ${STRATUM_TDI_LIB_DIR}/es2k/es2k_switch.h
    )
endif()

target_include_directories(stratum_tdi_target_o PRIVATE
    ${STRATUM_INCLUDES}
    ${SDE_INSTALL_DIR}/include
)

add_dependencies(stratum_tdi_target_o stratum_proto)

##################
# stratum_main_o #
##################

set(STRATUM_TDI_BIN_DIR ${STRATUM_SOURCE_DIR}/stratum/hal/bin/tdi)

add_library(stratum_main_o OBJECT)

if(TOFINO_TARGET)
    # Tofino files
    target_sources(stratum_main_o PRIVATE
        ${STRATUM_TDI_BIN_DIR}/tofino/tofino_main.cc
        ${STRATUM_TDI_BIN_DIR}/main.h
    )
elseif(DPDK_TARGET)
    # DPDK files
    target_sources(stratum_main_o PRIVATE
        ${STRATUM_TDI_BIN_DIR}/dpdk/dpdk_main.cc
        ${STRATUM_TDI_BIN_DIR}/main.h
    )
elseif(ES2K_TARGET)
    # ES2K files
    target_sources(stratum_main_o PRIVATE
        ${STRATUM_TDI_BIN_DIR}/es2k/es2k_main.cc
        ${STRATUM_TDI_BIN_DIR}/main.h
    )
endif()

target_include_directories(stratum_main_o PRIVATE
    ${STRATUM_INCLUDES}
    ${SDE_INSTALL_DIR}/include
)

add_dependencies(stratum_main_o stratum_proto gnmi_proto)

#######################
# add_stratum_objects #
#######################

function(add_stratum_objects _TGT)
    target_sources(${_TGT} PRIVATE
        $<TARGET_OBJECTS:stratum_glue_o>
        $<TARGET_OBJECTS:stratum_lib_o>
        $<TARGET_OBJECTS:stratum_hal_lib_common_o>
        $<TARGET_OBJECTS:stratum_hal_lib_phal_o>
        $<TARGET_OBJECTS:stratum_hal_lib_p4_o>
        $<TARGET_OBJECTS:stratum_yang_parse_tree_o>
        $<TARGET_OBJECTS:stratum_tdi_common_o>
        $<TARGET_OBJECTS:stratum_tdi_target_o>
        $<TARGET_OBJECTS:stratum_main_o>
    )
endfunction(add_stratum_objects)

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
endfunction()

# Google libraries
function(add_stratum_google_libs _TGT)
    target_link_libraries(${_TGT} PUBLIC
        glog
        gflags
        grpc
        protobuf
        grpc++
    )
endfunction()

# Protobuf libraries
function(add_stratum_proto_libs _TGT)
    target_link_libraries(${_TGT} PUBLIC
        openconfig_proto
        stratum_proto
        gnmi_proto
        grpc_proto
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
add_stratum_objects(stratum_static)
add_stratum_libraries(stratum_static)

#####################
# libstratum_shared #
#####################

add_library(stratum_shared SHARED)
add_stratum_objects(stratum_shared)
add_stratum_libraries(stratum_shared)

#######################
# configuration files #
#######################

add_subdirectory(config)
