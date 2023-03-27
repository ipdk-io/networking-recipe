#!/bin/bash

# Sample script to configure and build the subset of the dependency
# libraries needed to compile the Protobuf files on the development
# host when cross-compiling for the ES2K ACC platform.

# The Protobuf compiler and grpc plugins run natively on the development
# host. They generate C++ source and header files that can be compiled
# to run natively on the host or cross-compiled to run on the target
# platform.

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
_CFG_ONLY=false
_DRY_RUN=false
_PREFIX=./host-deps
_NJOBS=6
_SCOPE=minimal

print_help() {
    echo ""
    echo "Build host dependency libraries"
    echo ""
    echo "Options:"
    echo "  --build=DIR     -B  Build directory path (Default: ${_BLD_DIR})"
    echo "  --config            Only perform configuration step (Default: ${_CFG_ONLY})"
    echo "  --dry-run       -n  Display cmake parameters and exit (Default: false)"
    echo "  --force         -f  Specify -f when patching (Default: false)"
    echo "  --full              Build all dependency libraries (Default: ${_SCOPE})"
    echo "  --jobs=NJOBS    -j  Number of build threads (Default: ${_NJOBS})"
    echo "  --minimal           Build required host dependencies only (Default: ${_SCOPE})"
    echo "  --no-download       Do not download repositories (Default: false)"
    echo "  --prefix=DIR    -P  Install directory path (Default: ${_PREFIX})"
    echo "  --sudo              Use sudo when installing (Default: false)"
    echo ""
}

# Parse options
SHORTOPTS=BPfhj:n
LONGOPTS=build:,config,dry-run,force,full,help,jobs:,minimal,no-download,prefix:,sudo

eval set -- `getopt -o ${SHORTOPTS} --long ${LONGOPTS} -- "$@"`

while true ; do
    case "$1" in
    -B|--build)
        echo "Build directory: $2"
        _BLD_DIR=$2
        shift 2 ;;
    --config)
        _CFG_ONLY=true
        shift ;;
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
    --no-download)
        _DOWNLOAD="-DDOWNLOAD=FALSE"
        shift ;;
    -P|--prefix)
        echo "Install directory: $2"
        _PREFIX=$2
        shift 2 ;;
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
    echo "Configure options:"
    echo "  CMAKE_INSTALL_PREFIX=${_PREFIX}"
    [ -n "${_DOWNLOAD}" ] && echo "  ${_DOWNLOAD:2}"
    [ -n "${_FORCE_PATCH}" ] && echo "  ${_FORCE_PATCH:2}"
    [ -n "${_ON_DEMAND}" ] && echo "  ${_ON_DEMAND:2}"
    [ -n "${_USE_SUDO}" ] && echo "  ${_USE_SUDO:2}"
    echo ""
    if [ "${_CFG_ONLY}" = "true" ]; then
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
    ${_ON_DEMAND} ${_DOWNLOAD} ${_FORCE_PATCH} ${_USE_SUDO}

if [ "${_CFG_ONLY}" = "false" ]; then
    cmake --build ${_BLD_DIR} -j${_NJOBS} ${_TARGET}
fi
