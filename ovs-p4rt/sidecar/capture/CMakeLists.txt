# CMake build file for ovs-p4rt/sidecar/capture
#
# Copyright 2024 Intel Corporation
# SPDX-License-Identifier: Apache 2.0
#

add_library(ovsp4rt_capture STATIC
  capture_api_input.cc
  capture_api_input.h
  capture_mac_learning_info.cc
  capture_src_port_info.cc
  ovsp4rt_capture.h
  ovsp4rt_encode.h
)

target_include_directories(ovsp4rt_capture PUBLIC
  ${DEPEND_INSTALL_DIR}/include
  ${OVSP4RT_INCLUDE_DIR}
  ${SIDECAR_SOURCE_DIR}
)
