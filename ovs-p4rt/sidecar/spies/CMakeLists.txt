# CMake build file for ovs-p4rt/sidecar/spies
#
# Copyright 2024 Intel Corporation
# SPDX-License-Identifier: Apache 2.0
#

#-----------------------------------------------------------------------
# libovsp4rt_spies.a
#-----------------------------------------------------------------------
add_library(ovsp4rt_spies STATIC
    ovsp4rt_config_fdb_entry_spy.cc
    ovsp4rt_spies.cc
)

target_include_directories(ovsp4rt_spies PUBLIC
    ${DEPEND_INSTALL_DIR}/include
    ${OVSP4RT_INCLUDE_DIR}
)

#-----------------------------------------------------------------------
# ovsp4rt_config_fdb_entry_spy_test
#-----------------------------------------------------------------------
find_package(nlohmann_json REQUIRED)

add_executable(ovsp4rt_config_fdb_entry_spy_test
  ovsp4rt_config_fdb_entry_spy_test.cc
)

target_include_directories(ovsp4rt_config_fdb_entry_spy_test PRIVATE
  ${OVSP4RT_INCLUDE_DIR}
)

target_link_libraries(ovsp4rt_config_fdb_entry_spy_test PRIVATE
  nlohmann_json::nlohmann_json
  ovsp4rt_spies
)
