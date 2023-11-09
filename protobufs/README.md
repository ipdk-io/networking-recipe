# Building the P4Runtime Protobufs

This document explains how to generate protocol buffers for the custom
version of P4Runtime used by P4 Control Plane.

You can do this from the main directory, or from the `protobufs` subdirectory.

## Prerequisites

The following packages must be installed:

- Stratum dependencies

- Python

If you wish to generate protobufs for Go, you should also follow the
instructions in the
gRPC [Quick start](https://grpc.io/docs/languages/go/quickstart/) guide
to install Go and the gRPC plugins.

## Main directory

To build the P4Runtime protobufs in the main directory, you must first
configure cmake.

If you've done a recent build, you should already be configured.

Otherwise, you can use `make-all.sh` with the `--no-build` option.

First, make sure the `DEPEND_INSTALL` environment variable is defined.

To configure the build:

```bash
./make-all.sh --target=es2k --no-build --no-ovs
```

To build the protobufs:

```bash
cmake --build build --target protobufs
```

The console output will look like this:

```text
Scanning dependencies of target protobufs
[ 25%] Creating directories for 'protobufs'
[ 25%] No download step for 'protobufs'
[ 25%] No patch step for 'protobufs'
[ 50%] No update step for 'protobufs'
[ 50%] Performing configure step for 'protobufs'
   .
   .
[ 33%] Generating C++ tarball
[ 33%] Built target cpp-tarball
[ 66%] Generating Go tarball
[ 66%] Built target go-tarball
[100%] Generating Python tarball
[100%] Built target py-tarball
Install the project...
-- Install configuration: ""
-- Installing: /home/frodo/work/latest/install/share/stratum/protobufs/cpp_out_protos.tar.gz
-- Installing: /home/frodo/work/latest/install/share/stratum/protobufs/py_out_protos.tar.gz
-- Installing: /home/frodo/work/latest/install/share/stratum/protobufs/go_out_protos.tar.gz
[100%] Completed 'protobufs'
[100%] Built target protobufs  
```

The Go tarball will be omitted if golang is not installed.

## `protobufs` directory

The P4Runtime protocol buffers can also be built in the `protobufs` directory.

Change to the `protobufs` directory.

Make sure the `DEPEND_INSTALL` environment variable is set.

Issue the following commands:

```bash
cmake -B build
cmake --build build
cmake --install build --prefix install
```

The tarballs will be installed in the local `install` directory.
