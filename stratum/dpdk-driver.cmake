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

    # dpdk::bf_switchd_lib
    find_library(LIBBF_SWITCHD bf_switchd_lib REQUIRED)
    add_library(dpdk::bf_switchd_lib SHARED IMPORTED)
    set_property(TARGET dpdk::bf_switchd_lib
                 PROPERTY IMPORTED_LOCATION ${LIBBF_SWITCHD})

    # dpdk::driver
    find_library(LIBDRIVER driver REQUIRED)
    add_library(dpdk::driver SHARED IMPORTED)
    set_property(TARGET dpdk::driver PROPERTY IMPORTED_LOCATION ${LIBDRIVER})

    # dpdk::target_sys
    find_library(LIBTARGET_SYS target_sys REQUIRED)
    add_library(dpdk::target_sys SHARED IMPORTED)
    set_property(TARGET dpdk::target_sys PROPERTY IMPORTED_LOCATION ${LIBTARGET_SYS})

    # dpdk::target_utils
    find_library(LIBTARGET_UTILS target_utils)
    add_library(dpdk::target_utils SHARED IMPORTED)
    set_property(TARGET dpdk::target_utils
                 PROPERTY IMPORTED_LOCATION ${LIBTARGET_UTILS})

    # dpdk::tdi
    find_library(LIBTDI tdi REQUIRED)
    add_library(dpdk::tdi SHARED IMPORTED)
    set_property(TARGET dpdk::tdi PROPERTY IMPORTED_LOCATION ${LIBTDI})

    # dpdk::tdi_json_parser
    find_library(LIBTDI_JSON_PARSER tdi_json_parser REQUIRED)
    add_library(dpdk::tdi_json_parser SHARED IMPORTED)
    set_property(TARGET dpdk::tdi_json_parser
                 PROPERTY IMPORTED_LOCATION ${LIBTDI_JSON_PARSER})

    #############
    # Variables #
    #############

    set(${_LIBS}
        dpdk::driver
        dpdk::bf_switchd_lib
        dpdk::tdi
        dpdk::tdi_json_parser
        dpdk::target_utils
        dpdk::target_sys
        PARENT_SCOPE
    )

    set(${_DIRS}
        ${SDE_INSTALL}/lib
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

