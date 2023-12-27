# Defining the ACC Build Environment

In order to cross-compile for the ACC, you will need to define a number
of environment variables. This is typically done by putting the bash
commands in a file (e.g. `acc-setup.env`) and using the `source` command
to execute it. We recommend removing execute permission from the file
(`chmod a-x setup.env`) to remind yourself to source it, not run it.

## Setup script

The setup script requires access to the CMake toolchain file in order
to define the CMAKE_TOOLCHAIN_FILE environment variable. The toolchain
file is in the cmake subdirectory of the networking-recipe repository.
You will need this repository in order to define the P4CPBASE environment
variable.

```bash
# Define ACC build environment

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
[ -z "$ES2K_SAVE_PATH" ] && export ES2K_SAVE_PATH=$PATH
export PATH=$AARCH64/bin:$ES2K_SAVE_PATH

# CMake environment variable.
export CMAKE_TOOLCHAIN_FILE=$P4CPBASE/cmake/aarch64-toolchain.cmake
```

## Definitions

In the listing above, you will need to provide values for these variables:

- `ACC_SDK` - install path of the ACC-RL SDK (for example,
  `$HOME/p4cp-dev/acc_sdk`)
- `P4CPBASE` - path to the local networking-recipe directory (for example,
  `$HOME/p4cp-dev/p4cp.recipe`)

From these paths, the setup script derives:

- `AARCH64` - path to the directory containing the AArch64
  cross-compiler suite
- `SYSROOT` - path to the sysroot directory, which contains AArch64
  header files and binaries

These directories are part of the ACC SDK.

The setup script exports the following variables, which are used by CMake
and the helper scripts:

- `SDKTARGETSYSROOT` - path to the sysroot directory
- `PKG_CONFIG_PATH` - search path for `pkg-config` to use when looking for
  packages on the target system
- `PKG_CONFIG_SYSROOT_DIR` - path to the sysroot directory, for use by
  `pkg-config`
- `CMAKE_TOOLCHAIN_FILE` - path to the CMake toolchain file

The setup script also adds the directory containing the cross-compiler
executables to the system `PATH`.

## **Note**

The ACC-RL SDK includes its own setup file
(`environment-setup-aarch64-intel-linux-gnu`). We strongly recommend
that you *not* use this file when building the Stratum dependencies
or P4 Control Plane.

The SDK setup file is intended for use with GNU Autotools. Some of
the environment variables it defines affect the behavior of the C and
C++ compilers and the linker. These definitions may interfere with the
CMake build in non-obvious ways.
