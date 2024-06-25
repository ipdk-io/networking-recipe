#!/bin/bash

_P4MODE=P4OVS

print_help() {
    echo ""
    echo "Build Open vSwitch (OvS)"
    echo ""
    echo "Options:"
    echo "  --help        Display help text and exit"
    echo "  --none        Build OvS without P4 support"
    echo "  --ovsp4rt     Build OvS with P4 support (ovsp4rt library)"
    echo "  --p4ovs       Build OvS with P4 support (legacy mode)"
    echo "  --stubs       Build OvS with P4 support (stubs library)"
    echo ""
}

SHORTOPTS=h
LONGOPTS=ovsp4rt,p4ovs,stubs,none,help

GETOPTS=$(getopt -o ${SHORTOPTS} --long ${LONGOPTS} -- "$@")
eval set -- "${GETOPTS}"

while true; do
    case "$1" in
    --help|-h)
	print_help
	exit 99 ;;
    --none)
	_P4MODE=
	shift ;;
    --ovsp4rt)
	_P4MODE=OVSP4RT
	shift ;;
    --p4ovs)
	_P4MODE=P4OVS
	shift ;;
    --stubs)
	_P4MODE=STUBS
	shift ;;
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
cmake -S . -B build -DCMAKE_INSTALL_PREFIX=install -DP4MODE=${_P4MODE}
cmake --build build -j6 -- V=0
