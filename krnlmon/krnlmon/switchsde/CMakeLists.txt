# CMake build file for switchsde
#
# Copyright 2024 Intel Corporation
# SPDX-License-Identifier: Apache 2.0

###############
# switchsde_o #
###############

add_library(switchsde_o OBJECT
  ${SDE_PORT_INTF}
  sde_port_intf.h
  sde_status.c
  sde_status.h
  sde_types.h
)

if(DPDK_TARGET)
  target_sources(switchsde_o PRIVATE sde_dpdk_port_intf.c)
elseif(ES2K_TARGET)
  target_sources(switchsde_o PRIVATE sde_es2k_port_intf.c)
endif()

target_include_directories(switchsde_o PUBLIC ${SDE_INSTALL_DIR}/include)

if(DPDK_TARGET)
#######################
# switchsde_dpdk_test #
#######################

add_executable(switchsde_dpdk_test
  switchsde_dpdk_test.cc
  sde_status.h
  sde_types.h
)

target_include_directories(switchsde_dpdk_test PRIVATE
  ${SDE_INSTALL_DIR}/include
)

define_unit_test(switchsde_dpdk_test)
endif(DPDK_TARGET)

if(ES2K_TARGET)
#######################
# switchsde_es2k_test #
#######################

add_executable(switchsde_es2k_test
  switchsde_es2k_test.cc
  sde_status.h
  sde_types.h
)

target_include_directories(switchsde_es2k_test PRIVATE
  ${SDE_INSTALL_DIR}/include
)

define_unit_test(switchsde_es2k_test)
endif(ES2K_TARGET)
