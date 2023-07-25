#!/bin/bash

# Copyright (C) 2021-2023 Intel Corporation
# SPDX-License-Identifier: Apache-2.0

set -e

if [ -z "$1" ]; then
    echo "- Missing mandatory argument: P4CP_INSTALL"
    echo " - Usage: source setup_env.sh <P4CP_INSTALL> <SDE_INSTALL> <DEPEND_INSTALL>"
    return 0
fi

if [ -z "$2" ]; then
    echo "- Missing mandatory argument: SDE_INSTALL"
    echo " - Usage: source setup_env.sh <P4CP_INSTALL> <SDE_INSTALL> <DEPEND_INSTALL>"
    return 0
fi

if [ -z "$3" ]; then
    echo "- Missing mandatory argument: DEPEND_INSTALL"
    echo " - Usage: source setup_env.sh <P4CP_INSTALL> <SDE_INSTALL> <DEPEND_INSTALL>"
    return 0
fi

export P4CP_INSTALL=$1
export SDE_INSTALL=$2
export DEPEND_INSTALL=$3

# Get the OS and Version details
# shellcheck source=/dev/null
SCRIPT_DIR="$( cd "$(dirname "$( readlink -f ${BASH_SOURCE[0]} )")" >/dev/null 2>&1 && pwd)"
. "${SCRIPT_DIR}/os_ver_details.sh"
get_os_ver_details
echo "OS and Version details..."
echo "${OS} : ${VER}"

# Update SDE libraries
export LD_LIBRARY_PATH=${LD_LIBRARY_PATH}:${SDE_INSTALL}/lib
export LD_LIBRARY_PATH=${LD_LIBRARY_PATH}:${SDE_INSTALL}/lib64

if [ "$OS" = "Fedora" ]; then
    export LD_LIBRARY_PATH=${LD_LIBRARY_PATH}:${SDE_INSTALL}/lib64
else
    export LD_LIBRARY_PATH=${LD_LIBRARY_PATH}:${SDE_INSTALL}/lib/x86_64-linux-gnu
fi

# Update P4CP libraries
export LD_LIBRARY_PATH=${P4CP_INSTALL}/install/lib:${P4CP_INSTALL}/install/lib64:${LD_LIBRARY_PATH}
export PATH=${P4CP_INSTALL}/bin:${P4CP_INSTALL}/sbin:$PATH

export LD_LIBRARY_PATH=${LD_LIBRARY_PATH}:/usr/local/lib
export LD_LIBRARY_PATH=${LD_LIBRARY_PATH}:/usr/local/lib64

# Update Dependent libraries
export LD_LIBRARY_PATH=${DEPEND_INSTALL}/lib:${DEPEND_INSTALL}/lib64:${LD_LIBRARY_PATH}
export PATH=${DEPEND_INSTALL}/bin:${DEPEND_INSTALL}/sbin:${P4CP_INSTALL}/sbin:${P4CP_INSTALL}/bin:$PATH
export LIBRARY_PATH=${DEPEND_INSTALL}/lib:${DEPEND_INSTALL}/lib64:${LIBRARY_PATH}

echo ""
echo ""
echo "Updated Environment Variables ..."
echo "SDE_INSTALL: ${SDE_INSTALL}"
echo "DEPEND_INSTALL: ${DEPEND_INSTALL}"
echo "P4CP_INSTALL: ${P4CP_INSTALL}"
echo "LIBRARY_PATH: ${LIBRARY_PATH}"
echo "LD_LIBRARY_PATH: ${LD_LIBRARY_PATH}"
echo "PATH: $PATH"
echo ""

sudo "${SCRIPT_DIR}/set_hugepages.sh"

set +e
