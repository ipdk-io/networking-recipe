#!/bin/bash

# Sample script to configure and build the subset of the dependency
# libraries needed to compile the Protobuf files on the development
# host when cross-compiling for the ES2K ACC platform.

# The Protobuf compiler and grpc plugins run natively on the development
# host. They generate C++ source and header files that can be compiled
# to run natively on the host or cross-compiled to run on the target
# platform.

# Abort on error.
set -e

if [ -n "${SDKTARGETSYSROOT}" ]; then
    echo ""
    echo "-----------------------------------------------------"
    echo "Error: SDKTARGETSYSROOT is defined!"
    echo "The host dependencies must be built WITHOUT sourcing"
    echo "the cross-compile environment variables."
    echo "-----------------------------------------------------"
    echo ""
    exit 1
fi

# Default values
_BLD_DIR=build
_DO_BUILD=true
_DRY_RUN=false
_PREFIX=./host-deps
_NJOBS=8
_SCOPE=minimal

print_help() {
    echo ""
    echo "Build host dependency libraries"
    echo ""
    echo "Paths:"
    echo "  --build=DIR     -B  Build directory path (Default: ${_BLD_DIR})"
    echo "  --prefix=DIR    -P  Install directory path (Default: ${_PREFIX})"
    echo ""
    echo "Options:"
    echo "  --cxx=VERSION       CXX_STANDARD to build dependencies (Default: empty)"
    echo "  --dry-run       -n  Display cmake parameters and exit (Default: false)"
    echo "  --force         -f  Specify -f when patching (Default: false)"
    echo "  --full              Build all dependency libraries (Default: ${_SCOPE})"
    echo "  --jobs=NJOBS    -j  Number of build threads (Default: ${_NJOBS})"
    echo "  --minimal           Build required host dependencies only (Default: ${_SCOPE})"
    echo "  --no-build          Configure without building"
    echo "  --no-download       Do not download repositories (Default: false)"
    echo "  --sudo              Use sudo when installing (Default: false)"
    echo ""
}

# Parse options
SHORTOPTS=B:P:fhj:n
LONGOPTS=build:,cxx:,jobs:,prefix:
LONGOPTS=${LONGOPTS},dry-run,force,full,help,minimal,no-build,no-download,sudo

eval set -- `getopt -o ${SHORTOPTS} --long ${LONGOPTS} -- "$@"`

while true ; do
    case "$1" in
    # Paths
    -B|--build)
        _BLD_DIR=$2
        shift 2 ;;
    -P|--prefix)
        _PREFIX=$2
        shift 2 ;;
    # Options
    --cxx)
        _CXX_STANDARD_OPTION="-DCXX_STANDARD=$2"
        shift 2 ;;
    -n|--dry-run)
        _DRY_RUN=true
        shift ;;
    -f|--force)
        _FORCE_PATCH="-DFORCE_PATCH=TRUE"
        shift ;;
    --full)
        _SCOPE=full
        shift ;;
    -h|--help)
        print_help
        exit 99 ;;
    -j|--jobs)
        _JOBS=-j$2
        shift 2 ;;
    --minimal)
        _SCOPE=minimal
        shift ;;
    --no-build)
        _DO_BUILD=false
        shift ;;
    --no-download)
        _DOWNLOAD="-DDOWNLOAD=FALSE"
        shift ;;
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

if [ "${_SCOPE}" = minimal ]; then
    _ON_DEMAND="-DON_DEMAND=TRUE"
    _TARGET="--target grpc"
fi

if [ "${_DRY_RUN}" = "true" ]; then
    echo ""
    echo "Config options:"
    echo "  CMAKE_INSTALL_PREFIX=${_PREFIX}"
    [ -n "${_CXX_STANDARD_OPTION}" ] && echo "  ${_CXX_STANDARD_OPTION:2}"
    [ -n "${_DOWNLOAD}" ] && echo "  ${_DOWNLOAD:2}"
    [ -n "${_FORCE_PATCH}" ] && echo "  ${_FORCE_PATCH:2}"
    [ -n "${_ON_DEMAND}" ] && echo "  ${_ON_DEMAND:2}"
    [ -n "${_USE_SUDO}" ] && echo "  ${_USE_SUDO:2}"
    echo ""
    if [ "${_DO_BUILD}" = "false" ]; then
        echo "Configure only (${_SCOPE} build)"
    else
        echo "Build options:"
        echo "  -j${_NJOBS}"
        [ -n "${_TARGET}" ] && echo "  ${_TARGET}"
        echo ""
        echo "Will perform a ${_SCOPE} build"
    fi
    echo ""
    exit 0
fi

rm -fr ${_BLD_DIR} ${_PREFIX}

cmake -S . -B ${_BLD_DIR} \
    -DCMAKE_INSTALL_PREFIX=${_PREFIX} \
    ${_CXX_STANDARD_OPTION} \
    ${_ON_DEMAND} ${_DOWNLOAD} ${_FORCE_PATCH} ${_USE_SUDO}

if [ "${_DO_BUILD}" = "true" ]; then
    cmake --build ${_BLD_DIR} -j${_NJOBS} ${_TARGET}
fi
