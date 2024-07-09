# CMake build file for ovs-p4rt/sidecar/tracker
#
# Copyright 2024 Intel Corporation
# SPDX-License-Identifier: Apache 2.0
#

#-----------------------------------------------------------------------
# ovsp4rt_tracker_o
#-----------------------------------------------------------------------
add_library(ovsp4rt_tracker_o OBJECT
  ovsp4rt_encode.cc
  ovsp4rt_encode.h
  ovsp4rt_tracker.cc
  ovsp4rt_tracker.h
)

target_include_directories(ovsp4rt_tracker_o PUBLIC
  ${DEPEND_INSTALL_DIR}/include
  ${OVSP4RT_INCLUDE_DIR}
)

target_link_libraries(ovsp4rt_tracker_o PUBLIC
  p4runtime_proto
)
