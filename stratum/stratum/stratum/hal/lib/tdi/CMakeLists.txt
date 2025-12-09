# Build file for //stratum/hal/lib/tdi
#
# Copyright 2022-2024 Intel Corporation
# SPDX-License-Identifier: Apache 2.0
#

########################
# stratum_tdi_common_o #
########################

add_library(stratum_tdi_common_o OBJECT
    tdi_action_profile_manager.cc
    tdi_action_profile_manager.h
    tdi_status.h
    tdi_counter_manager.cc
    tdi_counter_manager.h
    tdi_global_vars.cc
    tdi_global_vars.h
    tdi_id_mapper.cc
    tdi_id_mapper.h
    tdi_node.cc
    tdi_node.h
    tdi_packetio_manager.cc
    tdi_packetio_manager.h
    tdi_pipeline_utils.cc
    tdi_pipeline_utils.h
    tdi_port_manager.cc
    tdi_port_manager.h
    tdi_pre_manager.cc
    tdi_pre_manager.h
    tdi_sde_action_profile.cc
    tdi_sde_clone_session.cc
    tdi_sde_common.h
    tdi_sde_counter.cc
    tdi_sde_flags.cc
    tdi_sde_flags.h
    tdi_sde_helpers.cc
    tdi_sde_helpers.h
    tdi_sde_interface.h
    tdi_sde_meter.cc
    tdi_sde_multicast.cc
    tdi_sde_register.cc
    tdi_sde_table_data.cc
    tdi_sde_table_entry.cc
    tdi_sde_table_key.cc
    tdi_sde_utils.h
    tdi_sde_wrapper.cc
    tdi_sde_wrapper.h
    tdi_table_manager.cc
    tdi_table_manager.h
    utils.cc
    utils.h
)

if(ES2K_TARGET)
    target_sources(stratum_tdi_common_o PRIVATE
        tdi_fixed_function_manager.cc
        tdi_fixed_function_manager.h
        tdi_ipsec_manager.cc
        tdi_ipsec_manager.h
    )
endif()

set_source_files_properties(
    tdi_pre_manager.cc
    PROPERTIES COMPILE_OPTIONS "-Wno-deprecated-declarations"
)

if(ES2K_TARGET)
  # For tdi_fixed_function_manager.
  target_link_libraries(stratum_tdi_common_o PUBLIC OpenSSL::Crypto)
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

if(DPDK_TARGET)
    add_subdirectory(dpdk)
elseif(ES2K_TARGET)
    add_subdirectory(es2k)
elseif(TOFINO_TARGET)
    add_subdirectory(tofino)
endif()

target_include_directories(stratum_tdi_target_o PRIVATE
    ${STRATUM_INCLUDES}
    ${SDE_INSTALL_DIR}/include
)

add_dependencies(stratum_tdi_target_o stratum_proto)
