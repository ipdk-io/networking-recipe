# CMake build file for infrap4d/daemon
#
# Copyright 2022 Intel Corporation
# SPDX-License-Identifier: Apache 2.0

cmake_minimum_required(VERSION 3.5)

add_library(daemon_o OBJECT
   daemon.c
   daemon.h
   daemon-unix.c
   fatal-signal.c
   fatal-signal.h
)
