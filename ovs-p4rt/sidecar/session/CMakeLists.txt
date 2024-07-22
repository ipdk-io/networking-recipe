# CMake build file for ovs-p4rt/sidecar/session
#
# Copyright 2022-2024 Intel Corporation
# SPDX-License-Identifier: Apache 2.0
#

#-----------------------------------------------------------------------
# ovsp4rt_session_o
#-----------------------------------------------------------------------
add_library(ovsp4rt_session_o OBJECT
  ovsp4rt_credentials.cc
  ovsp4rt_credentials.h
  ovsp4rt_session.cc
  ovsp4rt_session.h
)

target_include_directories(ovsp4rt_session_o PUBLIC
  ${SIDECAR_SOURCE_DIR}
  ${STRATUM_SOURCE_DIR}
)

target_link_libraries(ovsp4rt_session_o PUBLIC
    p4_role_config_proto
    p4runtime_proto
    stratum_utils
)
