# CMake build file for ovs-p4rt/sidecar/common
#
# Copyright 2022-2024 Intel Corporation
# SPDX-License-Identifier: Apache 2.0
#

target_sources(ovs_sidecar_o PUBLIC
    ovsp4rt_logutils.cc
    ovsp4rt_logutils.h
    ovsp4rt_private.cc
    ovsp4rt_private.h
    ovsp4rt_public.cc
    ovsp4rt_utils.cc
    ovsp4rt_utils.h
)
