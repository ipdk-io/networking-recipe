# CMake build file for script files
#
# Copyright 2023 Intel Corporation
# SPDX-License-Identifier: Apache 2.0
#

set(GENSCRIPT_COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/gen/genscript.py CACHE PATH
    "Command to generate a script file using a template")
mark_as_advanced(GENSCRIPT_COMMAND)

add_subdirectory(common)

if(DPDK_TARGET)
    add_subdirectory(dpdk)
elseif(ES2K_TARGET)
    add_subdirectory(es2k)
endif()
