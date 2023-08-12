# Import Tofino SDE libraries.
#
# Copyright 2022-2023 Intel Corporation
# SPDX-License-Identifier: Apache 2.0
#

#-----------------------------------------------------------------------
# Define SDE library targets.
#-----------------------------------------------------------------------
# sde::driver
find_library(LIBDRIVER driver REQUIRED)
add_library(sde::driver SHARED IMPORTED)
set_property(TARGET sde::driver PROPERTY IMPORTED_LOCATION ${LIBDRIVER})

# sde::target_sys
find_library(LIBTARGET_SYS target_sys REQUIRED)
add_library(sde::target_sys SHARED IMPORTED)
set_property(TARGET sde::target_sys
             PROPERTY IMPORTED_LOCATION ${LIBTARGET_SYS})

# sde::tdi
find_library(LIBTDI tdi REQUIRED)
add_library(sde::tdi SHARED IMPORTED)
set_property(TARGET sde::tdi PROPERTY IMPORTED_LOCATION ${LIBTDI})

# sde::tdi_json_parser
find_library(LIBTDI_JSON_PARSER tdi_json_parser REQUIRED)
add_library(sde::tdi_json_parser SHARED IMPORTED)
set_property(TARGET sde::tdi_json_parser
             PROPERTY IMPORTED_LOCATION ${LIBTDI_JSON_PARSER})

#-----------------------------------------------------------------------
# Set SDE properties on target.
#-----------------------------------------------------------------------
function(add_tofino_target_libraries TGT)
  target_link_libraries(${TGT} PUBLIC
      sde::driver
      sde::tdi
      sde::tdi_json_parser
      sde::target_sys
  )
endfunction()
