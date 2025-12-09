# CMake build file for switchapi/dpdk
#
# Copyright 2022-2024 Intel Corporation
# SPDX-License-Identifier: Apache 2.0

target_sources(switchapi_o PRIVATE
  switch_config.c
  switch_device.c
  switch_fdb.c
  switch_handle.c
  switch_id.c
  switch_l3.c
  switch_neighbor.c
  switch_nhop.c
  switch_pd_fdb.c
  switch_pd_p4_name_mapping.h
  switch_pd_routing.c
  switch_pd_routing.h
  switch_pd_tunnel.c
  switch_pd_utils.c
  switch_pd_utils.h
  switch_rif.c
  switch_rmac.c
  switch_table.c
  switch_tunnel.c
  switch_vrf.c
  switchapi_utils.c
)

target_link_libraries(switchapi_o PRIVATE
  sde::tdi
  sde::tdi_json_parser
)
