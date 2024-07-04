#!/bin/bash
#
# Copyright 2022-2024 Intel Corporation
# SPDX-License-Identifier: Apache 2.0
#
# Sample script to configure and build P4 Control Plane.
#

# Abort on error.
set -e

##################
# Default values #
##################

_DEPS_DIR=${DEPEND_INSTALL}
_HOST_DIR=${HOST_INSTALL}
_NJOBS=8
_OVS_DIR="${OVS_INSTALL:-ovs/install}"
_P4OVS_MODE=P4OVS
_PREFIX="install"
_RPATH=OFF
_SDE_DIR=${SDE_INSTALL}
_TGT_TYPE="DPDK"
_TOOLFILE=${CMAKE_TOOLCHAIN_FILE}
_WITH_KRNLMON=ON
_WITH_OVSP4RT=ON

_BLD_DIR=build
_OVS_BLD="ovs/build"

_do_build=1
_dry_run=0
_ovs_first=0
_ovs_last=0
_with_ovsp4rt=1

##############
# print_help #
##############

print_help() {
    echo ""
    echo "Configure and build P4 Control Plane"
    echo ""
    echo "General:"
    echo "  --dry-run        -n  Display cmake parameters and exit"
    echo "  --help           -h  Display help text and exit"
    echo ""
    echo "Paths:"
    echo "  --deps=DIR       -D  Target dependencies directory [${_DEPS_DIR}]"
    echo "  --host=DIR       -H  Host dependencies directory [${_HOST_DIR}]"
    echo "  --ovs=DIR        -O  OVS install directory [${_OVS_DIR}]"
    echo "  --prefix=DIR     -P  Install directory prefix [${_PREFIX}]"
    echo "  --sde=DIR        -S  SDE install directory [${_SDE_DIR}]"
#   echo "  --staging=DIR        Staging directory prefix [${_STAGING}]"
    echo "  --toolchain=FILE -T  CMake toolchain file"
    echo ""
    echo "Options:"
    echo "  --coverage           Instrument build to measure code coverage"
    echo "  --cxx-std=STD        C++ standard (11|14|17) [$_CXX_STD])"
    echo "  --jobs=NJOBS     -j  Number of build threads [${_NJOBS}]"
    echo "  --no-build           Configure without building"
    echo "  --no-krnlmon         Exclude Kernel Monitor"
    echo "  --no-ovs             Exclude OVS support"
    echo "  --p4ovs=MODE         Build OvS in specified P4OVS mode"
    echo "  --target=TARGET      Target to build (dpdk|es2k|tofino) [${_TGT_TYPE}]"
    echo ""
    echo "Build types:"
    echo "  --debug              Debug configuration"
    echo "  --minsize            MinSizeRel configuration"
    echo "  --reldeb             RelWithDebInfo configuration"
    echo "  --release            Release configuration"
    echo ""
    echo "P4OVS modes:"
    echo "  none                 Build OvS in non-P4 mode"
    echo "  ovsp4rt              Build OvS with ovsp4rt library"
    echo "  p4ovs                Build OvS in legacy P4 mode (default)"
    echo "  stubs                Build OVS with ovsp4rt stubs"
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
    echo "Networking-recipe options:"
    [ -n "${_GENERATOR}" ] && echo "  ${_GENERATOR}"
    [ -n "${_BUILD_TYPE}" ] && echo "  ${_BUILD_TYPE:2}"
    echo "  CMAKE_INSTALL_PREFIX=${_PREFIX}"
    [ -n "${_STAGING_PREFIX}" ] && echo "  ${_STAGING_PREFIX:2}"
    [ -n "${_TOOLCHAIN_FILE}" ] && echo "  ${_TOOLCHAIN_FILE:2}"
    [ -n "${_CXX_STD}" ] && echo "CMAKE_CXX_STANDARD=${_CXX_STD}"
    echo "  DEPEND_INSTALL_DIR=${_DEPS_DIR}"
    [ -n "${_HOST_DEPEND_DIR}" ] && echo "  ${_HOST_DEPEND_DIR:2}"
    echo "  OVS_INSTALL_DIR=${_OVS_DIR}"
    echo "  SDE_INSTALL_DIR=${_SDE_DIR}"
    [ -n "${_WITH_KRNLMON}" ] && echo "  ${_WITH_KRNLMON:2}"
    [ -n "${_WITH_OVSP4RT}" ] && echo "  ${_WITH_OVSP4RT:2}"
    [ -n "${_COVERAGE}" ] && echo "  ${_COVERAGE:2}"
    echo "  ${_SET_RPATH:2}"
    echo "  ${_TARGET_TYPE:2}"
    echo "  JOBS=${_NJOBS}"

    echo ""
    echo "OvS options:"
    [ -n "${_P4OVS_MODE}" ] && echo "  ${_P4OVS_MODE:2}"
    [ -n "${_PKG_CONFIG_PATH}" ] && echo "  PKG_CONFIG_PATH=${_PKG_CONFIG_PATH}"
    if [ ${_ovs_first} -ne 0 ]; then
        echo "  OVS will be built first"
    elif [ ${_ovs_last} -ne 0 ]; then
        echo "  OVS will be built last"
    else
        echo "  OVS will not be built"
    fi

    if [ ${_do_build} -eq 0 ]; then
        echo ""
        echo "Configure without building"
    fi

    echo ""
}

##############
# config_ovs #
##############

config_ovs() {
    if [ -n "${_PKG_CONFIG_PATH}" ]; then
        export PKG_CONFIG_PATH="${_PKG_CONFIG_PATH}"
    fi

    # shellcheck disable=SC2086
    cmake -S ovs -B ${_OVS_BLD} \
        ${_BUILD_TYPE}  \
        -DCMAKE_INSTALL_PREFIX="${_OVS_DIR}" \
        ${_TOOLCHAIN_FILE} \
        ${_P4OVS_MODE}
}

#############
# build_ovs #
#############

build_ovs() {
    cmake --build ${_OVS_BLD} -j6 -- V=0
}

#################
# config_recipe #
#################

config_recipe() {
    # shellcheck disable=SC2086
    cmake -S . -B ${_BLD_DIR} \
        ${_GENERATOR} \
        ${_BUILD_TYPE} \
        -DCMAKE_INSTALL_PREFIX="${_PREFIX}" \
        ${_STAGING_PREFIX} \
        ${_TOOLCHAIN_FILE} \
        ${_CXX_STANDARD} \
        -DDEPEND_INSTALL_DIR="${_DEPS_DIR}" \
        ${_HOST_DEPEND_DIR} \
        -DOVS_INSTALL_DIR="${_OVS_DIR}" \
        -DSDE_INSTALL_DIR="${_SDE_DIR}" \
        ${_WITH_KRNLMON} \
        ${_WITH_OVSP4RT} \
        ${_P4OVS_MODE} \
        ${_COVERAGE} \
        ${_SET_RPATH} \
        ${_TARGET_TYPE}
}

################
# build_recipe #
################

build_recipe() {
    [ ${_do_build} -eq 0 ] && return
    cmake --build ${_BLD_DIR} "-j${_NJOBS}" --target install
}

######################
# Parse command line #
######################

SHORTOPTS=D:H:O:P:S:T:
SHORTOPTS=${SHORTOPTS}hj:n

LONGOPTS=deps:,hostdeps:,ovs:,prefix:,sde:,toolchain:
LONGOPTS=${LONGOPTS},cxx-std:,jobs:,p4ovs:,staging:,target:
LONGOPTS=${LONGOPTS},debug,release,minsize,reldeb
LONGOPTS=${LONGOPTS},dry-run,help
LONGOPTS=${LONGOPTS},coverage,ninja,rpath
LONGOPTS=${LONGOPTS},no-build,no-krnlmon,no-ovs,no-rpath

GETOPTS=$(getopt -o ${SHORTOPTS} --long ${LONGOPTS} -- "$@")
eval set -- "${GETOPTS}"

while true ; do
    case "$1" in
    # Paths
    --deps|-D)
        _DEPS_DIR=$2
        shift 2 ;;
    --hostdeps|-H)
        _HOST_DIR=$2
        shift 2 ;;
    --ovs|-O)
        _OVS_DIR=$2
        shift 2 ;;
    --prefix|-P)
        _PREFIX=$2
        shift 2 ;;
    --sde|-S)
        _SDE_DIR=$2
        shift 2 ;;
    --staging)
        _STAGING=$2
        shift 2 ;;
    --toolchain|-T)
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
    --coverage)
        _COVERAGE="-DTEST_COVERAGE=ON"
        shift ;;
    --cxx-std)
        _CXX_STD=$2
        shift 2;;
    --dry-run|-n)
        _dry_run=1
        shift ;;
    --help|-h)
        print_help
        exit 99 ;;
    --jobs|-j)
        _NJOBS=$2
        shift 2 ;;
    --ninja)
        _GENERATOR="-G Ninja"
        shift 1 ;;
    --no-build)
        _do_build=0
        shift ;;
    --no-krnlmon)
        _WITH_KRNLMON=OFF
        shift ;;
    --no-ovs)
        _WITH_OVSP4RT=OFF
        _with_ovsp4rt=0
        shift ;;
    --no-rpath)
        _RPATH=OFF
        shift ;;
    --p4ovs)
        # convert to uppercase
        _P4OVS_MODE=${2^^}
        shift 2 ;;
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
        echo "Invalid parameter: $1"
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

config_legacy_mode() {
    _ovs_first=1
}

config_non_legacy_mode() {
    _ovs_last=1
    local _PKG_CONFIG_DIR
    _PKG_CONFIG_DIR="$(realpath -m "${_PREFIX}"/lib/pkgconfig)"
    _PKG_CONFIG_PATH="${PKG_CONFIG_PATH}:${_PKG_CONFIG_DIR}"
}

if [ ${_with_ovsp4rt} -eq 0 ]; then
    _P4OVS_MODE=NONE
elif [ "${_P4OVS_MODE}" == "NONE" ]; then
    _WITH_OVSP4RT=OFF
elif [ "${_P4OVS_MODE}" == "P4OVS" ]; then
    config_legacy_mode
elif [ "${_P4OVS_MODE}" == "OVSP4RT" ]; then
    config_non_legacy_mode
elif [ "${_P4OVS_MODE}" == "STUBS" ]; then
    config_non_legacy_mode
else
    echo "Unknown P4OVS MODE: '${_P4OVS_MODE}'"
    exit 1
fi

[ -n "${_BLD_TYPE}" ] && _BUILD_TYPE="-DCMAKE_BUILD_TYPE=${_BLD_TYPE}"
[ -n "${_CXX_STD}" ] && _CXX_STANDARD="-DCMAKE_CXX_STANDARD=${_CXX_STD}"
[ -n "${_HOST_DIR}" ] && _HOST_DEPEND_DIR="-DHOST_DEPEND_DIR=${_HOST_DIR}"
[ -n "${_P4OVS_MODE}" ] && _P4OVS_MODE="-DP4OVS_MODE=${_P4OVS_MODE}"
[ -n "${_RPATH}" ] && _SET_RPATH="-DSET_RPATH=${_RPATH}"
[ -n "${_STAGING}" ] && _STAGING_PREFIX="-DCMAKE_STAGING_PREFIX=${_STAGING}"
[ -n "${_TGT_TYPE}" ] && _TARGET_TYPE="-DTDI_TARGET=${_TGT_TYPE}"
[ -n "${_TOOLFILE}" ] && _TOOLCHAIN_FILE="-DCMAKE_TOOLCHAIN_FILE=${_TOOLFILE}"
[ -n "${_WITH_KRNLMON}" ] && _WITH_KRNLMON="-DWITH_KRNLMON=${_WITH_KRNLMON}"
[ -n "${_WITH_OVSP4RT}" ] && _WITH_OVSP4RT="-DWITH_OVSP4RT=${_WITH_OVSP4RT}"

# Show parameters if this is a dry run
if [ ${_dry_run} -ne 0 ]; then
    print_cmake_params
    exit 0
fi

################
# Do the build #
################

# Build OVS before recipe (legacy mode)
if [ ${_ovs_first} -ne 0 ]; then
    config_ovs
    build_ovs
fi

# Build networking recipe
config_recipe
build_recipe

# Build OVS after recipe
if [ ${_ovs_last} -ne 0 ]; then
    config_ovs
    if [ ${_do_build} -ne 0 ]; then
        build_ovs
    fi
fi
