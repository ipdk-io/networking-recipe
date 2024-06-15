# CMake build file for ovs-p4rt/sidecar/lib
#
# Copyright 2024 Intel Corporation
# SPDX-License-Identifier: Apache 2.0
#

add_library(ovsp4rt_logging_o OBJECT
  ovsp4rt_logging.cc
  ovsp4rt_logging.h
  ovsp4rt_logutils.cc
  ovsp4rt_logutils.h
)

target_include_directories(ovsp4rt_logging_o PUBLIC
  ${SIDECAR_SOURCE_DIR}
)

########################
# libovsp4rt_logging.a #
########################

add_library(ovsp4rt_logging STATIC
  $<TARGET_OBJECTS:ovsp4rt_logging_o>
)

target_link_libraries(ovsp4rt_logging PUBLIC
  absl::strings
)

#########################
# libovsp4rt_logging.so #
#########################

add_library(ovsp4rt_logging_so SHARED EXCLUDE_FROM_ALL
  $<TARGET_OBJECTS:ovsp4rt_logging_o>
)

target_link_libraries(ovsp4rt_logging_so PUBLIC
  absl::strings
)

set_target_properties(ovsp4rt_logging_so PROPERTIES
  OUTPUT_NAME ovsp4rt_logging
)
