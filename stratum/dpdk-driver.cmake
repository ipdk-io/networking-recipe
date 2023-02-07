# Definitions for the DPDK P4 driver package.
#
# Copyright 2022-2023 Intel Corporation
# SPDX-License-Identifier: Apache 2.0
#

# The definitions are encapsulated in a function to limit pollution of the
# global namespace.
function(define_dpdk_driver _LIBS _DIRS _OPTS)

    ###########
    # Targets #
    ###########

    # bf_switchd_lib
    find_library(LIBBF_SWITCHD bf_switchd_lib REQUIRED)
    add_library(bf_switchd_lib SHARED IMPORTED)
    set_property(TARGET bf_switchd_lib
                 PROPERTY IMPORTED_LOCATION ${LIBBF_SWITCHD})

    # driver
    find_library(LIBDRIVER driver REQUIRED)
    add_library(driver SHARED IMPORTED)
    set_property(TARGET driver PROPERTY IMPORTED_LOCATION ${LIBDRIVER})

    # target_sys
    find_library(LIBTARGET_SYS target_sys REQUIRED)
    add_library(target_sys SHARED IMPORTED)
    set_property(TARGET target_sys PROPERTY IMPORTED_LOCATION ${LIBTARGET_SYS})

    # target_utils
    find_library(LIBTARGET_UTILS target_utils)
    add_library(target_utils SHARED IMPORTED)
    set_property(TARGET target_utils
                 PROPERTY IMPORTED_LOCATION ${LIBTARGET_UTILS})

    # tdi
    find_library(LIBTDI tdi REQUIRED)
    add_library(tdi SHARED IMPORTED)
    set_property(TARGET tdi PROPERTY IMPORTED_LOCATION ${LIBTDI})

    # tdi_json_parser
    find_library(LIBTDI_JSON_PARSER tdi_json_parser REQUIRED)
    add_library(tdi_json_parser SHARED IMPORTED)
    set_property(TARGET tdi_json_parser
                 PROPERTY IMPORTED_LOCATION ${LIBTDI_JSON_PARSER})

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
        PARENT_SCOPE
    )

    set(${_DIRS}
        ${SDE_INSTALL_DIR}/lib
        ${DPDK_LIBRARY_DIRS}
        PARENT_SCOPE
    )

    set(${_OPTS}
        ${DPDK_LD_FLAGS}
        ${DPDK_LDFLAGS_OTHER}
        PARENT_SCOPE
    )

endfunction(define_dpdk_driver)

define_dpdk_driver(DRIVER_SDK_LIBS DRIVER_SDK_DIRS DPDK_DRIVER_OPTS)

