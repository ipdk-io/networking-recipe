#!/bin/bash
#
# Copyright 2023 Intel Corporation
# SPDX-License-Identifier: Apache 2.0
#
# Sample script to configure and cross-compile Open vSwitch
# for the ES2K ACC platform. The build artifacts are installed
# by default in the SYSROOT /opt/ovs directory.
#

# Abort on error.
set -e

if [ -z "${SDKTARGETSYSROOT}" ]; then
    echo ""
    echo "Error: SDKTARGETSYSROOT is not defined!"
    echo "Did you forget to source the environment variables?"
    echo ""
    exit 1
fi

_SYSROOT=${SDKTARGETSYSROOT}

##################
# Default values #
##################

_BLD_DIR=ovs/build
_BLD_TYPE=RelWithDebInfo
_JOBS=8
_PREFIX=//opt/ovs
_TOOLFILE=${CMAKE_TOOLCHAIN_FILE}

##############
# print_help #
##############

print_help() {
    echo ""
    echo "Build and install Open vSwitch"
    echo ""
    echo "Options:"
    echo "  --build=DIR      -B  Build directory path [${_BLD_DIR}]"
    echo "  --dry-run        -n  Display cmake parameters and exit"
    echo "  --jobs=NJOBS     -j  Number of build threads (Default: ${_JOBS})"
    echo "  --prefix=DIR*    -P  Install directory prefix [${_PREFIX}]"
    echo "  --toolchain=FILE -T  CMake toolchain file"
    echo ""
    echo "* '//' at the beginning of the directory path will be replaced"
    echo "  with the sysroot directory path."
    echo ""
    echo "Environment variables:"
    echo "  CMAKE_TOOLCHAIN_FILE - Default toolchain file"
    echo "  SDKTARGETSYSROOT - sysroot directory"
    echo ""
}

######################
# print_cmake_params #
######################

print_cmake_params() {
    echo ""
    echo "CMAKE_BUILD_TYPE=${_BLD_TYPE}"
    echo "CMAKE_INSTALL_PREFIX=${_PREFIX}"
    echo "CMAKE_TOOLCHAIN_FILE=${_TOOLFILE}"
    echo "JOBS=${_JOBS}"
    echo ""
}

######################
# Parse command line #
######################

SHORTOPTS=B:P:T:j:
SHORTOPTS=${SHORTOPTS}hn

LONGOPTS=build:,jobs:,prefix:,toolchain:
LONGOPTS=${LONGOPTS},dry-run,help

GETOPTS=$(getopt -o ${SHORTOPTS} --long ${LONGOPTS} -- "$@")
eval set -- "${GETOPTS}"

while true ; do
    case "$1" in
    -B|--build)
        _BLD_DIR=$2
        shift 2 ;;
    -n|--dry-run)
        _DRY_RUN=true
        shift 1 ;;
    -h|--help)
        print_help
        exit 99 ;;
    -j|--jobs)
        _JOBS=$2
        shift 2 ;;
    -P|--prefix)
        _PREFIX=$2
        shift 2 ;;
    -T|--toolchain)
        _TOOLFILE=$2
        shift 2 ;;
    --)
        shift
        break ;;
    *)
        echo "Invalid parameter: $1"
        exit 1 ;;
    esac
done

######################
# Process parameters #
######################

# Substitute ${_SYSROOT}/ for '//' prefix
[ "${_PREFIX:0:2}" = "//" ] && _PREFIX=${_SYSROOT}/${_PREFIX:2}

if [ "${_DRY_RUN}" = "true" ]; then
    print_cmake_params
    exit 0
fi

#######################
# Configure the build #
#######################

rm -fr "${_BLD_DIR}"

cmake -S ovs -B "${_BLD_DIR}" \
    -DCMAKE_BUILD_TYPE=${_BLD_TYPE} \
    -DCMAKE_INSTALL_PREFIX="${_PREFIX}" \
    -DCMAKE_TOOLCHAIN_FILE="${_TOOLFILE}" \
    -DP4OVS=TRUE

#############
# Build OVS #
#############

cmake --build "${_BLD_DIR}" "-j${_JOBS}" -- V=0
