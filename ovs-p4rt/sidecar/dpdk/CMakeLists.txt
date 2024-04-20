# CMake build file for sidecar/dpdk
#
# Copyright 2022-2024 Intel Corporation
# SPDX-License-Identifier: Apache 2.0

target_sources(ovs_sidecar_o PRIVATE
    ovsp4rt_dpdk_private.cc
    ovsp4rt_dpdk_public.cc
    p4_name_mapping.h
)
