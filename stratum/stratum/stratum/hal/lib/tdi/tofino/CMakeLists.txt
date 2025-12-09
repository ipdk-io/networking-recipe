# Build file for //stratum/hal/lib/tdi/tofino
#
# Copyright 2022-2023 Intel Corporation
# SPDX-License-Identifier: Apache 2.0
#

########################
# stratum_tdi_target_o #
########################

target_sources(stratum_tdi_target_o PRIVATE
    tofino_chassis_manager.cc
    tofino_chassis_manager.h
    tofino_hal.cc
    tofino_hal.h
    tofino_port_manager.cc
    tofino_port_manager.h
    tofino_sde_utils.cc
    tofino_sde_wrapper.cc
    tofino_switch.cc
    tofino_switch.h
)
