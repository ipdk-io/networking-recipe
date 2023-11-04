# Derived from github.com/p4lang/p4runtime/codegen/compile_protos.sh
# and used as the basis of the cmake build.
#
# SPDX-License-Identifier: Apache-2.0
#

PROTOC=/opt/deps/bin/protoc
GRPC_CPP_PLUGIN=$(realpath /opt/deps/bin/grpc_cpp_plugin)
GRPC_PY_PLUGIN=$(realpath /opt/deps/bin/grpc_python_plugin)

PROTO_DIR="../stratum/p4runtime/proto"
GOOGLE_PROTO_DIR="../stratum/googleapis"

PROTOS="\
$PROTO_DIR/p4/v1/p4data.proto \
$PROTO_DIR/p4/v1/p4runtime.proto \
$PROTO_DIR/p4/config/v1/p4info.proto \
$PROTO_DIR/p4/config/v1/p4types.proto \
$GOOGLE_PROTO_DIR/google/rpc/status.proto \
$GOOGLE_PROTO_DIR/google/rpc/code.proto"

PROTOFLAGS="-I$GOOGLE_PROTO_DIR -I$PROTO_DIR"

BUILD_DIR="build"
mkdir -p "$BUILD_DIR/cpp_out"
mkdir -p "$BUILD_DIR/grpc_out"
mkdir -p "$BUILD_DIR/py_out"
mkdir -p "$BUILD_DIR/go_out"

set -o xtrace
$PROTOC $PROTOS --cpp_out "$BUILD_DIR/cpp_out" $PROTOFLAGS
$PROTOC $PROTOS --grpc_out "$BUILD_DIR/grpc_out" --plugin=protoc-gen-grpc="$GRPC_CPP_PLUGIN" $PROTOFLAGS
# With the Python plugin, it seems that I need to use a single command for proto
# + grpc and that the output directory needs to be the same (because the grpc
# plugin inserts code into the proto-generated files). But maybe I am just using
# an old version of the Python plugin.
$PROTOC $PROTOS --python_out "$BUILD_DIR/py_out" $PROTOFLAGS --grpc_out "$BUILD_DIR/py_out" --plugin=protoc-gen-grpc="$GRPC_PY_PLUGIN"
$PROTOC $PROTOS --go_out="$BUILD_DIR/go_out" --go-grpc_out="$BUILD_DIR/go_out" $PROTOFLAGS

