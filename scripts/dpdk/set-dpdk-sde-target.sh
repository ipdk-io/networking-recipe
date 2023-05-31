# Copyright 2022-2023 Intel Corporation
# SPDX-License-Identifier: Apache 2.0
#
# Creates a share/TARGET file that identifies this as an ES2K SDE.
# This allows Bazel (or any other tool) to determine the SDE type.
#

# Abort on error.
set -e

# Error check.
if [ -z "${SDE_INSTALL}" ]; then
    echo "SDE_INSTALL not defined!"
    exit 1
fi

print_target_type() {
    read TYPE
    echo "Target type is '$TYPE'"
}

# Create TARGET file.
filename="${SDE_INSTALL}/share/TARGET"
if [ ! -e "${filename}" ]; then
    echo "Creating ${filename}"
    echo "DPDK" > ${filename}
    echo "TARGET file created"
else
    echo "TARGET file already exists"
    print_target_type < ${filename}
fi
