# Support include-what-you-use
#
# Copyright 2023-2024 Intel Corporation
# SPDX-License-Identifier: Apache 2.0
#
# include-what-you-use is a utility developed at Google that analyzes
# C and C++ source files, looking for violations of the Include What You Use
# rule, and recommends fixes for them.
#
# For more information:
#
#   https://include-what-you-use.org/
#
# To run include-what-you-use:
#
#   cmake -B build <other options> -DIWYU=ON
#   cmake --build build >& iwyu.log

option(IWYU "Run include-what-you-use" OFF)

if(IWYU)
  find_program(IWYU_PATH NAMES include-what-you-use iwyu)
  if(NOT IWYU_PATH)
    message(FATAL_ERROR "Could not find include-what-you-use")
  endif()
  mark_as_advanced(IWYU_PATH)
  set(CMAKE_CXX_INCLUDE_WHAT_YOU_USE ${IWYU_PATH})
  set(CMAKE_C_INCLUDE_WHAT_YOU_USE ${IWYU_PATH})
  message("IWYU enabled")
endif()
