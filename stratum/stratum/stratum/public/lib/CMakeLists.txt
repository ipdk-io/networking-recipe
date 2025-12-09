# Build file for //stratum/public/lib
#
# Copyright 2022-2024 Intel Corporation
# SPDX-License-Identifier: Apache 2.0
#

##############################
# Build stratum_public_lib_o #
##############################

add_library(stratum_public_lib_o OBJECT
    error.cc
    error.h
)

target_include_directories(stratum_public_lib_o PUBLIC ${STRATUM_INCLUDES})

add_dependencies(stratum_public_lib_o error_proto_o)
