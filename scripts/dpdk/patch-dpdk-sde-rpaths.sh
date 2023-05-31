# Copyright 2022-2023 Intel Corporation
# SPDX-License-Identifier: Apache 2.0
#
# Patches several libraries in the DPDK Target SDE so infrap4d can be run
# without requiring that LD_LIBRARY_PATH be set.
#
# Note that P4 Control Plane must be built with the RPATH option enabled
# for this to work.
#

# Abort on error.
set -e

# Libraries to patch.
dpdk_infra_lib=libdpdk_infra.so
dpdk_infra_path=${SDE_INSTALL}/lib/${dpdk_infra_lib}

driver_lib=libdriver.so
driver_path=${SDE_INSTALL}/lib/${driver_lib}

pipeline_lib=librte_pipeline.so

# Error checks.
if [ -z "${SDE_INSTALL}" ]; then
    echo "SDE_INSTALL not defined!"
    exit 1
fi

if [ ! -e "${dpdk_infra_path}" ]; then
    echo "Not a DPDK SDE!"
    exit 1
fi

# RTE library paths.
libx86_64=${SDE_INSTALL}/lib/x86_64-linux-gnu
lib64=${SDE_INSTALL}/lib64

# RTE libraries are in lib/x86_64-linux-gnu directory.
patch_lib_x86_64_libs() {
    echo "Patching ${dpdk_infra_path}"
    patchelf --set-rpath '$ORIGIN/x86_64-linux-gnu' ${dpdk_infra_path}
    patchelf --print-rpath ${dpdk_infra_path}

    echo "Patching ${driver_path}"
    patchelf --set-rpath '$ORIGIN:$ORIGIN/x86_64-linux-gnu' ${driver_path}
    patchelf --print-rpath ${driver_path}

    filename="${SDE_INSTALL}/lib/x86_64-linux-gnu/${pipeline_lib}"
    echo "Patching ${filename}"
    patchelf --set-rpath '$ORIGIN' ${filename}
    patchelf --print-rpath ${filename}
}

# RTE libraries are in lib64 directory.
patch_lib64_libs() {
    echo "Patching ${dpdk_infra_path}"
    patchelf --set-rpath '$ORIGIN/../lib64' ${dpdk_infra_path}
    patchelf --print-rpath ${dpdk_infra_path}

    echo "Patching ${driver_path}"
    patchelf --set-rpath '$ORIGIN:$ORIGIN/../lib64' ${driver_path}
    patchelf --print-rpath ${driver_path}

    filename="${SDE_INSTALL}/lib64/${pipeline_lib}"
    echo "Patching ${filename}"
    patchelf --set-rpath '$ORIGIN' ${filename}
    patchelf --print-rpath ${filename}
}

# RTE libraries are in lib directory.
patch_lib_libs() {
    echo "Patching ${dpdk_infra_path}"
    patchelf --set-rpath '$ORIGIN' ${dpdk_infra_path}
    patchelf --print-rpath ${dpdk_infra_path}

    echo "Patching ${driver_path}"
    patchelf --set-rpath '$ORIGIN' ${driver_path}
    patchelf --print-rpath ${driver_path}

    filename="${SDE_INSTALL}/lib/${pipeline_lib}"
    echo "Patching ${filename}"
    patchelf --set-rpath '$ORIGIN' ${filename}
    patchelf --print-rpath ${filename}
}

# Patch libraries.
if [ -e "${libx86_64}/${pipeline_lib}" ]; then
    patch_lib_x86_64_libs
elif [ -e "${lib64}/${pipeline_lib}" ]; then
    patch_lib64_libs
elif [ -e "${SDE_INSTALL}/lib/${pipeline_lib}" ]; then
    patch_lib_libs
else
    echo "No libraries patched!"
fi
