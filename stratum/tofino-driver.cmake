# Definitions for the Tofino P4 driver package.
#
# Copyright 2022-2023 Intel Corporation
# SPDX-License-Identifier: Apache 2.0
#

# The definitions are encapsulated in a function to limit pollution of the
# global namespace.
function(define_tofino_driver _LIBS)

    #############
    # Libraries #
    #############

    # driver
    find_library(LIBDRIVER driver REQUIRED)
    add_library(driver SHARED IMPORTED)
    set_property(TARGET driver PROPERTY IMPORTED_LOCATION ${LIBDRIVER})

    # target_sys
    find_library(LIBTARGET_SYS target_sys REQUIRED)
    add_library(target_sys SHARED IMPORTED)
    set_property(TARGET target_sys
                 PROPERTY IMPORTED_LOCATION ${LIBTARGET_SYS})

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
        tdi
        tdi_json_parser
        target_sys
        PARENT_SCOPE
    )

endfunction(define_tofino_driver)

define_tofino_driver(DRIVER_SDK_LIBS)
