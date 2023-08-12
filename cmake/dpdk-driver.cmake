# Import DPDK SDE libraries.
#
# Copyright 2022-2023 Intel Corporation
# SPDX-License-Identifier: Apache 2.0
#

include(FindPkgConfig)

pkg_check_modules(DPDK REQUIRED libdpdk)

#-----------------------------------------------------------------------
# Define SDE_INCLUDE_DIR.
#-----------------------------------------------------------------------
set(SDE_INCLUDE_DIR ${SDE_INSTALL_DIR}/include)

#-----------------------------------------------------------------------
# Define SDE library targets.
#-----------------------------------------------------------------------
# sde::bf_switchd
find_library(LIBBF_SWITCHD bf_switchd_lib REQUIRED)
  add_library(sde::bf_switchd SHARED IMPORTED)
  set_target_properties(sde::bf_switchd PROPERTIES
      PROPERTY IMPORTED_LOCATION ${LIBBF_SWITCHD})

# sde::driver
find_library(LIBDRIVER driver REQUIRED)
add_library(sde::driver SHARED IMPORTED)
set_target_properties(driver PROPERTIES
    IMPORTED_LOCATION ${LIBDRIVER})

# sde::tdi
find_library(LIBTDI tdi REQUIRED)
add_library(sde::tdi SHARED IMPORTED)
set_target_properties(sde::tdi PROPERTIES
    IMPORTED_LOCATION ${LIBTDI})

# sde::tdi_json_parser
find_library(LIBTDI_JSON_PARSER tdi_json_parser REQUIRED)
add_library(sde::tdi_json_parser SHARED IMPORTED)
set_target_properties(sde::tdi_json_parser PROPERTIES
    IMPORTED_LOCATION ${LIBTDI_JSON_PARSER})

# sde::target_sys
find_library(LIBTARGET_SYS target_sys REQUIRED)
add_library(sde::target_sys UNKNOWN IMPORTED)
set_target_properties(sde::target_sys PROPERTIES
    IMPORTED_LOCATION ${LIBTARGET_SYS}
    INTERFACE_INCLUDE_DIRECTORIES ${SDE_INCLUDE_DIR}
    IMPORTED_LINK_INTERFACE_LANGUAGES "C")

# sde::target_utils
find_library(LIBTARGET_UTILS target_utils)
add_library(sde::target_utils UNKNOWN IMPORTED)
set_target_properties(sde::target_utils PROPERTIES
    IMPORTED_LOCATION ${LIBTARGET_UTILS}
    INTERFACE_INCLUDE_DIRECTORIES ${SDE_INCLUDE_DIR}
    IMPORTED_LINK_INTERFACE_LANGUAGES "C")

#-----------------------------------------------------------------------
# Define SDE_LIBRARY_DIRS.
#-----------------------------------------------------------------------
set(SDE_LIBRARY_DIRS
    ${SDE_INSTALL_DIR}/lib
    ${DPDK_LIBRARY_DIRS}
)

#-----------------------------------------------------------------------
# Set SDE properties on target.
#-----------------------------------------------------------------------
function(add_dpdk_target_libraries TGT)

  target_link_libraries(${TGT} PUBLIC
      sde::driver
      sde::bf_switchd
      sde::tdi
      sde::tdi_json_parser
      sde::target_utils
      sde::target_sys
  )

  target_link_directories(${TGT} PUBLIC ${SDE_LIBRARY_DIRS})

  target_link_options(${TGT} PUBLIC
      ${DPDK_LD_FLAGS}
      ${DPDK_LDFLAGS_OTHER}
  )

endfunction()
