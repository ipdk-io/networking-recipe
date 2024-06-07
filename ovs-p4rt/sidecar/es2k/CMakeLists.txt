# CMake build file for ovs-p4rt/es2k
#
# Copyright 2022-2024 Intel Corporation
# SPDX-License-Identifier: Apache 2.0

target_sources(ovs_sidecar_o PRIVATE
    ovsp4rt_es2k_private.cc
    ovsp4rt_es2k_private.h
    p4_name_mapping.h
)
