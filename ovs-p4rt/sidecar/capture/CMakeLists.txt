# CMake build file for ovs-p4rt/sidecar/capture
#
# Copyright 2024 Intel Corporation
# SPDX-License-Identifier: Apache 2.0
#

add_library(ovsp4rt_capture STATIC
  capture_mac_learning_info.cc
  ovsp4rt_capture.h
)

target_include_directories(ovsp4rt_capture PUBLIC
  ${DEPEND_INSTALL_DIR}/include
  ${OVSP4RT_INCLUDE_DIR}
  ${SIDECAR_SOURCE_DIR}
)
