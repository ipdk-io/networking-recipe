#!/bin/bash
#
# Copyright 2022-2023 Intel Corporation
# SPDX-License-Identifier: Apache 2.0
#
# Sample script to configure and build the P4 Control Plane software.
#

##################
# Default values #
##################

_BLD_TYPE="RelWithDebInfo"
_DEPS_DIR=${DEPEND_INSTALL}
_HOST_DIR=${HOST_INSTALL:-setup/host-deps}
_JOBS=8
_OVS_DIR="${OVS_INSTALL:-ovs/install}"
_PREFIX="install"
_RPATH=OFF
_SDE_DIR=${SDE_INSTALL}
_TGT_TYPE="DPDK"
_TOOLFILE=${CMAKE_TOOLCHAIN_FILE}

_BLD_DIR=build
_DRY_RUN=0
_OVS_BLD="ovs/build"
_WITH_OVS=1

##############
# print_help #
##############

print_help() {
    echo ""
    echo "Configure and build P4 Control Plane software"
    echo ""
    echo "Paths:"
    echo "  --deps=DIR       -D  Target dependencies directory [${_DEPS_DIR}]"
    echo "  --host=DIR       -H  Host dependencies directory [${_HOST_DIR}]"
    echo "  --ovs=DIR        -O  OVS install directory [${_OVS_DIR}]"
    echo "  --prefix=DIR     -P  Install directory prefix [${_PREFIX}]"
    echo "  --sde=DIR        -S  SDE install directory [${_SDE_DIR}]"
    echo "  --toolchain=FILE -T  CMake toolchain file [${_TOOLFILE}]"
    echo ""
    echo "Options:"
    echo "  --dry-run        -n  Display cmake parameter values and exit"
    echo "  --help           -h  Display this help text"
    echo "  --jobs=NJOBS     -j  Number of build threads (Default: ${_JOBS})"
    echo "  --no-krnlmon         Exclude Kernel Monitor"
    echo "  --no-ovs             Exclude OVS support"
    echo "  --target=TARGET      Target to build (dpdk|es2k|tofino) [${_TGT_TYPE}]"
    echo ""
    echo "Configurations:"
    echo "  --debug              Debug configuration"
    echo "  --minsize            MinSizeRel configuration"
    echo "  --reldeb             RelWithDebInfo configuration (default)"
    echo "  --release            Release configuration"
    echo ""
    echo "  Default config is RelWithDebInfo (--reldeb)"
    echo ""
    echo "Environment variables:"
    echo "  CMAKE_TOOLCHAIN_FILE - Default toolchain file"
    echo "  DEPEND_INSTALL - Default target dependencies directory"
    echo "  HOST_INSTALL - Default host dependencies directory"
    echo "  OVS_INSTALL - Default OVS install directory"
    echo "  SDE_INSTALL - Default SDE install directory"
    echo ""
}

######################
# print_cmake_params #
######################

print_cmake_params() {
    echo ""
    echo "CMAKE_BUILD_TYPE=${_BLD_TYPE}"
    echo "CMAKE_INSTALL_PREFIX=${_PREFIX}"
    [ -n "${_CMAKE_TOOLCHAIN_FILE}" ] && echo "${_CMAKE_TOOLCHAIN_FILE:2}"
    echo "DEPEND_INSTALL_DIR=${_DEPS_DIR}"
    [ -n "${_HOST_DEPEND_DIR}" ] && echo "${_HOST_DEPEND_DIR:2}"
    echo "OVS_INSTALL_DIR=${_OVS_DIR}"
    echo "SDE_INSTALL_DIR=${_SDE_DIR}"
    [ -n "${_WITH_KRNLMON}" ] && echo "${_WITH_KRNLMON:2}"
    [ -n "${_WITH_OVSP4RT}" ] && echo "${_WITH_OVSP4RT:2}"
    echo "${_SET_RPATH:2}"
    echo "${_TARGET_TYPE:2}"
    echo "JOBS=${_JOBS}"
    echo ""
}

#############
# build_ovs #
#############

build_ovs() {
    cmake -S ovs -B ${_OVS_BLD} \
        -DCMAKE_BUILD_TYPE=${_BLD_TYPE} \
        -DCMAKE_INSTALL_PREFIX=${_OVS_DIR} \
        ${_CMAKE_TOOLCHAIN_FILE} \
        -DP4OVS=ON

    cmake --build ${_OVS_BLD} -j6 -- V=0
}

################
# build_recipe #
################

build_recipe() {
    cmake -S . -B ${_BLD_DIR} \
        -DCMAKE_BUILD_TYPE=${_BLD_TYPE} \
        -DCMAKE_INSTALL_PREFIX=${_PREFIX} \
        ${_CMAKE_TOOLCHAIN_FILE} \
        -DDEPEND_INSTALL_DIR=${_DEPS_DIR} \
        ${_HOST_DEPEND_DIR} \
        -DOVS_INSTALL_DIR=${_OVS_DIR} \
        -DSDE_INSTALL_DIR=${_SDE_DIR} \
        ${_WITH_KRNLMON} ${_WITH_OVSP4RT} \
        ${_SET_RPATH} \
        ${_TARGET_TYPE} -DRTE_FLOW_SHIM=TRUE

    cmake --build ${_BLD_DIR} -j${_JOBS} --target install
}

######################
# Parse command line #
######################

SHORTOPTS=D:H:O:P:S:T:
SHORTOPTS=${SHORTOPTS}hn

LONGOPTS=deps:,hostdeps:,ovs:,prefix:,sde:,target:,toolchain:
LONGOPTS=${LONGOPTS},debug,release,minsize,reldeb
LONGOPTS=${LONGOPTS},dry-run,help,no-krnlmon,no-ovs,rpath,no-rpath

GETOPTS=`getopt -o ${SHORTOPTS} --long ${LONGOPTS} -- "$@"`
eval set -- "${GETOPTS}"

# Process command-line options.
while true ; do
    case "$1" in
    # Paths
    -D|--deps)
        _DEPS_DIR=$2
        shift 2 ;;
    -H|--hostdeps)
        _HOST_DIR=$2
        shift 2 ;;
    -O|--ovsdir)
        _OVS_DIR=$2
        shift ;;
    -P|--prefix)
        _PREFIX=$2
        shift 2 ;;
    -S|--sde)
        _SDE_DIR=$2
        shift 2 ;;
    -T|--toolchain)
        _TOOLFILE=$2
        shift 2 ;;
    # Configurations
    --debug)
        _BLD_TYPE="Debug"
        shift ;;
    --minsize)
        _BLD_TYPE="MinSizeRel"
        shift ;;
    --reldeb)
        _BLD_TYPE="RelWithDebInfo"
        shift ;;
    --release)
        _BLD_TYPE="Release"
        shift ;;
    # Options
    -n|--dry_run)
        _DRY_RUN=1
        shift ;;
    -h|--help)
        print_help
        exit 99 ;;
    -j|--jobs)
        _JOBS=$2
        shift 2 ;;
    --no-krnlmon)
        _WITH_KRNLMON=FALSE
        shift ;;
    --no-ovs)
        _WITH_OVS=0
        _WITH_OVSP4RT=FALSE
        shift ;;
    --no-rpath)
        _RPATH=OFF
        shift ;;
    --rpath)
        _RPATH=ON
        shift ;;
    --target)
        # convert to uppercase
        _TGT_TYPE=${2^^}
        shift 2 ;;
    --)
        shift
        break ;;
    *)
        echo "Internal error!"
        exit 1 ;;
    esac
done

######################
# Process parameters #
######################

if [ -z "${_SDE_DIR}" ]; then
    echo "ERROR: SDE_INSTALL not defined!"
    exit 1
fi

# --host and --toolfile are only used when cross-compiling
if [ -n "${_HOST_DIR}" ]; then
    _HOST_DEPEND_DIR=-DHOST_DEPEND_DIR=${_HOST_DIR}
fi

if [ -n "${_TOOLFILE}" ]; then
    _CMAKE_TOOLCHAIN_FILE=-DCMAKE_TOOLCHAIN_FILE=${_TOOLFILE}
fi

_SET_RPATH=-DSET_RPATH=${_RPATH}
_TARGET_TYPE=-D${_TGT_TYPE}_TARGET=ON

# Expand WITH_KRNLMON and WITH_OVSP4RT if not empty
[ -n "${_WITH_KRNLMON}" ] && _WITH_KRNLMON=-DWITH_KRNLMON=${_WITH_KRNLMON}
[ -n "${_WITH_OVSP4RT}" ] && _WITH_OVSP4RT=-DWITH_OVSP4RT=${_WITH_OVSP4RT}

# Show parameters if this is a dry run
if [ ${_DRY_RUN} -ne 0 ]; then
    print_cmake_params
    exit 0
fi

################
# Do the build #
################

# Abort on error.
set -e

# First build OVS
if [ ${_WITH_OVS} -ne 0 ]; then
    build_ovs
fi

# Now build the rest of the recipe.
build_recipe
