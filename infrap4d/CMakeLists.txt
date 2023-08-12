# Builds infrap4d daemon
#
# Copyright 2022-2023 Intel Corporation
# SPDX-License-Identifier: Apache 2.0
#

add_subdirectory(daemon)

set(STRATUM_TDI_BIN_DIR ${STRATUM_SOURCE_DIR}/stratum/hal/bin/tdi)

if(WITH_KRNLMON)
    add_executable(infrap4d infrap4d_main.cc)
    target_sources(infrap4d PRIVATE $<TARGET_OBJECTS:daemon_o>)
    target_include_directories(infrap4d PRIVATE ${KRNLMON_SOURCE_DIR})
elseif(DPDK_TARGET)
    add_executable(infrap4d ${STRATUM_TDI_BIN_DIR}/dpdk/main.cc)
elseif(ES2K_TARGET)
    add_executable(infrap4d ${STRATUM_TDI_BIN_DIR}/es2k/main.cc)
elseif(TOFINO_TARGET)
    add_executable(infrap4d ${STRATUM_TDI_BIN_DIR}/tofino/main.cc)
endif()

if(NOT TOFINO_TARGET)
    target_sources(infrap4d PRIVATE ${STRATUM_TDI_BIN_DIR}/main.h)
endif()

target_include_directories(infrap4d PRIVATE ${STRATUM_SOURCE_DIR})

target_link_libraries(infrap4d PRIVATE
    -Wl,--whole-archive
    stratum_static
    -Wl,--no-whole-archive
)

if(DPDK_TARGET)
    add_dpdk_target_libraries(infrap4d)
elseif(ES2K_TARGET)
    add_es2k_target_libraries(infrap4d)
elseif(TOFINO_TARGET)
    add_tofino_target_libraries(infrap4d)
endif()

if(WITH_KRNLMON)
    target_link_libraries(infrap4d PRIVATE krnlmon pthread)
endif()

set_install_rpath(infrap4d ${EXEC_ELEMENT} ${SDE_ELEMENT} ${DEP_ELEMENT})

install(TARGETS infrap4d DESTINATION sbin)
