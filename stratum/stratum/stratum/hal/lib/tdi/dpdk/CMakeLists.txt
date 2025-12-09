# Build file for //stratum/hal/lib/tdi/dpdk
#
# Copyright 2022-2023 Intel Corporation
# SPDX-License-Identifier: Apache 2.0
#

########################
# stratum_tdi_target_o #
########################

target_sources(stratum_tdi_target_o PRIVATE
    dpdk_chassis_manager.cc
    dpdk_chassis_manager.h
    dpdk_hal.cc
    dpdk_hal.h
    dpdk_port_config.cc
    dpdk_port_config.h
    dpdk_port_manager.cc
    dpdk_port_manager.h
    dpdk_sde_utils.cc
    dpdk_sde_wrapper.cc
    dpdk_switch.cc
    dpdk_switch.h
)

#############################
# stratum_yang_parse_tree_o #
#############################

target_sources(stratum_yang_parse_tree_o PRIVATE
    dpdk_add_subtree_interface.cc
    dpdk_parse_tree_interface.cc
    dpdk_parse_tree_interface.h
)

