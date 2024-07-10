# CMake build file for ovs-p4rt/sidecar/logging
#
# Copyright 2024 Intel Corporation
# SPDX-License-Identifier: Apache 2.0
#

#-----------------------------------------------------------------------
# ovsp4rt_logging_o
#-----------------------------------------------------------------------
add_library(ovsp4rt_logging_o OBJECT
  ovsp4rt_diag_detail.cc
  ovsp4rt_diag_detail.h
  ovsp4rt_logging.cc
  ovsp4rt_logging.h
  ovsp4rt_logutils.cc
  ovsp4rt_logutils.h
)

target_include_directories(ovsp4rt_logging_o PUBLIC
  ${SIDECAR_SOURCE_DIR}
)

#-----------------------------------------------------------------------
# libovsp4rt_logging.a
#-----------------------------------------------------------------------
add_library(ovsp4rt_logging STATIC
  $<TARGET_OBJECTS:ovsp4rt_logging_o>
)

#-----------------------------------------------------------------------
# ovsp4rt_logging_so
#-----------------------------------------------------------------------
add_library(ovsp4rt_logging_so SHARED EXCLUDE_FROM_ALL
  $<TARGET_OBJECTS:ovsp4rt_logging_o>
)

set_target_properties(ovsp4rt_logging_so PROPERTIES
  OUTPUT_NAME ovsp4rt_logging
)

#-----------------------------------------------------------------------
# ovsp4rt_logging_test
#-----------------------------------------------------------------------
add_executable(ovsp4rt_logging_test EXCLUDE_FROM_ALL
  ovsp4rt_logging_test.cc
)

target_include_directories(ovsp4rt_logging_test PUBLIC
  ${SIDECAR_SOURCE_DIR}
  ${OVSP4RT_INCLUDE_DIR}
)

target_link_libraries(ovsp4rt_logging_test PUBLIC
  ovsp4rt_logging_so
  sde::target_sys
)
