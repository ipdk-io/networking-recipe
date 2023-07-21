# Building the Stratum Dependencies

Stratum is the component of `infrap4d` that implements the P4Runtime and gNMI
(OpenConfig) services. It requires that a number of third-party libraries
be built and installed on the system.

This document explains how to build and install the Stratum dependencies.

> **Note**: For the Intel&reg; IPU E2100, see
[Building Stratum Dependencies for the ES2K ACC](building-es2k-stratum-deps.md).

## 1 Prerequisites

### 1.1 CMake

CMake 3.15 or above must be installed.

Avoid versions 3.24 and 3.25. There is an issue in cmake that causes the
Protobuf build to fail. This problem was fixed in version 3.26.

### 1.2 OpenSSL

OpenSSL 1.1 must be installed. P4 Control Plane uses OpenSSL instead of
BoringSSL.

## 2 Source Code

There are two ways to obtain the source code: from the open-source repository
on GitHub, or as a source package.

### 2.1 GitHub Repository

The Stratum dependencies can be built using a script that is part of the
IPDK networking-recipe. To clone the repository:

```bash
git clone --recursive https://github.com/ipdk-io/networking-recipe.git ipdk.recipe
```

You may omit the `--recursive` option if you are only interested in building
the dependencies.

You may substitute your own local directory name for `ipdk.recipe`.

The build script for the stratum dependencies is in the `setup` directory.

### 2.2 Source Package

You may receive the source code for the Stratum dependencies in the form of
an RPM file, a Debian file, or a tarball.

If this is case, use the appropriate command to unpack the source files.

## 3 Install Location

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

## 4 Downloaded Source vs. Source Package

By default, the cmake build script downloads the source code for each module
and then builds and installs it.

If you are working from a source distribution, or if the source files were
downloaded by a previous build, you can override the download stage by
specifying:

```cmake
  -DDOWNLOAD=no -DFORCE_PATCH=yes
```  

when you configure the cmake build.

## 5 CMake Build Options

The CMake build script supports the following configuration options.

| Option | Type | Description |
| ------ | ---- | ----------- |
| `CMAKE_INSTALL_PREFIX` | Path | Directory in which the dependencies should be installed. |
| `CXX_STANDARD` | Number | C++ standard (11, 14, 17, etc.) the compiler should apply. (Default: not specified) |
| `DOWNLOAD` | Boolean | Whether to download the source repositories. (Default: TRUE)
| `FORCE_PATCH` | Boolean | Whether to specify the force (`-f`) option when patching a downloaded repository. (Default: FALSE) |
| `ON_DEMAND` | Boolean | Whether to build only the specified target(s). If this option is FALSE, all targets will be built. (Default: FALSE) |
| `USE_LDCONFIG` | Boolean | Whether to use `ldconfig` to update the loader cache[1] after installing a module. Only valid if `USE_SUDO` is enabled. (Default: FALSE) |
| `USE_SUDO` | Boolean | Whether to use `sudo` to install each module. (Default: FALSE) |

Boolean values are (`TRUE`, `YES`, `ON`) and (`FALSE`, `NO`, `OFF`).
They may be upper or lower case.

[1] See the `ldconfig` man page for more information.

## 6 Examples

### 6.1 Non-root build

To build and install as a non-privileged user:

```bash
cd setup
cmake -B build -DCMAKE_INSTALL_PREFIX=./install
cmake --build build -j8
```

The source files will be downloaded, built, and installed in the `setup/install`
directory. The targets will be built in parallel, using eight threads.

CMake will generate all of its temporary files in the `setup/build` directory.
This is called an "out-of-source" build.

### 6.2 Non-root build to system directory

To build as a non-privileged user and use `sudo` to install the libraries to
a system directory:

```bash
cmake -B build -DCMAKE_INSTALL_PREFIX=/opt/deps -DUSE_SUDO=yes
cmake --build build -j6
```

CMake will build the dependencies as the current user and use `sudo` to
install the libraries in `/opt/deps`. The build will be done in parallel,
using six threads.

### 6.3 Root build from source

To build from a source distribution or from repositories that were previously
downloaded:

```bash
cmake -B build -DDOWNLOAD=no -DFORCE_PATCH=yes \
      -DCMAKE_INSTALL_PREFIX=/opt/ipdk/hostdeps
cmake --build build -j8
```

CMake will build and install the dependency libraries without downloading the
source code.

The `FORCE_PATCH` option instructs the patch step to continue without prompting
if a module has already patched.

### 6.4 On-demand build

To build and install just gRPC and its dependencies:

```bash
cmake -B build -DON_DEMAND=yes -DCMAKE_INSTALL_PREFIX=~/hostdeps
cmake --build build -j6 --target grpc
```

The necessary components will be downloaded, built, and installed in
`/home/<username>/hostdeps`. The build will be done in parallel, using
six threads.

### 6.5 Clean build

To do a clean build without deleting the source directories:

```bash
rm -fr build
```

This deletes all temporary files from the previous build, preserving the
source directories.

### 6.6 Clean the `setup` directory

To scrub the directory:

```bash
./cleanup.sh
```

This removes the `build` and source directories.
