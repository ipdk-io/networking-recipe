# Copyright 2024 Intel Corporation
# SPDX-License-Identifier: Apache 2.0

install(
  FILES
    CMakeUserPresets.json
    dpdk.cmake
    es2k.cmake
    README.md
  DESTINATION
    ${CMAKE_INSTALL_PREFIX}/share/p4cp/cmake
)
