# CMake build file for ovs-p4rt/sidecar/lib
#
# Copyright 2022-2024 Intel Corporation
# SPDX-License-Identifier: Apache 2.0
#

target_sources(ovs_sidecar_o PUBLIC
    ovsp4rt_credentials.cc
    ovsp4rt_credentials.h
    ovsp4rt_session.cc
    ovsp4rt_session.h
)
