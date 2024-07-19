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

install(
    TARGETS ovsp4rt_spies
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

set(ovsp4rt_spies_cfg_file ${CMAKE_CURRENT_BINARY_DIR}/libovsp4rt_spies.pc)
configure_file(libovsp4rt_spies.pc.in ${ovsp4rt_spies_cfg_file} @ONLY)

install(
    FILES ${ovsp4rt_spies_cfg_file}
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig
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
