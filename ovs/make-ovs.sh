#!/bin/bash

_P4OVS_MODE=P4OVS
_OVS_PREFIX=install

print_help() {
    echo ""
    echo "Build Open vSwitch (OvS)"
    echo ""
    echo "Options:"
    echo "  --help          -h  Display help text and exit"
    echo "  --p4ovs=MODE        Build OvS in specified P4OVS mode"
    echo "  --prefix=DIR    -P  Install directory prefix [${_OVS_PREFIX}]"
    echo ""
    echo "P4OVS modes:"
    echo "  none                Build OvS in non-P4 mode"
    echo "  ovsp4rt             Build OvS with ovsp4rt library"
    echo "  p4ovs               Build OvS in legacy P4 mode (default)"
    echo "  stubs               Build OVS with ovsp4rt stubs"
    echo ""
}

SHORTOPTS=hP
LONGOPTS=help,p4ovs:,prefix:

GETOPTS=$(getopt -o ${SHORTOPTS} --long ${LONGOPTS} -- "$@")
eval set -- "${GETOPTS}"

while true; do
    case "$1" in
    --help|-h)
        print_help
        exit 99 ;;
    --p4ovs)
        # convert to uppercase
        _P4OVS_MODE=${2^^}
        shift 2 ;;
    --prefix|-P)
        _OVS_PREFIX=$2
        shift 2 ;;
    --)
        shift
        break ;;
    *)
        echo "Invalid parameter: $1"
        exit 1 ;;
    esac
done

rm -fr build install

# ${_OVS_BLD} ${_OVS_DIR} ${_TOOLCHAIN_FILE}
cmake -S . -B build \
    -DCMAKE_INSTALL_PREFIX="${_OVS_PREFIX}" \
    -DP4OVS_MODE="${_P4OVS_MODE}"

cmake --build build -j6 -- V=0
