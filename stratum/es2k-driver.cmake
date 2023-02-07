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

    # bf_switchd_lib
    find_library(LIBBF_SWITCHD bf_switchd_lib REQUIRED)
    add_library(bf_switchd_lib SHARED IMPORTED)
    set_property(TARGET bf_switchd_lib
                 PROPERTY IMPORTED_LOCATION ${LIBBF_SWITCHD})

    # cpf
    find_library(LIBCPF cpf REQUIRED)
    add_library(cpf SHARED IMPORTED)
    set_property(TARGET cpf PROPERTY IMPORTED_LOCATION ${LIBCPF})

    # es2k:cpf_pmd_infra
    find_library(LIBCPF_PMD_INFRA cpf_pmd_infra REQUIRED)
    add_library(es2k:cpf_pmd_infra SHARED IMPORTED)
    set_property(TARGET es2k:cpf_pmd_infra
                 PROPERTY IMPORTED_LOCATION ${LIBCPF_PMD_INFRA})

    # driver
    find_library(LIBDRIVER driver REQUIRED)
    add_library(driver SHARED IMPORTED)
    set_property(TARGET driver PROPERTY IMPORTED_LOCATION ${LIBDRIVER})

    # es2kcp
    if(CMAKE_CROSSCOMPILING)
        # TODO: Base selection on the platform type.
        find_library(LIBES2KCP acccp REQUIRED)
      else()
        find_library(LIBES2KCP xeoncp REQUIRED)
    endif()
    add_library(es2kcp SHARED IMPORTED)
    set_property(TARGET es2kcp PROPERTY IMPORTED_LOCATION ${LIBES2KCP})

    # rte_net_idpf
    find_library(LIBRTE_NET_IDPF rte_net_idpf REQUIRED)
    add_library(rte_net_idpf SHARED IMPORTED)
    set_property(TARGET rte_net_idpf
                 PROPERTY IMPORTED_LOCATION ${LIBRTE_NET_IDPF})

    # target_utils
    find_library(LIBTARGET_UTILS target_utils REQUIRED)
    add_library(target_utils SHARED IMPORTED)
    set_property(TARGET target_utils
                 PROPERTY IMPORTED_LOCATION ${LIBTARGET_UTILS})

    # vfio
    find_library(LIBVFIO vfio REQUIRED)
    add_library(vfio SHARED IMPORTED)
    set_property(TARGET vfio PROPERTY IMPORTED_LOCATION ${LIBVFIO})

    #############
    # Variables #
    #############

    set(${_LIBS}
        driver
        bf_switchd_lib
        tdi
        tdi_json_parser
        target_utils
        target_sys
        es2kcp
        vfio
        cpf
        es2k:cpf_pmd_infra
        rte_net_idpf
        PARENT_SCOPE
    )

    set(${_DIRS}
        ${SDE_INSTALL}/lib
        PARENT_SCOPE
    )
endfunction(define_es2k_driver)

define_es2k_driver(DRIVER_SDK_LIBS DRIVER_SDK_DIRS)
