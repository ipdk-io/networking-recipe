# cmake/LnwVersion.cmake
#
# Copyright 2024 Intel Corporation
# SPDX-License-Identifier: Apache 2.0
#
# Processes the Linux Networking Version (LNW_VERSION) option.
#

if(ES2K_TARGET)
  string(STRIP "${LNW_VERSION}" LNW_VERSION)

  if(LNW_VERSION STREQUAL "2" OR LNW_VERSION STREQUAL "3")
    set(LNW_VERSION "${LNW_VERSION}" CACHE STRING "" FORCE)
  elseif(LNW_VERSION STREQUAL "")
    set(LNW_VERSION "2" CACHE STRING "" FORCE)
  else()
    message(FATAL_ERROR
      "LNW_VERSION accepts values {2,3} but was set to \"${LNW_VERSION}\"")
  endif()

  set(lnw_flag "LNW_V${LNW_VERSION}")
  add_compile_options("-D${lnw_flag}")
  set(${lnw_flag} TRUE)
  unset(lnw_flag)
else()
  set(LNW_VERSION "0" CACHE STRING "" FORCE)
endif()
