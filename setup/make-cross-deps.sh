#!/bin/bash
#
# Copyright 2022-2023 Intel Corporation
# SPDX-License-Identifier: Apache 2.0
#
# Sample script to configure and build the dependency libraries
# on the development host when cross-compiling for the ES2K ACC.
#

# Abort on error.
set -e

if [ -z "${SDKTARGETSYSROOT}" ]; then
    echo ""
    echo "-----------------------------------------------------"
    echo "Error: SDKTARGETSYSROOT is not defined!"
    echo "Did you forget to source the environment variables?"
    echo "-----------------------------------------------------"
    echo ""
    exit 1
fi

_SYSROOT=${SDKTARGETSYSROOT}

##################
# Default values #
##################

_BLD_DIR=build
_DO_BUILD=true
_DRY_RUN=false
_NJOBS=8
_PREFIX=//opt/deps
_TOOLFILE=${CMAKE_TOOLCHAIN_FILE}

##############
# print_help #
##############

print_help() {
    echo ""
    echo "Build target dependency libraries"
    echo ""
    echo "Paths:"
    echo "  --build=DIR      -B  Build directory path [${_BLD_DIR}]"
    echo "  --host=DIR       -H  Host dependencies directory [${_HOST_DIR}]"
    echo "  --prefix=DIR*    -P  Install directory prefix [${_PREFIX}]"
    echo "  --toolchain=FILE -T  CMake toolchain file"
    echo ""
    echo "Options:"
    echo "  --cxx=VERSION        CXX_STANDARD to build dependencies (Default: empty)"
    echo "  --dry-run        -n  Display cmake parameters and exit"
    echo "  --force          -f  Specify -f when patching (Default: false)"
    echo "  --help           -h  Display this help text"
    echo "  --jobs=NJOBS     -j  Number of build threads (Default: ${_NJOBS})"
    echo "  --no-build           Configure without building"
    echo "  --no-download        Do not download repositories (Default: false)"
    echo "  --sudo               Use sudo when installing (Default: false)"
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
# Parse command line #
######################

SHORTOPTS=B:H:P:T:fhj:n
LONGOPTS=build:,cxx:,hostdeps:,jobs:,prefix:,toolchain:
LONGOPTS=${LONGOPTS},dry-run,force,help,no-build,no-download,sudo

eval set -- `getopt -o ${SHORTOPTS} --long ${LONGOPTS} -- "$@"`

while true ; do
    case "$1" in
    # Paths
    -B|--build)
        _BLD_DIR=$2
        shift 2 ;;
    -H|--hostdeps)
        _HOST_DIR=$2
        shift 2 ;;
    -P|--prefix)
        _PREFIX=$2
        shift 2 ;;
    -T|--toolchain)
        _TOOLFILE=$2
        shift 2 ;;
    # Options
    --cxx)
        _CXX_STANDARD_OPTION="-DCXX_STANDARD=$2"
        shift 2 ;;
    -f|--force)
        _FORCE_PATCH="-DFORCE_PATCH=TRUE"
        shift 1 ;;
    -h|--help)
        print_help
        exit 99 ;;
    -j|--jobs)
        _NJOBS=$2
        shift 2 ;;
    -n|--dry-run)
        _DRY_RUN=true
        shift 1 ;;
    --no-build)
        _DO_BUILD=false
        shift ;;
    --no-download)
        _DOWNLOAD="-DDOWNLOAD=FALSE"
        shift 1 ;;
    --sudo)
        _USE_SUDO="-DUSE_SUDO=TRUE"
        shift ;;
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

# Replace "//"" prefix with "${_SYSROOT}/""
[ "${_PREFIX:0:2}" = "//" ] && _PREFIX="${_SYSROOT}/${_PREFIX:2}"

if [ -n "${_HOST_DIR}" ]; then
    _HOST_DEPEND_DIR="-DHOST_DEPEND_DIR=${_HOST_DIR}"
fi

if [ "${_DRY_RUN}" = "true" ]; then
    echo ""
    echo "CMAKE_INSTALL_PREFIX=${_PREFIX}"
    echo "CMAKE_TOOLCHAIN_FILE=${_TOOLFILE}"
    echo "JOBS=${_NJOBS}"
    [ -n "${_CXX_STANDARD_OPTION}" ] && echo "${_CXX_STANDARD_OPTION:2}"
    [ -n "${_DOWNLOAD}" ] && echo "${_DOWNLOAD:2}"
    [ -n "${_HOST_DEPEND_DIR}" ] && echo "${_HOST_DEPEND_DIR:2}"
    [ -n "${_FORCE_PATCH}" ] && echo "${_FORCE_PATCH:2}"
    [ -n "${_USE_SUDO}" ] && echo "${_USE_SUDO:2}"

    if [ "${_DO_BUILD}" = "false" ]; then
	echo ""
        echo "Configure without building"
    fi

    echo ""
    exit 0
fi

######################
# Build dependencies #
######################

rm -fr ${_BLD_DIR}

cmake -S . -B ${_BLD_DIR} \
    -DCMAKE_INSTALL_PREFIX=${_PREFIX} \
    -DCMAKE_TOOLCHAIN_FILE=${_TOOLFILE} \
    ${_HOST_DEPEND_DIR} \
    ${_CXX_STANDARD_OPTION} \
    ${_DOWNLOAD} ${_FORCE_PATCH} ${_USE_SUDO}

if [ "${_DO_BUILD}" = "true" ]; then
    cmake --build ${_BLD_DIR} -j${_NJOBS}
fi
