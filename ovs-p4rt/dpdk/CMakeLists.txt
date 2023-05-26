# CMake build file for krnlmon/switchapi
#
# Copyright 2022-2023 Intel Corporation
# SPDX-License-Identifier: Apache 2.0

cmake_minimum_required(VERSION 3.5)

add_library(ovsp4rt_p4_mapping_o OBJECT
    p4_name_mapping.h
)
