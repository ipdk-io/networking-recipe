#!/bin/bash

# Sample script to configure and build the subset of the dependency
# libraries needed to compile the Protobuf files on the development
# host when cross-compiling for the ES2K ACC platform.

# The Protobuf compiler and grpc plugins run natively on the development
# host. They generate C++ source and header files that can be compiled
# to run natively on the host or cross-compiled to run on the target
# platform.

check_environment() {
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
}

# Default values
_DEFAULT_BUILD=build
_DEFAULT_INSTALL=host-deps
_DEFAULT_JOBS=6
_DEFAULT_SCOPE=minimal

print_help() {
    echo ""
    echo "Build host dependency libraries"
    echo ""
    echo "Options:"
    echo "  --build=PATH    Build directory path (Default: ${_DEFAULT_BUILD})"
    echo "  --config        Only perform configuration step"
    echo "  --force         Specify -f when patching"
    echo "  --full          Build all dependency libraries (Default: ${_DEFAULT_SCOPE})"
    echo "  --install=PATH  Install directory path (Default: ${_DEFAULT_INSTALL})"
    echo "  --jobs=NJOBS    Number of build threads (Default: ${_DEFAULT_JOBS})"
    echo "  --minimal       Build required host dependencies only (Default: ${_DEFAULT_SCOPE})"
    echo "  --no-download   Do not download repositories"
    echo ""
    echo "Synonyms:"
    echo "  -jJNOBS         Same as --jobs=NJOBS"
    echo "  --prefix=PATH   Same as --install=PATH"
    echo ""
}

# Initial values
_BUILD_DIR=${_DEFAULT_BUILD}
_CONFIG_ONLY=false
_INSTALL_DIR=${_DEFAULT_INSTALL}
_JOBS=-j${_DEFAULT_JOBS}
_SCOPE=minimal

# Parse options
SHORTOPTS=j:
LONGOPTS=build:,config,force,full,help,install:,jobs:,minimal,no-download,prefix:

eval set -- `getopt -o ${SHORTOPTS} --long ${LONGOPTS} -- "$@"`

while true ; do
    case "$1" in
    --build)
        echo "Build directory: $2"
        _BUILD_DIR=$2
        shift 2 ;;
    --config)
        _CONFIG_ONLY=true
        shift ;;
    --force)
        _FORCE_PATCH="-DFORCE_PATCH=TRUE"
        shift ;;
    --full)
        _SCOPE=full
        shift ;;
    --install|--prefix)
        echo "Install directory: $2"
        _INSTALL_DIR=$2
        shift 2 ;;
    --help)
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
    --)
        shift
        break ;;
    *)
        echo "Invalid parameter: $1"
        exit 1 ;;
    esac
done

check_environment

if [ "${_SCOPE}" = minimal ]; then
    _ON_DEMAND="-DON_DEMAND=TRUE"
    _BUILD_TARGET="--target grpc"
fi
echo "Performing ${_SCOPE} build"

rm -fr ${_BUILD_DIR} ${_INSTALL_DIR}

cmake -S . -B ${_BUILD_DIR} \
    -DCMAKE_INSTALL_PREFIX=${_INSTALL_DIR} \
    ${_ON_DEMAND} ${_DOWNLOAD} ${_FORCE_PATCH}

if [ "${_CONFIG_ONLY}" = "false" ]; then
    cmake --build ${_BUILD_DIR} ${_JOBS} ${_BUILD_TARGET}
fi
