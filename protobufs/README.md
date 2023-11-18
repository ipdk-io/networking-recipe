# Building the P4Runtime Protobufs

This document explains how to generate protocol buffers for the custom
version of P4Runtime used by P4 Control Plane.

You can do this from the main directory, or from the `protobufs` subdirectory.

## Prerequisites

The following packages must be installed:

- Stratum dependencies
- Python 3.8 or above
- Python dependencies

The Python wheel requires that the `build`, `setuptools`, and `wheel`
modules be installed. These are included in the `requirements.txt` file.

If you wish to generate protobufs for Go, follow the instructions in the
[Quick start](https://grpc.io/docs/languages/go/quickstart/) guide
to install golang and the gRPC plugins.

## Build environment

The P4Runtime protobufs are built in HOST mode. They are are not
cross-compiled, and will not build correctly in the cross-compilation
environment.

The build requires the Host (x86) dependencies, not the ACC (aarch64)
dependencies.

You may use the `DEPEND_INSTALL` environment variable
or the `DEPEND_INSTALL_DIR` cmake variable
to specify the location of the host dependencies.

If you are in the ACC build environment, see
[Escaping the ACC build environment](#escaping-the-acc-build-environment)
for a way to change the environment temporarily so you can build the
protobufs.

## Main directory

To build the P4Runtime protobufs in the main directory, you must first
configure cmake to build for the x86 host (not cross-compile for the ACC).

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

1. Change to the `protobufs` directory.

2. Remove the `build` directory, if it exists.

   ```bash
   rm -fr build
   ```

3. Issue the following commands:

   ```bash
   cmake -B build [-DDEPEND_INSTALL_DIR=<host dependencies>]
   cmake --build build
   cmake --install build --prefix <install directory>
   ```

   `<host dependencies>` is the directory containing the host dependencies
   (the Stratum dependencies compiled for the x86).

   `<install directory>` is the directory in which you are installing
   P4 Control Plane, or an alternative location (e.g. `install`).

   You may omit `DEPEND_INSTALL_DIR` if the `DEPEND_INSTALL` environment
   variable is set to the location of the host dependencies.

The tarballs and Python wheel will be installed in the `share/p4runtime`
folder under the `<install directory>`.

The Go tarball will be omitted if golang is not installed.

## Escaping the ACC build environment

As noted above, the P4Runtime protobufs must be generated in the host build
environment, not the environment used to cross-compile P4 Control Plane
for the ACC.

If you're in the ACC build environment, the following procedure may be
used to escape temporarily to the host build environment.

1. Change to the `protobufs` directory.

2. Remove the `build` directory, if it exists.

   ```bash
   rm -fr build
   ```

3. Start a subshell.

   ```bash
   bash
   ```

   This preserves your current environment.

4. Neutralize the cross-compilation environment.

   ```bash
   unset CMAKE_TOOLCHAIN_FILE
   unset CMAKE_SYSROOT
   ```

5. Build the P4Runtime protobufs.

   ```bash
   cmake -B build -DDEPEND_INSTALL_DIR=<host dependencies>
   cmake --build build
   cmake --install build --prefix <install directory>
   ```

   `<host dependencies>` is the directory containing the host dependencies
   (the Stratum dependencies compiled for the x86).

   `<install directory>` is the directory in which you are installing
   P4 Control Plane, or an alternative location (e.g. `install`).

6. Exit the subshell.

   ```bash
   exit
   ```

   This restores your original environment.

The tarballs and Python wheel will be installed in the `share/p4runtime`
folder under the `<install directory>`.

The Go tarball will be omitted if golang is not installed.
