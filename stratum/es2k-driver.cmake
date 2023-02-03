# Definitions for the ES2000 P4 driver.
#
# Copyright 2022-2023 Intel Corporation
# SPDX-License-Identifier: Apache 2.0
#

##########
# driver #
##########

find_library(LIBDRIVER driver REQUIRED)

add_library(driver SHARED IMPORTED)
set_property(TARGET driver PROPERTY IMPORTED_LOCATION ${LIBDRIVER})

##################
# bf_switchd_lib #
##################

# bf_switchd_lib
find_library(LIBBF_SWITCHD bf_switchd_lib REQUIRED)

add_library(bf_switchd_lib SHARED IMPORTED)
set_property(TARGET bf_switchd_lib
             PROPERTY IMPORTED_LOCATION ${LIBBF_SWITCHD})

################
# target-utils #
################

# target_utils
find_library(LIBTARGET_UTILS target_utils REQUIRED)

add_library(target_utils SHARED IMPORTED)
set_property(TARGET target_utils
             PROPERTY IMPORTED_LOCATION ${LIBTARGET_UTILS})

##########
# es2kcp #
##########

# TODO: Base this on the platform type.
if(CMAKE_CROSSCOMPILING)
    find_library(LIBES2KCP acccp REQUIRED)
else()
    find_library(LIBES2KCP xeoncp REQUIRED)
endif()

add_library(es2kcp SHARED IMPORTED)
set_property(TARGET es2kcp PROPERTY IMPORTED_LOCATION ${LIBES2KCP})

########
# vfio #
########

# vfio
find_library(LIBVFIO vfio REQUIRED)

add_library(vfio SHARED IMPORTED)
set_property(TARGET vfio PROPERTY IMPORTED_LOCATION ${LIBVFIO})

#######
# cpf #
#######

find_library(LIBCPF cpf REQUIRED)

add_library(cpf SHARED IMPORTED)
set_property(TARGET cpf PROPERTY IMPORTED_LOCATION ${LIBCPF})

#################
# cpf_pmd_infra #
#################

find_library(LIBCPF_PMD_INFRA cpf_pmd_infra REQUIRED)

add_library(cpf_pmd_infra SHARED IMPORTED)
set_property(TARGET cpf_pmd_infra
             PROPERTY IMPORTED_LOCATION ${LIBCPF_PMD_INFRA})

################
# rte_net_idpf #
################

find_library(LIBRTE_NET_IDPF rte_net_idpf REQUIRED)

add_library(rte_net_idpf SHARED IMPORTED)
set_property(TARGET rte_net_idpf PROPERTY IMPORTED_LOCATION ${LIBRTE_NET_IDPF})

####################
# ES2K_DRIVER_LIBS #
####################

set(ES2K_DRIVER_LIBS
    driver
    bf_switchd_lib
    tdi
    tdi_json_parser
    target_utils
    target_sys
    es2kcp
    vfio
    cpf
    cpf_pmd_infra
    rte_net_idpf
)

set(ES2K_LINK_DIRS ${SDE_INSTALL}/lib)
