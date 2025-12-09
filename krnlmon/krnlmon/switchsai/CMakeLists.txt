# CMake build file for switchsai
#
# Copyright 2022-2024 Intel Corporation
# SPDX-License-Identifier: Apache 2.0

add_library(switchsai_o OBJECT
   sai.c
   saifdb.c
   saiinternal.h
   saineighbor.c
   sainexthop.c
   sainexthopgroup.c
   sairoute.c
   sairouterinterface.c
   saitunnel.c
   saiutils.c
   saivirtualrouter.c
)

if(ES2K_TARGET)
  target_sources(switchsai_o PRIVATE sailag.c)
endif()

target_include_directories(switchsai_o PRIVATE
    ${SAI_INCLUDE_DIR}
    ${SDE_INSTALL_DIR}/include
    ${SDE_INSTALL_DIR}/include/target-utils # id
    ${SDE_INSTALL_DIR}/include/target-utils/third-party
    ${SDE_INSTALL_DIR}/include/target-utils/third-party/tommyds
    ${SDE_INSTALL_DIR}/include/target-sys # bf_sal
)
