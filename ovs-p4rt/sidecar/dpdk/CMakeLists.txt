# CMake build file for ovs-p4rt/dpdk
#
# Copyright 2022-2024 Intel Corporation
# SPDX-License-Identifier: Apache 2.0

target_sources(ovs_sidecar_o PRIVATE
    ovsp4rt_dpdk_defs.h
    ovsp4rt_dpdk_private.cc
    ovsp4rt_dpdk_private.h
    ovsp4rt_dpdk_public.cc
)
