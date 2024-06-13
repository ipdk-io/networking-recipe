# CMake build file for ovs-p4rt/sidecar/encode
#
# Copyright 2024 Intel Corporation
# SPDX-License-Identifier: Apache 2.0
#

if(ES2K_TARGET)
  target_sources(ovs_sidecar_o PRIVATE
    PrepareL2ToTunnel.cc
    PrepareL2ToTunnel.h
  )
endif()
