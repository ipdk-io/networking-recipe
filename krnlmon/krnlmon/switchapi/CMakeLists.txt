# CMake build file for switchapi
#
# Copyright 2022-2024 Intel Corporation
# SPDX-License-Identifier: Apache 2.0

add_library(switchapi_o OBJECT
  switch_base_types.h
  switch_config.h
  switch_config_int.h
  switch_device.h
  switch_device_int.h
  switch_fdb.h
  switch_handle.h
  switch_handle_int.h
  switch_id.h
  switch_interface.h
  switch_internal.h
  switch_l3.h
  switch_l3_int.h
  switch_lag.h
  switch_neighbor.h
  switch_neighbor_int.h
  switch_pd_routing.h
  switch_nhop.h
  switch_nhop_int.h
  switch_port_int.h
  switch_rif.h
  switch_rif_int.h
  switch_rmac.h
  switch_rmac_int.h
  switch_status.h
  switch_table.h
  switch_tunnel.h
  switch_types_int.h
  switch_vrf.h
)

if(ES2K_TARGET)
  add_subdirectory(es2k)
elseif(DPDK_TARGET)
  add_subdirectory(dpdk)
endif()

target_include_directories(switchapi_o PRIVATE
    ${SDE_INSTALL_DIR}/include
    ${SDE_INSTALL_DIR}/include/target-utils # id
    ${SDE_INSTALL_DIR}/include/target-utils/third-party
    ${SDE_INSTALL_DIR}/include/target-utils/third-party/tommyds
    ${SDE_INSTALL_DIR}/include/target-sys
)
