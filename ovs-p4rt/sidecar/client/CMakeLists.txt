# CMake build file for ovs-p4rt/sidecar/client
#
# Copyright 2024 Intel Corporation
# SPDX-License-Identifier: Apache 2.0
#

#-----------------------------------------------------------------------
# ovsp4rt_client_o
#-----------------------------------------------------------------------
add_library(ovsp4rt_client_o OBJECT
  ovsp4rt_client.cc
  ovsp4rt_client.h
  ovsp4rt_client_interface.h
)

if(BUILD_JOURNAL)
  target_sources(ovsp4rt_client_o PRIVATE
    ovsp4rt_journal_client.cc
    ovsp4rt_journal_client.h
  )
endif()

target_include_directories(ovsp4rt_client_o PUBLIC
  ${OVSP4RT_INCLUDE_DIR}
  ${SIDECAR_SOURCE_DIR}
  ${STRATUM_SOURCE_DIR}
)

target_link_libraries(ovsp4rt_client_o PUBLIC
  p4_role_config_proto
  p4runtime_proto
  stratum_utils
)

#-----------------------------------------------------------------------
# libovsp4rt_client.a
#-----------------------------------------------------------------------
add_library(ovsp4rt_client STATIC
  $<TARGET_OBJECTS:ovsp4rt_client_o>
)

#-----------------------------------------------------------------------
# ovsp4rt_client_so
#-----------------------------------------------------------------------
add_library(ovsp4rt_client_so SHARED EXCLUDE_FROM_ALL
  $<TARGET_OBJECTS:ovsp4rt_client_o>
)

set_target_properties(ovsp4rt_client_so PROPERTIES
  OUTPUT_NAME ovsp4rt_client
)
