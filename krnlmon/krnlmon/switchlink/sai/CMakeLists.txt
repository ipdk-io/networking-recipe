# CMake build file for switchlink/sai
#
# Copyright 2022-2024 Intel Corporation
# SPDX-License-Identifier: Apache 2.0

add_library(switchlink_sai_o OBJECT
   switchlink_handle_ecmp.c
   switchlink_handle_link.c
   switchlink_handle_neigh.c
   switchlink_handle_nexthop.c
   switchlink_handle_route.c
   switchlink_handle_tunnel.c
   switchlink_init_sai.c
   switchlink_init_sai.h
)

if(ES2K_TARGET)
  target_sources(switchlink_sai_o PRIVATE switchlink_handle_lag.c)
endif()

target_include_directories(switchlink_sai_o PRIVATE
    ${SDE_INSTALL_DIR}/include
    ${SDE_INSTALL_DIR}/include/target-utils/third-party  # judy
    ${SDE_INSTALL_DIR}/include/target-utils/third-party/tommyds
    ${SDE_INSTALL_DIR}/include/target-utils/third-party/xxHash
    ${SDE_INSTALL_DIR}/include/target-sys
    ${SAI_INCLUDE_DIR}
)


