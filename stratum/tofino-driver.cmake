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

    # tofino::driver
    find_library(LIBDRIVER driver REQUIRED)
    add_library(tofino::driver SHARED IMPORTED)
    set_property(TARGET tofino::driver PROPERTY IMPORTED_LOCATION ${LIBDRIVER})

    # tofino::target_sys
    find_library(LIBTARGET_SYS target_sys REQUIRED)
    add_library(tofino::target_sys SHARED IMPORTED)
    set_property(TARGET tofino::target_sys
                 PROPERTY IMPORTED_LOCATION ${LIBTARGET_SYS})

    # tofino::tdi
    find_library(LIBTDI tdi REQUIRED)
    add_library(tofino::tdi SHARED IMPORTED)
    set_property(TARGET tofino::tdi PROPERTY IMPORTED_LOCATION ${LIBTDI})

    # tofino::tdi_json_parser
    find_library(LIBTDI_JSON_PARSER tdi_json_parser REQUIRED)
    add_library(tofino::tdi_json_parser SHARED IMPORTED)
    set_property(TARGET tofino::tdi_json_parser
                 PROPERTY IMPORTED_LOCATION ${LIBTDI_JSON_PARSER})

    #############
    # Variables #
    #############

    set(${_LIBS}
        tofino::driver
        tofino::tdi
        tofino::tdi_json_parser
        tofino::target_sys
        PARENT_SCOPE
    )

endfunction(define_tofino_driver)

define_tofino_driver(DRIVER_SDK_LIBS)
