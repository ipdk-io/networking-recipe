# CMake build file for switchapi/es2k/lnw_v3
#
# Copyright 2022-2024 Intel Corporation
# SPDX-License-Identifier: Apache 2.0

target_sources(switchapi_o PRIVATE
  lnw_ecmp_hash_table.h
  lnw_ecmp_nexthop_table.h
  lnw_nexthop_table.h
  switch_pd_lag.c
  switch_pd_lag.h
  switch_pd_routing.c
)
