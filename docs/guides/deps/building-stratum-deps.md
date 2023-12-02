# Building the Stratum Dependencies

Stratum is the component of `infrap4d` that implements the P4Runtime and gNMI
(OpenConfig) services. It requires that a number of third-party libraries
be built and installed on the system.

This document explains how to build and install the Stratum dependencies.

> **Note**: For the Intel&reg; IPU E2100, see
[Building Stratum Dependencies for the ACC](building-acc-stratum-deps.md).

## Prerequisites

There are a couple of things to do before you build the dependencies:

- Install CMake 3.15 or above

  Avoid versions 3.24 and 3.25. There is an issue in cmake that causes the
  Protobuf build to fail. This problem was fixed in version 3.26.

- Install OpenSSL 3.x

  P4 Control Plane uses OpenSSL instead of BoringSSL.

## Source Code

There are two ways to obtain the source code: from the open-source repository
on GitHub, or as a source package.

### GitHub repository

The script to build the Stratum dependencies is in the `stratum-deps`
repository. To clone the repository:

```bash
git clone https://github.com/ipdk-io/stratum-deps.git
```

The source code for the dependencies is not part of the distribution.
It is downloaded by the build script.

### Source package

You may receive the source code for the Stratum dependencies in the form of
an RPM file, a Debian file, or a tarball.

If this is case, use the appropriate command to unpack the source files.

## Install Location

You will need to decide where on your system you would like to install the
dependencies. This location (the "install prefix") must be specified when you
configure the build.

It is recommended that you _not_ install the dependency libraries in `/usr` or
`/usr/local`. It is easier to upgrade or do a clean build if packages that are
updated on different cadences are kept separate from one another.

The `CMAKE_INSTALL_PREFIX` option is used to specify the directory in which
the dependencies should be installed.

If you plan to install the dependency libraries in a system directory, you will
need to log in as `root` or run from an account that has `sudo` privilege.

## CMake Build Options

The CMake build script supports the following configuration options.

| Option | Type | Description |
| ------ | ---- | ----------- |
| `CMAKE_INSTALL_PREFIX` | Path | Directory in which the dependencies should be installed. |
| `CXX_STANDARD` | Number | C++ standard (11, 14, 17, etc.) the compiler should apply. (Default: None) |
| `DOWNLOAD` | Boolean | Whether to download the source repositories. (Default: [1]) |
| `PATCH` | Boolean | Whether to patch the downloaded repositories. (Default: [1]) |
| `ON_DEMAND` | Boolean | Whether to build only the specified target(s). If this option is FALSE, all targets will be built. (Default: FALSE) |
| `USE_LDCONFIG` | Boolean | Whether to use `ldconfig` to update the loader cache[2] after installing a module. Only valid if `USE_SUDO` is enabled. (Default: FALSE) |
| `USE_SUDO` | Boolean | Whether to use `sudo` to install each module. (Default: FALSE) |

Boolean values are (`TRUE`, `YES`, `ON`) and (`FALSE`, `NO`, `OFF`).
They may be upper or lower case.

[1] `DOWNLOAD` and `PATCH` default to `TRUE` if you download stratum-deps
    from GitHub, and `FALSE` if you receive a distribution that includes
    the source.

[2] See the `ldconfig` man page for more information.

## Examples

### Non-root build

To build and install as a non-privileged user:

```bash
cd setup
cmake -B build -DCMAKE_INSTALL_PREFIX=./install
cmake --build build -j8
```

The source files will be downloaded, built, and installed in the `install`
directory. The targets will be built in parallel, using eight threads.

CMake will generate all of its temporary files in the `build` directory.
This is called an "out-of-source" build.

### Non-root build to system directory

To build as a non-privileged user and install the libraries in a
system directory:

```bash
cmake -B build -DCMAKE_INSTALL_PREFIX=/opt/deps -DUSE_SUDO=yes
cmake --build build -j6
```

CMake will build the dependencies as the current user.
It will use `sudo` only when it installs the libraries.
The build will be done in parallel, using six threads.

### On-demand build

To build and install just gRPC and its dependencies:

```bash
cmake -B build -DON_DEMAND=yes -DCMAKE_INSTALL_PREFIX=~/hostdeps
cmake --build build -j6 --target grpc
```

The necessary components will be downloaded, built, and installed in
`/home/<username>/hostdeps`. The build will be done in parallel, using
six threads.

### Rebuilding without downloading

To instruct cmake not to download the source repositories again:

```bash
cmake -B build -DCMAKE_INSTALL_PREFIX=deps -DDOWNLOAD=NO -DPATCH=NO
cmake --build build -j6
```

----

Alternatively, you can run a script that changes the default values
of the `DOWNLOAD` and `PATCH` parameters to `NO`.

```bash
./scripts/preconfig.sh
```

The script writes a `preload.cmake` file to the `source` directory
that cmake will read the next time it configures the build.

### Clean builds

To do a clean build without deleting the source directories:

```bash
rm -fr build
cmake -B build -DCMAKE_INSTALL_PREFIX=deps ...
```

You'll need to specify `-DDOWNLOAD=NO -DPATCH=NO` or run the `preload.sh`
script, as described under
[Rebuilding without downloading](#rebuilding-without-downloading), or
cmake will download the repositories a second time.

This erases the configuration as well as the binary build artifacts.

----

To get a fresh start, delete both the `build` and `source` directories:

To scrub the directory:

```bash
rm -fr build 
```
