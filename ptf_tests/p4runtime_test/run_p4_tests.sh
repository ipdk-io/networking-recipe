#!/bin/bash
# Start running PTF tests associated with a P4 program
# TODO: remove tofino/barefoot references
set -x
function print_help() {
  echo "USAGE: $(basename ""$0"") {-p <...> | -t <...>} [OPTIONS -- PTF_OPTIONS]"
  echo "Options for running PTF tests:"
  echo "  -p <p4_program_name>"
  echo "    Run PTF tests associated with P4 program"
  echo "  -t TEST_DIR"
  echo "    TEST_DIR contains test cases executed by PTF."
  echo "  -f PORTINFO_FILE"
  echo "    Read port to veth mapping information from PORTINFO_FILE"
  echo "  -s TEST_SUITE"
  echo "    Name of the test suite to execute passed to PTF"
  echo "  -c TEST_CASE"
  echo "    Name of the test case to execute passed to PTF"
  echo "  --arch <ARCHITECTURE>"
  echo "    Architecture (Tofino, Tofino2, Tofino3, etc.)"
  echo "  --target <TARGET>"
  echo "    Target (asic-model or hw)"
  echo "  --no-veth"
  echo "    Skip veth setup and special CPU port setup"
  echo "  --config-file"
  echo "    PTF config-file"
  echo "  --ip <target switch IP address>"
  echo "    Target switch's IP address, localhost by default"
  echo "  --thrift-server <thrift_server_address>"
  echo "    Depreciated, use --ip"
  echo "  --setup"
  echo "    Run test setup only"
  echo "  --cleanup"
  echo "    Run test cleanup only"
  echo "  --traffic-gen <traffic_generator>"
  echo "    Traffic Generator (ixia, scapy)"
  echo "  --socket-recv-size <socket bytes size>"
  echo "    socket buffer size for ptf scapy verification "
  echo "  --failfast"
  echo "    Fail and exit on first failure"
  echo "  --test-params <ptf_test_params>"
  echo "    PTF test params as a string, e.g. arch='Tofino';target='hw';"
  echo "  --with-p4c <compiler version>"
  echo "    P4C compiler version, e.g. v6"
  echo "  --gen-xml-output <gen_xml_output>"
  echo "    Specify this flag to generate xml output for tests"
  echo "  --p4info"
  echo "    Path to P4Info Protobuf text file for P4Runtime tests"
  echo "  --profile"
  echo "    Enable Python profiling"
  echo "  -h"
  echo "    Print this message"
  exit 0
}

trap 'exit' ERR

if [ -z $WORKSPACE ]; then
  WORKSPACE=`pwd`
fi
echo "Using workspace ${WORKSPACE}"

opts=`getopt -o p:t:f:s:c:m:h --long reboot: --long config-file: --long arch: --long target: --long num-pipes: --long drv-test-info: --long failfast --long seed: --long no-veth --long thrift-server: --long setup --long cleanup --long traffic-gen: --long socket-recv-size: --long no-status-srv --long status-port: --long ip: --long test-params: --long port-mode: --long with-p4c: --long gen-xml-output --long db-prefix: --long p4info: --long p4bin: --long default-negative-timeout: --long default-timeout: --long profile -- "$@"`

if [ $? != 0 ]; then
  exit 1
fi
eval set -- "$opts"

# P4 program name
P4_NAME=""
# json file specifying model port to veth mapping info
PORTINFO=None
CONFIG_FILE='cfg'
HELP=false
SETUP=""
CLEANUP=""
ARCH="DPDK"
TARGET="asic-model"
TEST_PARAMS=""
COMPILER_VERSION="v5"
P4INFO_PATH=""
PROFILE=""

while true; do
    case "$1" in
      -p) P4_NAME=$2; shift 2;;
      -t) TEST_DIR="$2"; shift 2;;
      -s) TEST_SUITE="$2"; shift 2;;
      -f) PORTINFO=$2; shift 2;;
      -h) HELP=true; shift 1;;
      --no-veth) NO_VETH=true; shift 1;;
      --config-file) CONFIG_FILE=$2; shift 2;;
      --arch) ARCH=$2; shift 2;;
      --target) TARGET=$2; shift 2;;
      --ip) TARGET_IP=$2; shift 2;;
      --p4bin) P4BIN_PATH=$2; shift 2;;
      --test-params) TEST_PARAMS=$2; shift 2;;
      --with-p4c) COMPILER_VERSION=$2; shift 2;;
      --p4info) P4INFO_PATH=$2; shift 2;;
      --p4bin) P4BIN_PATH=$2; shift 2;;
      --grpc-server) TARGET_IP=$2; shift 2;;
      --) shift; break;;
    esac
done

ARCH=`echo $ARCH | tr '[:upper:]' '[:lower:]'`
case "$ARCH" in
  "tofino") CPUPORT=64;;
  "tofino2") CPUPORT=2;;
  "tofino3") CPUPORT=2;;
esac

if [ $HELP = true ] || ( [ -z $P4_NAME ] && [ -z $TEST_DIR ] ); then
  print_help
fi

if [ $NO_VETH = true ]; then
  CPUPORT=None
  CPUVETH=None
fi


[ -d "$TEST_DIR" ] || exit "Test directory $TEST_DIR directory does not exist"

echo "Using TEST_DIR ${TEST_DIR}"

if [[ $PORTINFO != None ]]; then
  CPUPORT=None
  CPUVETH=None
fi

export TESTDIR=$TEST_DIR
export PORT_INFO=$TESTDIR/port_info.json
PYTHON_VER=`python --version 2>&1 | awk {'print $2'} | awk -F"." {'print $1"."$2'}`

export PATH=$PATH
export LD_LIBRARY_PATH=/usr/local/lib:$LD_LIBRARY_PATH
PYTHONPATH=/usr/local/lib/python3.6/dist-packages:/usr/local/lib/python3.8/dist-packages:$PYTHONPATH

echo "Using PATH ${PATH}"
echo "Using LD_LIBRARY_PATH ${LD_LIBRARY_PATH}"
echo "Using PYTHONPATH ${PYTHONPATH}"

echo "Reboot type is $REBOOT_TYPE"

# Setup veth interfaces
if [ $NO_VETH = false ]; then
  echo "Setting up veth interfaces"
  sudo env "PATH=$PATH" $BF_SDE_INSTALL_DIR/bin/veth_setup.sh
fi
echo "Arch is $ARCH"
echo "Target is $TARGET"

if [ -z "$P4INFO_PATH" ]; then
    if [ -n "$P4_NAME" ]; then
        p4info=$BF_SDE_INSTALL_DIR/share/${ARCH}pd/$P4_NAME/p4info.pb.txt
        if [ -f $p4info ]; then
            P4INFO_PATH=$p4info
        fi
    fi
fi
if [ "$P4INFO_PATH" != "" ]; then
    if [ "$TEST_PARAMS" != "" ]; then
        TEST_PARAMS="$TEST_PARAMS;p4info='$P4INFO_PATH'"
    else
        TEST_PARAMS="p4info='$P4INFO_PATH'"
    fi
fi
if [ "$TARGET_IP" != "" ]; then
    if [ "$TEST_PARAMS" != "" ]; then
         TEST_PARAMS="$TEST_PARAMS;ip='$TARGET_IP'"
     else
         TEST_PARAMS="ip='$TARGET_IP'"
     fi
fi
if [ "$P4BIN_PATH" != "" ]; then
     if [ "$TEST_PARAMS" != "" ]; then
          TEST_PARAMS="$TEST_PARAMS;p4bin='$P4BIN_PATH'"
      else
          TEST_PARAMS="p4bin='$P4BIN_PATH'"
      fi
fi


TEST_PARAMS_STR=""
if [ "$TEST_PARAMS" != "" ]; then
    TEST_PARAMS_STR="--test-params $TEST_PARAMS"
fi

#Run PTF tests
sudo env -u http_proxy -u socks_proxy "PATH=$PATH" "PYTHONPATH=$PYTHONPATH"
 python3 \
    $TESTDIR/../run_ptf_tests.py \
    --test-dir $TEST_DIR \
    $TEST_SUITE \
    $TEST_CASE \
    --arch $ARCH \
    --target $TARGET \
    --port-info $PORT_INFO \
    $PTF_BINARY \
    $PROFILE \
    $DRV_TEST_SEED $FAILFAST $SETUP $CLEANUP $TEST_PARAMS_STR $DB_PREFIX $@
