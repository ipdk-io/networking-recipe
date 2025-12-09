# //bazel/rules:library_rule.bzl

# Copyright 2018-present Open Networking Foundation
# Copyright 2024 Intel Corporation
# SPDX-License-Identifier: Apache-2.0

load("@rules_cc//cc:defs.bzl", "cc_library")
load("//bazel:defs.bzl", "TARGET_DEFINES")

def krnlmon_cc_library(
        name,
        deps = None,
        srcs = None,
        data = None,
        hdrs = None,
        copts = [],
        defines = [],
        include_prefix = None,
        includes = None,
        strip_include_prefix = None,
        testonly = None,
        textual_hdrs = None,
        visibility = None,
        arches = None,
        linkopts = []):
    cc_library(
        name = name,
        deps = deps,
        srcs = srcs,
        data = data,
        hdrs = hdrs,
        # alwayslink = alwayslink,
        copts = copts,
        defines = TARGET_DEFINES + defines,
        include_prefix = include_prefix,
        includes = includes,
        strip_include_prefix = strip_include_prefix,
        testonly = testonly,
        textual_hdrs = textual_hdrs,
        visibility = visibility,
        linkopts = linkopts,
    )
