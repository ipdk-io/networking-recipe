#!/bin/bash
#
# Copyright 2023 Intel Corporation
# SPDX-License-Identifier: Apache 2.0
#
# Sample script to configure the non-OVS portion of the P4 Control Plane
# to cross-compile for the ES2K ACC platform.
#

if [ -z "${SDKTARGETSYSROOT}" ]; then
    echo ""
    echo "Error: SDKTARGETSYSROOT is not defined!"
    echo "Did you forget to source the environment variables?"
    echo ""
    exit 1
fi

_SYSROOT=${SDKTARGETSYSROOT}

# Default values
_BLD_DIR=build
_BLD_TYPE=RelWithDebInfo
_DEPS_DIR="${DEPEND_INSTALL:-//opt/deps}"
_DRY_RUN=false
_HOST_DIR="${HOST_INSTALL:-setup/host-deps}"
_OVS_DIR="${OVS_INSTALL:-//opt/ovs}"
_PREFIX=install
_SDE_DIR="${SDE_INSTALL:-//opt/p4sde}"
_TOOLFILE=${CMAKE_TOOLCHAIN_FILE}

# Displays help text
print_help() {
    echo ""
    echo "Configure recipe build"
    echo ""
    echo "Options:"
    echo "  --build=DIR      -B  Build directory path [${_BLD_DIR}]"
    echo "  --deps=DIR*      -D  Target dependencies directory [${_DEPS_DIR}]"
    echo "  --dry-run        -n  Display cmake parameter values and exit"
    echo "  --help           -h  Display this help text"
    echo "  --hostdeps=DIR*  -H  Host dependencies directory [${_HOST_DIR}]"
    echo "  --ovs=DIR*       -O  OVS install directory [${_OVS_DIR}]"
    echo "  --no-krnlmon         Exclude Kernel Monitor"
    echo "  --no-ovs             Exclude OVS support"
    echo "  --prefix=DIR*    -P  Install directory prefix [${_PREFIX}]"
    echo "  --sde=DIR*       -S  SDE install directory [${_SDE_DIR}]"
    echo "  --toolchain=FILE -T  CMake toolchain file"
    echo ""
    echo "* '//' at the beginning of the directory path will be replaced"
    echo "  with the sysroot directory path."
    echo ""
    echo "Environment variables:"
    echo "  CMAKE_TOOLCHAIN_FILE - Default toolchain file"
    echo "  DEPEND_INSTALL - Default target dependencies directory"
    echo "  HOST_INSTALL - Default host dependencies directory"
    echo "  OVS_INSTALL - Default OVS install directory"
    echo "  SDE_INSTALL - Default SDE install directory"
    echo "  SDKTARGETSYSROOT - sysroot directory"
    echo ""
}

# Parse options
SHORTOPTS=B:D:H:O:P:S:T:hn
LONGOPTS=build:,deps:,dry-run,help,hostdeps:,ovs:,prefix:,sde:,toolchain:
LONGOPTS=${LONGOPTS},no-krnlmon,no-ovs,rfs-enable

eval set -- `getopt -o ${SHORTOPTS} --long ${LONGOPTS} -- "$@"`

while true ; do
    case "$1" in
    -B|--build)
        echo "Build directory: $2"
        _BLD_DIR=$2
        shift 2 ;;
    -D|--deps)
        _DEPS_DIR=$2
        shift 2 ;;
    -n|--dry-run)
        _DRY_RUN=true
        shift 1 ;;
    -h|--help)
        print_help
        exit 99 ;;
    -H|--hostdeps)
        _HOST_DIR=$2
        shift 2 ;;
    --no-krnlmon)
        _WITH_KRNLMON=FALSE
        shift 1 ;;
    --no-ovs)
        _WITH_OVSP4RT=FALSE
        shift 1 ;;
    -O|--ovs)
        _OVS_DIR=$2
        shift 2 ;;
    -P|--prefix)
        echo "Install prefix: $2"
        _PREFIX=$2
        shift 2 ;;
    -S|--sde)
        _SDE_DIR=$2
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

# Substitute ${_SYSROOT}/ for // prefix
[ "${_DEPS_DIR:0:2}" = "//" ] && _DEPS_DIR=${_SYSROOT}/${_DEPS_DIR:2}
[ "${_HOST_DIR:0:2}" = "//" ] && _HOST_DIR=${_SYSROOT}/${_HOST_DIR:2}
[ "${_OVS_DIR:0:2}" = "//" ] && _OVS_DIR=${_SYSROOT}/${_OVS_DIR:2}
[ "${_PREFIX:0:2}" = "//" ] && _PREFIX=${_SYSROOT}/${_PREFIX:2}
[ "${_SDE_DIR:0:2}" = "//" ] && _SDE_DIR=${_SYSROOT}/${_SDE_DIR:2}

# Expand WITH_KRNLMON and WITH_OVSP4RT if not empty
[ -n "${_WITH_KRNLMON}" ] && _WITH_KRNLMON=-DWITH_KRNLMON=${_WITH_KRNLMON}
[ -n "${_WITH_OVSP4RT}" ] && _WITH_OVSP4RT=-DWITH_OVSP4RT=${_WITH_OVSP4RT}

if [ "${_DRY_RUN}" = "true" ]; then
    echo ""
    echo "CMAKE_BUILD_TYPE=${_BLD_TYPE}"
    echo "CMAKE_INSTALL_PREFIX=${_PREFIX}"
    echo "CMAKE_TOOLCHAIN_FILE=${_TOOLFILE}"
    echo "DEPEND_INSTALL_DIR=${_DEPS_DIR}"
    echo "HOST_DEPEND_DIR=${_HOST_DIR}"
    echo "OVS_INSTALL_DIR=${_OVS_DIR}"
    echo "SDE_INSTALL_DIR=${_SDE_DIR}"
    [ -n "${_WITH_KRNLMON}" ] && echo "${_WITH_KRNLMON:2}"
    [ -n "${_WITH_OVSP4RT}" ] && echo "${_WITH_OVSP4RT:2}"
    echo ""
    exit 0
fi

rm -fr ${_BLD_DIR}

cmake -S . -B ${_BLD_DIR} \
    -DCMAKE_BUILD_TYPE=${_BLD_TYPE} \
    -DCMAKE_INSTALL_PREFIX=${_PREFIX} \
    -DCMAKE_TOOLCHAIN_FILE=${_TOOLFILE} \
    -DDEPEND_INSTALL_DIR=${_DEPS_DIR} \
    -DHOST_DEPEND_DIR=${_HOST_DIR} \
    -DOVS_INSTALL_DIR=${_OVS_DIR} \
    -DSDE_INSTALL_DIR=${_SDE_DIR} \
    ${_WITH_KRNLMON} ${_WITH_OVSP4RT} \
    -DSET_RPATH=TRUE \
    -DES2K_TARGET=ON
