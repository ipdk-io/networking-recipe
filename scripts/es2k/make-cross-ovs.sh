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
_DRY_RUN=0
_NJOBS=8
_PREFIX=//opt/ovs
_TOOLFILE=${CMAKE_TOOLCHAIN_FILE}

##############
# print_help #
##############

print_help() {
    echo ""
    echo "Build and install Open vSwitch"
    echo ""
    echo "Paths:"
    echo "  --build=DIR      -B  Build directory path [${_BLD_DIR}]"
    echo "  --prefix=DIR*    -P  Install directory prefix [${_PREFIX}]"
    echo "  --toolchain=FILE -T  CMake toolchain file"
    echo ""
    echo "Options:"
    echo "  --dry-run        -n  Display cmake parameters and exit"
    echo "  --help           -h  Display this help text"
    echo "  --jobs=NJOBS     -j  Number of build threads [${_NJOBS}]"
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
    echo "CMAKE_INSTALL_PREFIX=${_PREFIX}"
    echo "CMAKE_TOOLCHAIN_FILE=${_TOOLFILE}"
    echo "JOBS=${_NJOBS}"
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
    # Paths
    --build|-B)
        _BLD_DIR=$2
        shift 2 ;;
    --prefix|-P)
        _PREFIX=$2
        shift 2 ;;
    --toolchain|-T)
        _TOOLFILE=$2
        shift 2 ;;
    # Options
    --dry-run|-n)
        _DRY_RUN=1
        shift 1 ;;
    --help|-h)
        print_help
        exit 99 ;;
    --jobs|-j)
        _NJOBS=$2
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

# Show parameters if this is a dry run
if [ ${_DRY_RUN} -ne 0 ]; then
    print_cmake_params
    exit 0
fi

#######################
# Configure the build #
#######################

rm -fr "${_BLD_DIR}"

cmake -S ovs -B "${_BLD_DIR}" \
    -DCMAKE_INSTALL_PREFIX="${_PREFIX}" \
    -DCMAKE_TOOLCHAIN_FILE="${_TOOLFILE}" \
    -DP4OVS=TRUE

#############
# Build OVS #
#############

cmake --build "${_BLD_DIR}" "-j${_NJOBS}" -- V=0
