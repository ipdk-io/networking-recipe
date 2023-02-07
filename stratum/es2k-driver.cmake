# Definitions for the ES2000 P4 driver package.
#
# Copyright 2022-2023 Intel Corporation
# SPDX-License-Identifier: Apache 2.0
#

# The definitions are encapsulated in a function to limit pollution of the
# global namespace.
function(define_es2k_driver _LIBS _DIRS)

    ###########
    # Targets #
    ###########

    # es2k::bf_switchd_lib
    find_library(LIBBF_SWITCHD bf_switchd_lib REQUIRED)
    add_library(es2k::bf_switchd_lib SHARED IMPORTED)
    set_property(TARGET es2k::bf_switchd_lib
                 PROPERTY IMPORTED_LOCATION ${LIBBF_SWITCHD})

    # es2k::cpf
    find_library(LIBCPF cpf REQUIRED)
    add_library(es2k::cpf SHARED IMPORTED)
    set_property(TARGET es2k::cpf PROPERTY IMPORTED_LOCATION ${LIBCPF})

    # es2k:cpf_pmd_infra
    find_library(LIBCPF_PMD_INFRA cpf_pmd_infra REQUIRED)
    add_library(es2k:cpf_pmd_infra SHARED IMPORTED)
    set_property(TARGET es2k:cpf_pmd_infra
                 PROPERTY IMPORTED_LOCATION ${LIBCPF_PMD_INFRA})

    # es2k::driver
    find_library(LIBDRIVER driver REQUIRED)
    add_library(es2k::driver SHARED IMPORTED)
    set_property(TARGET es2k::driver PROPERTY IMPORTED_LOCATION ${LIBDRIVER})

    # es2k::es2kcp
    if(CMAKE_CROSSCOMPILING)
        # TODO: Base selection on the platform type.
        find_library(LIBES2KCP acccp REQUIRED)
      else()
        find_library(LIBES2KCP xeoncp REQUIRED)
    endif()
    add_library(es2k::es2kcp SHARED IMPORTED)
    set_property(TARGET es2k::es2kcp PROPERTY IMPORTED_LOCATION ${LIBES2KCP})

    # es2k::rte_net_idpf
    find_library(LIBRTE_NET_IDPF rte_net_idpf REQUIRED)
    add_library(es2k::rte_net_idpf SHARED IMPORTED)
    set_property(TARGET es2k::rte_net_idpf
                 PROPERTY IMPORTED_LOCATION ${LIBRTE_NET_IDPF})

    # es2k::target_utils
    find_library(LIBTARGET_UTILS target_utils REQUIRED)
    add_library(es2k::target_utils SHARED IMPORTED)
    set_property(TARGET es2k::target_utils
                 PROPERTY IMPORTED_LOCATION ${LIBTARGET_UTILS})

    # es2k::vfio
    find_library(LIBVFIO vfio REQUIRED)
    add_library(es2k::vfio SHARED IMPORTED)
    set_property(TARGET es2k::vfio PROPERTY IMPORTED_LOCATION ${LIBVFIO})

    #############
    # Variables #
    #############

    set(${_LIBS}
        es2k::driver
        es2k::bf_switchd_lib
        es2k::tdi
        es2k::tdi_json_parser
        es2k::target_utils
        es2k::target_sys
        es2k::es2kcp
        es2k::vfio
        es2k::cpf
        es2k:cpf_pmd_infra
        es2k::rte_net_idpf
        PARENT_SCOPE
    )

    set(${_DIRS}
        ${SDE_INSTALL}/lib
        PARENT_SCOPE
    )
endfunction(define_es2k_driver)

define_es2k_driver(DRIVER_SDK_LIBS DRIVER_SDK_DIRS)
