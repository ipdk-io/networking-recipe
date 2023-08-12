# Import ES2K SDE libraries.
#
# Copyright 2022-2023 Intel Corporation
# SPDX-License-Identifier: Apache 2.0
#

#-----------------------------------------------------------------------
# Define SDE_INCLUDE_DIR.
#-----------------------------------------------------------------------
set(SDE_INCLUDE_DIR ${SDE_INSTALL_DIR}/include)

#-----------------------------------------------------------------------
# Define SDE library targets.
#-----------------------------------------------------------------------
# sde::bf_switchd
find_library(LIBBF_SWITCHD bf_switchd_lib REQUIRED)
if(LIBBF_SWITCHD)
  add_library(sde::bf_switchd UNKNOWN IMPORTED)
  set_target_properties(sde::bf_switchd PROPERTIES
      IMPORTED_LOCATION ${LIBBF_SWITCHD}
      INTERFACE_INCLUDE_DIRECTORIES ${SDE_INCLUDE_DIR}
      IMPORTED_LINK_INTERFACE_LANGUAGES "C")
else()
  message(FATAL_ERROR "sde::bf_switchd_lib library not found")
endif()

# sde::driver
find_library(LIBDRIVER driver REQUIRED)
if(LIBDRIVER)
  add_library(sde::driver UNKNOWN IMPORTED)
  set_target_properties(sde::driver PROPERTIES
      IMPORTED_LOCATION ${LIBDRIVER}
      INTERFACE_INCLUDE_DIRECTORIES ${SDE_INCLUDE_DIR}
      IMPORTED_LINK_INTERFACE_LANGUAGES "C")
else()
  message(FATAL_ERROR "sde::driver library not found")
endif()

# sde::tdi
find_library(LIBTDI tdi REQUIRED)
if(LIBTDI)
  add_library(sde::tdi SHARED IMPORTED)
  set_target_properties(sde::tdi PROPERTIES
      IMPORTED_LOCATION ${LIBTDI}
      INTERFACE_INCLUDE_DIRECTORIES ${SDE_INCLUDE_DIR}
      IMPORTED_LINK_INTERFACE_LANGUAGES "CXX")
else()
  message(FATAL_ERROR "sde::tdi library not found")
endif()

# sde::tdi_json_parser
find_library(LIBTDI_JSON_PARSER tdi_json_parser REQUIRED)
if(LIBTDI_JSON_PARSER)
  add_library(sde::tdi_json_parser UNKNOWN IMPORTED)
  set_target_properties(sde::tdi_json_parser PROPERTIES
      IMPORTED_LOCATION ${LIBTDI_JSON_PARSER}
      INTERFACE_INCLUDE_DIRECTORIES ${SDE_INCLUDE_DIR}
      IMPORTED_LINK_INTERFACE_LANGUAGES "CXX")
else()
  message(FATAL_ERROR "sde::tdi_json_parser library not found")
endif()

# sde::target_sys
find_library(LIBTARGET_SYS target_sys REQUIRED)
if(LIBTARGET_SYS)
  add_library(sde::target_sys UNKNOWN IMPORTED)
  set_target_properties(sde::target_sys PROPERTIES
      IMPORTED_LOCATION ${LIBTARGET_SYS}
      INTERFACE_INCLUDE_DIRECTORIES ${SDE_INCLUDE_DIR}
      IMPORTED_LINK_INTERFACE_LANGUAGES "C")
else()
  message(FATAL_ERROR "sde::target_sys library not found")
endif()

# sde::target_utils
find_library(LIBTARGET_UTILS target_utils REQUIRED)
if(LIBTARGET_UTILS)
  add_library(sde::target_utils UNKNOWN IMPORTED)
  set_target_properties(sde::target_utils PROPERTIES
      IMPORTED_LOCATION ${LIBTARGET_UTILS}
      INTERFACE_INCLUDE_DIRECTORIES ${SDE_INCLUDE_DIR}
      IMPORTED_LINK_INTERFACE_LANGUAGES "C")
else()
  message(FATAL_ERROR "sde::target_utils library not found")
endif()

#-----------------------------------------------------------------------
# Define SDE_LIBRARY_DIRS.
#-----------------------------------------------------------------------
function(_define_es2k_library_dirs DIRS)
  set(dirs ${SDE_INCLUDE_DIR}/lib)

  set(candidates
      ${SDE_INCLUDE_DIR}/lib/x86_64-linux-gnu
      ${SDE_INCLUDE_DIR}/lib64
  )

  # Find the auxiliary directory that contains the RTE libraries
  # and add it to the link directories list.
  foreach(dirpath ${candidates})
    if(EXISTS ${dirpath}/librte_ethdev.so)
      list(APPEND dirs ${dirpath})
      break()
    endif()
  endforeach()

  set(${DIRS} ${dirs} PARENT_SCOPE)
endfunction()

# library directories
_define_es2k_library_dirs(SDE_LIBRARY_DIRS)

#-----------------------------------------------------------------------
# Set SDE properties on target.
#-----------------------------------------------------------------------
function(add_es2k_target_libraries TGT)

  target_link_libraries(${TGT} PUBLIC
      sde::driver
      sde::bf_switchd
      sde::tdi
      sde::tdi_json_parser
      sde::target_utils
      sde::target_sys
  )

  target_link_directories(${TGT} PUBLIC ${SDE_LIBRARY_DIRS})

endfunction()
