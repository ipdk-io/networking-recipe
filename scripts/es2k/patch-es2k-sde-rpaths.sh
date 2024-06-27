#!/bin/bash
# Copyright 2022-2024 Intel Corporation
# SPDX-License-Identifier: Apache-2.0
#
# Sets RPATH for several libraries in the ES2K Target SDE so infrap4d
# can be run without requiring that LD_LIBRARY_PATH be set.
#
# Requires that patchelf be installed on the development system.
#
# Note that P4 Control Plane should also be built with RPATH enabled.
#

# Abort on error.
set -e

# Libraries to patch.
es2k_infra_lib=libcpf_pmd_infra.so
es2k_infra_path=${SDE_INSTALL}/lib/${es2k_infra_lib}

driver_lib=libdriver.so
driver_path=${SDE_INSTALL}/lib/${driver_lib}

net_idpf_lib=librte_net_idpf.so
net_cpfl_lib=librte_net_cpfl.so

# Error checks.
if [ -z "${SDE_INSTALL}" ]; then
    echo "SDE_INSTALL not defined!"
    exit 1
fi

if [ ! -e "${es2k_infra_path}" ]; then
    echo "Not an ES2K SDE!"
    exit 1
fi

# RTE library paths.
libx86_64=${SDE_INSTALL}/lib/x86_64-linux-gnu
lib64=${SDE_INSTALL}/lib64

# RTE libraries are in lib/x86_64-linux-gnu directory.
patch_lib_x86_64_libs() {
    echo "Patching ${es2k_infra_path}"
    # shellcheck disable=SC2016
    patchelf --set-rpath '$ORIGIN/x86_64-linux-gnu' "${es2k_infra_path}"
    patchelf --print-rpath "${es2k_infra_path}"

    echo "Patching ${driver_path}"
    # shellcheck disable=SC2016
    patchelf --set-rpath '$ORIGIN:$ORIGIN/x86_64-linux-gnu' "${driver_path}"
    patchelf --print-rpath "${driver_path}"

    filename="${SDE_INSTALL}/lib/x86_64-linux-gnu/${net_idpf_lib}"
    echo "Patching ${filename}"
    # shellcheck disable=SC2016
    patchelf --set-rpath '$ORIGIN' "${filename}"
    patchelf --print-rpath "${filename}"

    filename="${SDE_INSTALL}/lib/x86_64-linux-gnu/${net_cpfl_lib}"
    echo "Patching ${filename}"
    # shellcheck disable=SC2016
    patchelf --set-rpath '$ORIGIN' "${filename}"
    patchelf --print-rpath "${filename}"
}

# RTE libraries are in lib64 directory.
patch_lib64_libs() {
    echo "Patching ${es2k_infra_path}"
    # shellcheck disable=SC2016
    patchelf --set-rpath '$ORIGIN/../lib64' "${es2k_infra_path}"
    patchelf --print-rpath "${es2k_infra_path}"

    echo "Patching ${driver_path}"
    # shellcheck disable=SC2016
    patchelf --set-rpath '$ORIGIN:$ORIGIN/../lib64' "${driver_path}"
    patchelf --print-rpath "${driver_path}"

    filename="${SDE_INSTALL}/lib64/${net_idpf_lib}"
    echo "Patching ${filename}"
    # shellcheck disable=SC2016
    patchelf --set-rpath '$ORIGIN' "${filename}"
    patchelf --print-rpath "${filename}"

    filename="${SDE_INSTALL}/lib64/${net_cpfl_lib}"
    echo "Patching ${filename}"
    # shellcheck disable=SC2016
    patchelf --set-rpath '$ORIGIN' "${filename}"
    patchelf --print-rpath "${filename}"
}

# RTE libraries are in lib directory.
patch_lib_libs() {
    echo "Patching ${es2k_infra_path}"
    # shellcheck disable=SC2016
    patchelf --set-rpath '$ORIGIN' "${es2k_infra_path}"
    patchelf --print-rpath "${es2k_infra_path}"

    echo "Patching ${driver_path}"
    # shellcheck disable=SC2016
    patchelf --set-rpath '$ORIGIN' "${driver_path}"
    patchelf --print-rpath "${driver_path}"

    filename="${SDE_INSTALL}/lib/${net_idpf_lib}"
    echo "Patching ${filename}"
    # shellcheck disable=SC2016
    patchelf --set-rpath '$ORIGIN' "${filename}"
    patchelf --print-rpath "${filename}"

    filename="${SDE_INSTALL}/lib/${net_cpfl_lib}"
    echo "Patching ${filename}"
    # shellcheck disable=SC2016
    patchelf --set-rpath '$ORIGIN' "${filename}"
    patchelf --print-rpath "${filename}"
}

# Patch libraries.
if [ -e "${libx86_64}/${net_idpf_lib}" ]; then
    patch_lib_x86_64_libs
elif [ -e "${lib64}/${net_idpf_lib}" ]; then
    patch_lib64_libs
elif [ -e "${SDE_INSTALL}/lib/${net_idpf_lib}" ]; then
    patch_lib_libs
else
    echo "No libraries patched!"
fi
