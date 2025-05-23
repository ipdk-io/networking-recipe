# Build file for p4rt-ctl.
#
# Copyright 2022-2025 Intel Corporation
# SPDX-License-Identifier: Apache 2.0
#
cmake_minimum_required(VERSION 3.16)

# Define PYTHON3 variable.
find_program(PYTHON3 python3)
mark_as_advanced(PYTHON3)
if(NOT PYTHON3)
    include(FindPython)
    find_package(Python COMPONENTS Interpreter)
    set(PYTHON3 ${Python_EXECUTABLE})
endif()

set(P4RT_CTL_OUT ${CMAKE_CURRENT_BINARY_DIR}/p4rt-ctl)

configure_file(p4rt-ctl.in ${P4RT_CTL_OUT} @ONLY)

# The target cannot have the same name as the generated file.
# This causes a dependency cycle in Ninja.
add_custom_target(p4rt-ctl-target ALL DEPENDS ${P4RT_CTL_OUT})

set_source_files_properties(${P4RT_CTL_OUT} PROPERTIES GENERATED TRUE)

install(PROGRAMS ${P4RT_CTL_OUT} DESTINATION bin)
