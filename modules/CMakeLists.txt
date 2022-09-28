# CMake build file for modules directory
#
# Copyright 2022 Intel Corporation
# SPDX-License-Identifier: Apache 2.0

cmake_minimum_required(VERSION 3.5)

# Define PYTHON3 variable.
find_program(PYTHON3 python3)
if(NOT PYTHON3)
    include(FindPython)
    find_package(Python COMPONENTS Interpreter)
    set(PYTHON3 ${Python_EXECUTABLE})
endif()

if(WITH_STRATUM)
    add_subdirectory(clients)
endif()
