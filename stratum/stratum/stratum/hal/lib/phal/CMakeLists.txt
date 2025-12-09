# Build file for //stratum/hal/lib/phal
#
# Copyright 2022-2023 Intel Corporation
# SPDX-License-Identifier: Apache 2.0
#

##########################
# stratum_hal_lib_phal_o #
##########################

add_library(stratum_hal_lib_phal_o OBJECT
    phal_sim.cc
    phal_sim.h
)

target_include_directories(stratum_hal_lib_phal_o PRIVATE ${STRATUM_INCLUDES})

add_dependencies(stratum_hal_lib_phal_o stratum_proto)
