# Building Stratum Dependencies for the ES2K ACC

This document explains how to build the Stratum dependencies for the
ARM Compute Complex (ACC) of the Intel&reg; IPU E2100.

> **Note**: If you are building P4 Control Plane to run on an x86 host
core of the E2100, follow the directions in
[Building the Stratum Dependencies](../building-stratum-deps.md) and
specify `ES2K`` as the TDI target type.

## 1. Introduction

Stratum is the component of `infrap4d` that implements the P4Runtime and gNMI
(OpenConfig) services. It requires that a number of third-party libraries
be built and installed on the system.

You will need to build two versions of the dependency libraries:

- The **Host** libraries, which run on the build system and the x86
  cores of the E2100. They also provide tools that are used during the
  build process.

- The **Target** libraries, which run on the ACC. The P4 Control Plane is
  compiled and linked against these libraries.

The Host and Target libraries versions must match, to ensure compatibility
between client applications and `infrap4d`.

## 2. Setup

There are several things you need to take care of on your development
system before you can build.

### CMake

CMake 3.15 or above must be installed.

Avoid versions 3.24 and 3.25. There is an issue in cmake that causes the
Protobuf build to fail. The issue was fixed in version 3.26.

### OpenSSL

OpenSSL 1.1 must be installed. P4 Control Plane uses OpenSSL instead of
BoringSSL.

### ACC SDK

Install the ACC SDK on your system. See
[Installing the ACC SDK](../docs/guides/es2k/installing-acc-sdk.md) for
instructions. You will be using the Toolchain and Sysroot directories
to build the Target dependencies.

## 3. Source Code

Download the Open-Source IPDK networking-recipe from GitHub. To clone the
repository:

```bash
git clone --recursive https://github.com/ipdk-io/networking-recipe.git ipdk.recipe
```

You may omit the `--recursive` option if you are only interested in building
the dependencies.

You may substitute your own local directory name for `ipdk.recipe`.

The build script for the Stratum dependencies is in the `setup` directory.

## 4. Host Dependencies

### Host install location

You will need to decide where on your system you would like to install the
Host dependencies. This location (the "install prefix") must be specified
when you configure the build.

It is recommended that you *not* install the Host dependencies in `/usr` or
`/usr/local`. You will find it easier to update or rebuild the libraries
if components that are updated at different times are kept separate from
one another.

The `CMAKE_INSTALL_PREFIX` option is used to specify the directory in which
the Host dependencies should be installed.

If you plan *do* install the Host dependencies in a system directory, you will
need to log in as `root` or run from an account that has `sudo` privilege.

The distribution includes a helper script (`make-host-deps.sh`) that can be
used to build the Host dependencies.

### Example: non-root build

To install the dependencies in a user directory:

```bash
./make-host-deps.sh --prefix=~/hostdeps
```

The source files will be download and built, and the results of the build
will be installed in `/home/peabody/hostdeps`.

### Example: root build

```bash
./make-host-deps.sh --prefix=/opt/p4cp/x86deps --sudo
```

The `--sudo` parameter may be omitted if you are logged in as `root`.

### Example: complete build

By default, the helper script does a *minimal* build, consisting only of
the components that are needed to support cross-compilation. Specify the
`--full` parameter if you would like to build all the components.

```bash
./make-host-deps.sh --prefix=$HOME/hostdeps --full
```

### make-host-deps.sh

The helper script supports the following options:

```text
peabody@wabac:~/recipe/setup$ ./make-host-deps.sh -h

Build host dependency libraries

Options:
  --build=DIR     -B  Build directory path (Default: build)
  --config            Only perform configuration step (Default: false)
  --cxx=VERSION       CXX_STANDARD to build dependencies (Default: empty)
  --dry-run       -n  Display cmake parameters and exit (Default: false)
  --force         -f  Specify -f when patching (Default: false)
  --full              Build all dependency libraries (Default: minimal)
  --jobs=NJOBS    -j  Number of build threads (Default: 8)
  --minimal           Build required host dependencies only (Default: minimal)
  --no-download       Do not download repositories (Default: false)
  --prefix=DIR    -P  Install directory path (Default: ./host-deps)
  --sudo              Use sudo when installing (Default: false)
```

## 4. Target Dependencies

### Target install location

under construction

### Cross-compilation

under construction

### Example: sysroot build

```bash
./make-cross-deps.sh --host=/opt/x86deps --prefix=//opt/p4cp/deps
```

### make-cross-deps.sh

The helper script supports the following options:

```text
peabody@wabac:~/recipe/setup$ ./make-cross-deps.sh -h

Build target dependency libraries

Options:
  --build=DIR      -B  Build directory path [build]
  --cxx=VERSION        CXX_STANDARD to build dependencies (Default: empty)
  --dry-run        -n  Display cmake parameters and exit
  --force          -f  Specify -f when patching (Default: false)
  --host=DIR       -H  Host dependencies directory []
  --jobs=NJOBS     -j  Number of build threads (Default: 8)
  --no-download        Do not download repositories (Default: false)
  --prefix=DIR*    -P  Install directory prefix [//opt/deps]
  --sudo               Use sudo when installing (Default: false)
  --toolchain=FILE -T  CMake toolchain file

* '//' at the beginning of the directory path will be replaced
  with the sysroot directory path.

Environment variables:
  CMAKE_TOOLCHAIN_FILE - Default toolchain file
  SDKTARGETSYSROOT - sysroot directory
```
