# Building Stratum Dependencies for the ES2K ACC

This document explains how to build the Stratum dependencies for the
ARM Compute Complex (ACC) of the Intel&reg; IPU E2100.

> **Note**: To build the dependencies for a different target, see
[Building the Stratum Dependencies](building-stratum-deps.md).

## 1. Introduction

Stratum is the component of P4 Control Plane that implements the P4Runtime
and gNMI (OpenConfig) services. It requires a number of third-party
libraries, which this package provides.

You will need to build two versions of the libraries:

- The **Host** libraries, which run on the build system. They provide
  tools that are used to build P4 Control Plane.

- The **Target** libraries, which run on the ACC. P4 Control Plane is
  compiled and linked against these libraries.

The Host and Target libraries must be the same version.

## 2. Preparing the System

There are several things to do before you can build the dependencies.

- Install CMake 3.15 or above

  Avoid versions 3.24 and 3.25. They cause the dependencies build to fail.

- Install OpenSSL 1.1

  Note that P4 Control Plane is not compatible with BoringSSL.

- Install the ACC SDK

  See [Installing the ACC SDK](../docs/guides/es2k/installing-acc-sdk.md)
  for directions.

## 3. Getting the Source Code

The script to build the Stratum dependencies is in the IPDK networking-recipe
repository.

To clone the repository:

```bash
git clone --recursive https://github.com/ipdk-io/networking-recipe.git ipdk.recipe
```

You may omit the `--recursive` option if you are only interested in building
the dependencies.

You may substitute your own local directory name for `ipdk.recipe`.

The build script for the Stratum dependencies is in the `setup` directory.

The source code for the dependencies is not part of the distribution.
It is downloaded by the build script.

## 4. Building the Host Dependencies

First, decide where to install the Host dependencies. This location (the
"install prefix") must be specified when you configure the build.

It is recommended that you *not* install the Host dependencies in `/usr` or
`/usr/local`. It will be easier to rebuild or update the dependencies if
their libraries are not mingled with other libraries.

The `setup` directory includes a helper script (`make-host-deps.sh`) that
can be used to build the Host dependencies.

- The `--help` (`-h`) option lists the parameters the helper script supports

- The `--dry-run` (`-n`) option displays the parameter values without
  running CMake

The script normally does a minimal build, containing just the components
needed for cross-compilation. Specify the `--full` parameter if you want
to build all the libraries.

> **Note:** The Host and Target build environments are mutually incompatible.
  You must ensure that the [target build environment variables](#5-defining-the-target-build-environment)
  are undefined before you build the Host dependencies.

### User build

To install the dependencies in a user directory:

```bash
./make-host-deps.sh --prefix=PREFIX
```

PREFIX might something like `~/hostdeps`.

The source files will be downloaded and built, and the results will be
installed in the specified directory.

### System build

To install the Host dependencies in a system directory, log in as `root`
or build from an account that has `sudo` privilege.

```bash
./make-host-deps.sh --prefix=PREFIX --sudo
```

PREFIX might be something like `/opt/ipdk/x86deps`.

The script only uses `sudo` when installing libraries. Omit the parameter
if you are running as `root`.

## 5. Defining the Target Build Environment

In order to cross-compile for the ACC, you will need to define a number
of environment variables. This is typically done by putting the bash
commands in a file (e.g. `es2k-setup.env`) and using the `source` command
to execute it. We recommend removing execute permission from the file
(`chmod a-x setup.env`) to remind yourself to source it, not run it.

For example:

```bash
# Set by user. Used internally.
ACC_SDK=<acc-sdk-directory>
P4CPBASE=<recipe-directory>

# Used internally.
AARCH64=$ACC_SDK/aarch64-intel-linux-gnu
SYSROOT=$AARCH64/aarch64-intel-linux-gnu/sysroot

# Used externally for build.
export SDKTARGETSYSROOT=$SYSROOT
export PKG_CONFIG_SYSROOT_DIR=$SYSROOT
export PKG_CONFIG_PATH=$SYSROOT/usr/lib64/pkgconfig:$SYSROOT/usr/lib/pkgconfig:$SYSROOT/usr/share/pkgconfig
export CMAKE_TOOLCHAIN_FILE=$P4CPBASE/cmake/aarch64-toolchain.cmake
[ -z "$ES2K_SAVE_PATH" ] && export ES2K_SAVE_PATH=$PATH
export PATH=$AARCH64/bin:$ES2K_SAVE_PATH
```

In the listing above, you will need to provide values for these variables:

- `ACC_SDK` - install path of the ACC-RL SDK (for example,
  `$HOME/p4cp-dev/acc_sdk`)
- `P4CPBASE` - path to the local networking-recipe directory (for example,
  `$HOME/p4cp-dev/ipdk.recipe`)

From these paths, the setup script derives:

- `AARCH64` - path to the directory containing the AArch64
  cross-compiler suite
- `SYSROOT` - path to the sysroot directory, which contains AArch64
  header files and binaries

These directories are part of the ACC SDK.

The setup script exports the following variables, which are used by CMake
and the helper script:

- `SDKTARGETSYSROOT` - path to the sysroot directory
- `CMAKE_TOOLCHAIN_FILE` - path to the CMake toolchain file
- `PKG_CONFIG_PATH` - search path for `pkg-config` to use when looking for
  packages on the target system
- `PKG_CONFIG_SYSROOT_DIR` - path to the sysroot directory, for use by
  `pkg-config`

The setup script also adds the directory containing the cross-compiler
executables to the system `PATH`.

> **Note:** The ACC-RL SDK includes its own setup file
> (`environment-setup-aarch64-intel-linux-gnu`). We strongly recommend
> that you *not* use this file when building the Stratum dependencies
> or P4 Control Plane.
>
> The SDK setup file is intended for use with GNU Autotools. Some of
> the environment variables it defines affect the behavior of the C and
> C++ compilers and the linker. These definitions may interfere with the
> CMake build in non-obvious ways.

## 6. Building the Target Dependencies

You will need to pick an install location for the target dependencies.
This will typically be under the sysroot directory structure. For
example, the `opt` subdirectory will become the root-level `/opt`
directory when the file structure is copied to the E2100 file system.

The `setup` directory includes a helper script (`make-cross-deps.sh`) that
can be used to build the Target dependencies.

- The `--help` (`-h`) option lists the parameters the helper script supports

- The `--dry-run` (`-n`) option displays the parameter values without
  running CMake

You will need to provide the helper script with the path to the Host
dependencies (`--host`) as well as the install prefix (`--prefix`).

### Target build

Source the file that the defines the [target build environment variables](#5-defining-the-target-build-environment).

```bash
source es2k-setup.env
```

Remove the `build` directory from the previous build.

```bash
rm -fr build
```

Now run the build script:

```bash
./make-cross-deps.sh --host=HOSTDEPS --prefix=PREFIX
```

`HOSTDEPS` is the path to the Host dependencies, e.g., `~/p4cp-dev/hostdeps`.

`PREFIX` is the install prefix for the Target dependencies. Here, you might
specify something like `//opt/ipdk/deps`.

The `//` at the beginning of the prefix path is a shortcut provided by
the helper script. It will be replaced with the sysroot directory path.
