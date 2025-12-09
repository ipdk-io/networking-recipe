# Build file for //stratum/hal/lib/tdi/es2k
#
# Copyright 2022-2023,2025 Intel Corporation
# SPDX-License-Identifier: Apache 2.0
#

########################
# stratum_tdi_target_o #
########################

target_sources(stratum_tdi_target_o PRIVATE
    es2k_chassis_manager.cc
    es2k_chassis_manager.h
    es2k_hal.cc
    es2k_hal.h
    es2k_node.cc
    es2k_node.h
    es2k_port_manager.cc
    es2k_port_manager.h
    es2k_sde_meter.cc
    es2k_sde_packetio.cc
    es2k_sde_table_data.cc
    es2k_sde_utils.cc
    es2k_sde_wrapper.cc
    es2k_switch.cc
    es2k_switch.h
    es2k_virtual_port_manager.cc
    es2k_virtual_port_manager.h
)
