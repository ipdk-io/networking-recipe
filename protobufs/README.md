# Building the P4Runtime Protobufs

This document explains how to generate protocol buffers for the custom
version of P4Runtime used by P4 Control Plane.

You can do this from the main directory, or from the `protobufs` subdirectory.

## Prerequisites

The following packages must be installed:

- Stratum dependencies
- Python 3.8 or above
- Python dependencies (`requirements.txt`)

If you wish to generate protobufs for Go, follow the instructions in the
[Quick start](https://grpc.io/docs/languages/go/quickstart/) guide
to install Go and the gRPC plugins.

## Main directory

To build the P4Runtime protobufs in the main directory, you must first
configure cmake.

If you've done a recent build, there should already be a configuration.

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
Successfully built p4runtime-2023.11.0.tar.gz and p4runtime-2023.11.0-py3-none-any.whl
[ 66%] Built target py-wheel
[100%] Generating Go tarball
[100%] Built target go-tarball
Install the project...
-- Install configuration: ""
-- Installing: /home/samwise/work/latest/install/share/p4runtime/p4runtime-cpp-2023.11.0.tar.gz
-- Installing: /home/samwise/work/latest/install/share/p4runtime/p4runtime-go-2023.11.0.tar.gz
-- Up-to-date: /home/samwise/work/latest/install/share/p4runtime
-- Installing: /home/samwise/work/latest/install/share/p4runtime/p4runtime-2023.11.0-py3-none-any.whl
-- Installing: /home/samwise/work/latest/install/share/p4runtime/p4runtime-2023.11.0.tar.gz
[100%] Completed 'protobufs'
[100%] Built target protobufs
```

The tarballs and the Python wheel are placed in the `share/p4runtime` folder
in the install tree.

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
