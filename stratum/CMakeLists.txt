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

option(INSTALL-PROTO "Install .proto files" OFF)

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

##########################
# stratum_hal_lib_yang_o #
##########################

set(STRATUM_TDI_LIB_DIR  ${STRATUM_SOURCE_DIR}/stratum/hal/lib/tdi)
set(STRATUM_YANG_LIB_DIR ${STRATUM_SOURCE_DIR}/stratum/hal/lib/yang)

if(TOFINO_TARGET)
  set(STRATUM_TARGET_YANG_FILES
    ${STRATUM_YANG_LIB_DIR}/yang_add_subtree_interface.cc
  )
elseif(DPDK_TARGET)
  set(STRATUM_TARGET_YANG_FILES
    ${STRATUM_TDI_LIB_DIR}/dpdk/dpdk_add_subtree_interface.cc
    ${STRATUM_TDI_LIB_DIR}/dpdk/dpdk_parse_tree_interface.cc
    ${STRATUM_TDI_LIB_DIR}/dpdk/dpdk_parse_tree_interface.h
  )
elseif(ES2K_TARGET)
  set(STRATUM_TARGET_YANG_FILES
    ${STRATUM_YANG_LIB_DIR}/yang_add_subtree_interface.cc
  )
endif()

add_library(stratum_hal_lib_yang_o OBJECT
    ${STRATUM_TARGET_YANG_FILES}
    ${STRATUM_YANG_LIB_DIR}/yang_parse_tree.cc
    ${STRATUM_YANG_LIB_DIR}/yang_parse_tree.h
    ${STRATUM_YANG_LIB_DIR}/yang_parse_tree_chassis.cc
    ${STRATUM_YANG_LIB_DIR}/yang_parse_tree_component.cc
    ${STRATUM_YANG_LIB_DIR}/yang_parse_tree_component.h
    ${STRATUM_YANG_LIB_DIR}/yang_parse_tree_helpers.cc
    ${STRATUM_YANG_LIB_DIR}/yang_parse_tree_helpers.h
    ${STRATUM_YANG_LIB_DIR}/yang_parse_tree_interface.cc
    ${STRATUM_YANG_LIB_DIR}/yang_parse_tree_interface.h
    ${STRATUM_YANG_LIB_DIR}/yang_parse_tree_node.cc
    ${STRATUM_YANG_LIB_DIR}/yang_parse_tree_optical.cc
    ${STRATUM_YANG_LIB_DIR}/yang_parse_tree_paths.cc
    ${STRATUM_YANG_LIB_DIR}/yang_parse_tree_paths.h
    ${STRATUM_YANG_LIB_DIR}/yang_parse_tree_singleton.cc
    ${STRATUM_YANG_LIB_DIR}/yang_parse_tree_system.cc
)

target_include_directories(stratum_hal_lib_yang_o PRIVATE
    ${STRATUM_INCLUDES}
)

add_dependencies(stratum_hal_lib_yang_o
    gnmi_proto
    stratum_proto
)

####################
# stratum_target_o #
####################

if(TOFINO_TARGET)
    # Tofino files
    set(STRATUM_TARGET_FILES
        ${STRATUM_TDI_LIB_DIR}/tofino/tofino_chassis_manager.cc
        ${STRATUM_TDI_LIB_DIR}/tofino/tofino_chassis_manager.h
        ${STRATUM_TDI_LIB_DIR}/tofino/tofino_hal.cc
        ${STRATUM_TDI_LIB_DIR}/tofino/tofino_hal.h
        ${STRATUM_TDI_LIB_DIR}/tofino/tofino_sde_target.cc
        ${STRATUM_TDI_LIB_DIR}/tofino/tofino_sde_utils.cc
        ${STRATUM_TDI_LIB_DIR}/tofino/tofino_switch.cc
        ${STRATUM_TDI_LIB_DIR}/tofino/tofino_switch.h
    )
elseif(DPDK_TARGET)
    # DPDK files
    set(STRATUM_TARGET_FILES
        ${STRATUM_TDI_LIB_DIR}/dpdk/dpdk_chassis_manager.cc
        ${STRATUM_TDI_LIB_DIR}/dpdk/dpdk_chassis_manager.h
        ${STRATUM_TDI_LIB_DIR}/dpdk/dpdk_hal.cc
        ${STRATUM_TDI_LIB_DIR}/dpdk/dpdk_hal.h
        ${STRATUM_TDI_LIB_DIR}/dpdk/dpdk_port_config.cc
        ${STRATUM_TDI_LIB_DIR}/dpdk/dpdk_port_config.h
        ${STRATUM_TDI_LIB_DIR}/dpdk/dpdk_sde_target.cc
        ${STRATUM_TDI_LIB_DIR}/dpdk/dpdk_sde_utils.cc
        ${STRATUM_TDI_LIB_DIR}/dpdk/dpdk_switch.cc
        ${STRATUM_TDI_LIB_DIR}/dpdk/dpdk_switch.h
    )
elseif(ES2K_TARGET)
    # ES2K files
    set(STRATUM_TARGET_FILES
        ${STRATUM_TDI_LIB_DIR}/es2k/es2k_chassis_manager.cc
        ${STRATUM_TDI_LIB_DIR}/es2k/es2k_chassis_manager.h
        ${STRATUM_TDI_LIB_DIR}/es2k/es2k_hal.cc
        ${STRATUM_TDI_LIB_DIR}/es2k/es2k_hal.h
        ${STRATUM_TDI_LIB_DIR}/es2k/es2k_sde_target.cc
        ${STRATUM_TDI_LIB_DIR}/es2k/es2k_sde_utils.cc
        ${STRATUM_TDI_LIB_DIR}/es2k/es2k_switch.cc
        ${STRATUM_TDI_LIB_DIR}/es2k/es2k_switch.h
    )
endif()

add_library(stratum_target_o OBJECT
    ${STRATUM_TARGET_FILES}

    # TDI files
    ${STRATUM_SOURCE_DIR}/stratum/hal/lib/tdi/tdi_action_profile_manager.cc
    ${STRATUM_SOURCE_DIR}/stratum/hal/lib/tdi/tdi_action_profile_manager.h
    ${STRATUM_SOURCE_DIR}/stratum/hal/lib/tdi/tdi_counter_manager.cc
    ${STRATUM_SOURCE_DIR}/stratum/hal/lib/tdi/tdi_counter_manager.h
    ${STRATUM_SOURCE_DIR}/stratum/hal/lib/tdi/tdi_id_mapper.cc
    ${STRATUM_SOURCE_DIR}/stratum/hal/lib/tdi/tdi_id_mapper.h
    ${STRATUM_SOURCE_DIR}/stratum/hal/lib/tdi/tdi_node.cc
    ${STRATUM_SOURCE_DIR}/stratum/hal/lib/tdi/tdi_node.h
    ${STRATUM_SOURCE_DIR}/stratum/hal/lib/tdi/tdi_packetio_manager.cc
    ${STRATUM_SOURCE_DIR}/stratum/hal/lib/tdi/tdi_packetio_manager.h
    ${STRATUM_SOURCE_DIR}/stratum/hal/lib/tdi/tdi_pipeline_utils.cc
    ${STRATUM_SOURCE_DIR}/stratum/hal/lib/tdi/tdi_pipeline_utils.h
    ${STRATUM_SOURCE_DIR}/stratum/hal/lib/tdi/tdi_pre_manager.cc
    ${STRATUM_SOURCE_DIR}/stratum/hal/lib/tdi/tdi_pre_manager.h
    ${STRATUM_SOURCE_DIR}/stratum/hal/lib/tdi/tdi_sde_action_profile.cc
    ${STRATUM_SOURCE_DIR}/stratum/hal/lib/tdi/tdi_sde_clone_session.cc
    ${STRATUM_SOURCE_DIR}/stratum/hal/lib/tdi/tdi_sde_common.h
    ${STRATUM_SOURCE_DIR}/stratum/hal/lib/tdi/tdi_sde_counter.cc
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

target_include_directories(stratum_target_o PRIVATE
    ${STRATUM_INCLUDES}
    ${SDE_INSTALL_DIR}/include
)

add_dependencies(stratum_target_o stratum_proto)

##################
# stratum_main_o #
##################

set(STRATUM_TDI_BIN_DIR ${STRATUM_SOURCE_DIR}/stratum/hal/bin/tdi)

if(TOFINO_TARGET)
    # Tofino files
    set(STRATUM_MAIN_FILES
        ${STRATUM_TDI_BIN_DIR}/tofino/tofino_main.cc
        ${STRATUM_TDI_BIN_DIR}/tofino/tofino_main.h
    )
elseif(DPDK_TARGET)
    # DPDK files
    set(STRATUM_MAIN_FILES
        ${STRATUM_TDI_BIN_DIR}/dpdk/dpdk_main.cc
        ${STRATUM_TDI_BIN_DIR}/dpdk/dpdk_main.h
    )
elseif(ES2K_TARGET)
    # ES2K files
    set(STRATUM_MAIN_FILES
        ${STRATUM_TDI_BIN_DIR}/es2k/es2k_main.cc
        ${STRATUM_TDI_BIN_DIR}/es2k/es2k_main.h
    )
endif()

add_library(stratum_main_o OBJECT
    ${STRATUM_MAIN_FILES}
)

target_include_directories(stratum_main_o PRIVATE
    ${STRATUM_INCLUDES}
    ${SDE_INSTALL_DIR}/include
)

add_dependencies(stratum_main_o stratum_proto gnmi_proto)

#####################
# libstratum_static #
#####################

add_library(stratum_static STATIC
    $<TARGET_OBJECTS:stratum_glue_o>
    $<TARGET_OBJECTS:stratum_lib_o>
    $<TARGET_OBJECTS:stratum_hal_lib_common_o>
    $<TARGET_OBJECTS:stratum_hal_lib_phal_o>
    $<TARGET_OBJECTS:stratum_hal_lib_p4_o>
    $<TARGET_OBJECTS:stratum_hal_lib_yang_o>
    $<TARGET_OBJECTS:stratum_target_o>
    $<TARGET_OBJECTS:stratum_main_o>
)

target_link_libraries(stratum_static PUBLIC
    absl::strings absl::synchronization absl::graphcycles_internal
    absl::stacktrace absl::symbolize absl::malloc_internal
    absl::debugging_internal absl::demangle_internal absl::time
    absl::strings_internal absl::throw_delegate
    absl::base absl::spinlock_wait absl::int128 absl::raw_logging_internal
    absl::log_severity absl::civil_time absl::civil_time absl::time_zone
    absl::status absl::cord absl::exponential_biased absl::str_format_internal
    absl::hash absl::raw_hash_set absl::city absl::bad_optional_access
    absl::bad_variant_access
)

target_link_libraries(stratum_static PUBLIC glog gflags grpc protobuf grpc++)
target_link_libraries(stratum_static PUBLIC pthread)

target_link_libraries(stratum_static PUBLIC
    openconfig_proto
    stratum_proto
    gnmi_proto
    grpc_proto
    p4runtime_proto
)

target_link_libraries(stratum_static PUBLIC pthread)

# tdi
find_library(LIBTDI tdi ${SDE_INSTALL_DIR}/lib)
if(NOT LIBTDI)
    message(FATAL_ERROR "Cannot find library: tdi")
endif()

add_library(tdi SHARED IMPORTED)
set_property(TARGET tdi PROPERTY IMPORTED_LOCATION ${LIBTDI})

# tdi_json_parser
find_library(LIBTDI_JSON_PARSER tdi_json_parser ${SDE_INSTALL_DIR}/lib)
if(NOT LIBTDI_JSON_PARSER)
    message(FATAL_ERROR "Cannot find library: tdi_json_parser")
endif()

add_library(tdi_json_parser SHARED IMPORTED)
set_property(TARGET tdi_json_parser
             PROPERTY IMPORTED_LOCATION ${LIBTDI_JSON_PARSER})

# target_sys
find_library(LIBTARGET_SYS target_sys ${SDE_INSTALL_DIR}/lib)
if(NOT LIBTARGET_SYS)
    message(FATAL_ERROR "Cannot find library: target_sys")
endif()

add_library(target_sys SHARED IMPORTED)
set_property(TARGET target_sys PROPERTY IMPORTED_LOCATION ${LIBTARGET_SYS})

if(TOFINO_TARGET)
    # Tofino driver
    find_library(LIBDRIVER driver ${SDE_INSTALL_DIR}/lib)
    if(NOT LIBDRIVER)
        message(FATAL_ERROR "Cannot find library: driver")
    endif()

    add_library(driver SHARED IMPORTED)
    set_property(TARGET driver PROPERTY IMPORTED_LOCATION ${LIBDRIVER})

    target_link_libraries(stratum_static PUBLIC
        driver
        tdi
        tdi_json_parser
        target_sys
    )
elseif(DPDK_TARGET)
    # DPDK driver
    find_library(LIBDRIVER driver ${SDE_INSTALL_DIR}/lib)
    if(NOT LIBDRIVER)
        message(FATAL_ERROR "Cannot find library: driver")
    endif()

    add_library(driver SHARED IMPORTED)
    set_property(TARGET driver PROPERTY IMPORTED_LOCATION ${LIBDRIVER})

    # bf_switchd_lib
    find_library(LIBBF_SWITCHD bf_switchd_lib ${SDE_INSTALL_DIR}/lib)
    if(NOT LIBBF_SWITCHD)
        message(FATAL_ERROR "Cannot find library: bf_switchd_lib")
    endif()

    add_library(bf_switchd_lib SHARED IMPORTED)
    set_property(TARGET bf_switchd_lib
                 PROPERTY IMPORTED_LOCATION ${LIBBF_SWITCHD})

    # target_utils
    find_library(LIBTARGET_UTILS target_utils ${SDE_INSTALL_DIR}/lib)
    if(NOT LIBTARGET_UTILS)
        message(FATAL_ERROR "Cannot find library: target_utils")
    endif()

    add_library(target_utils SHARED IMPORTED)
    set_property(TARGET target_utils
                 PROPERTY IMPORTED_LOCATION ${LIBTARGET_UTILS})

    target_link_libraries(stratum_static PUBLIC
        driver
        bf_switchd_lib
        tdi
        tdi_json_parser
        target_utils
        target_sys
    )

    target_link_directories(stratum_static PUBLIC
        ${SDE_INSTALL}/lib
        ${DPDK_LIBRARY_DIRS})

    target_link_options(stratum_static PUBLIC
        ${DPDK_LD_FLAGS}
        ${DPDK_LDFLAGS_OTHER}
    )
elseif(ES2K_TARGET)
    include(es2k-driver.cmake)

    target_link_libraries(stratum_static PUBLIC ${ES2K_DRIVER_LIBS})
    target_link_directories(stratum_static PUBLIC ${ES2K_LINK_DIRS})
endif()
