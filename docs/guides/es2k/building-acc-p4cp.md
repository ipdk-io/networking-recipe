# Building P4 Control Plane for the ACC

This document explains how to build P4 Control Plane for the ARM Compute
Complex (ACC) of the Intel&reg; IPU E2100.

## Prepare the System

There are several things to do before you can build P4 Control Plane.

- Install CMake 3.15 or above

  Avoid versions 3.24 and 3.25. They cause the dependencies build to fail.

- Install OpenSSL 3.x

  Note that P4 Control Plane is not compatible with BoringSSL.

- Install the ACC SDK

  See [Installing the ACC SDK](installing-acc-sdk.md) for instructions.

## Install the Stratum Dependencies

See [Building Stratum Dependencies for the ACC](/guides/deps/building-acc-stratum-deps.md)
for instructions.

You will need the Host dependencies, Target dependencies, and Build environment
setup file, in order to cross-compile P4 Control Plane for the ACC.

## Fetch the Source Code

If you have not already done so, you will need to get a copy of the IPDK
networking-recipe repository and its submodules:

```bash
git clone --recursive https://github.com/ipdk-io/networking-recipe.git p4cp.recipe
```

You may substitute your own local directory name for `p4cp.recipe`.

## Define the Environment

First, change to the source directory:

```bash
cd p4cp.recipe
```

Source the file that defines the
[target build environment variables](defining-acc-environment.md).
For example:

```bash
source SETUPFILE
```

where `SETUPFILE` is the path to the file you created when you built the
Stratum dependencies (for example, `setup/es2k-setup.env`).

## Build with OVS

### Building OVS

The distribution includes a helper script (`make-cross-ovs.sh`) that can be
used to build OVS for P4 Control Plane.

> The `--help` (`-h`) option lists the parameters the helper script supports.

To build OVS and install it in the sysroot directory under `/opt/ipdk/ovs`:

```bash
./scripts/es2k/make-cross-ovs.sh --prefix=//opt/ipdk/ovs
```

Options:

- `--prefix=PREFIX` - where to install OVS

The `//` at the beginning of the prefix path is a shortcut provided by
the helper script. It will be replaced with the sysroot directory path.

To do a clean build, issue the command `rm -fr ovs/build` before running
`make-cross-ovs.sh`. You may also want to remove the installation
directory from the previous build.

### Configuring P4 Control Plane

The distribution includes a helper script (`config-cross-deps.sh`) that
can be used to configure CMake to build P4 Control Plane.

> The `--help` (`-h`) option lists the parameters the helper script supports.

To configure CMake to build P4 Control Plane and install it in the sysroot
directory under `/opt/ipdk/p4cp`:

```bash
./scripts/es2k/config-cross-recipe.sh \
    --host=../hostdeps --deps=//opt/ipdk/deps \
    --ovs=//opt/ipdk/ovs --sde=//opt/p4/p4sde \
    --prefix=//opt/ipdk/p4cp
```

Options:

- `--host=HOST` - path to the Host dependencies
- `--deps=DEPS` - path to the Target dependencies
- `--ovs=OVS` - path to the OVS installation
- `--sde=SDE` - path to the SDE installation
- `--prefix=PREFIX` - where to install P4 control plane

The `//` at the beginning of the prefix path is a shortcut provided by
the helper script. It will be replaced with the sysroot directory path.

To do a clean build, issue the command `rm -fr build` before running
`config-cross-recipe.sh`. You may also want to remove the installation
directory from the previous build.

### Building P4 Control Plane

Now use CMake to build and install P4 Control Plane:

```bash
cmake --build build -j8 --target install
```

## Build without OVS

To build P4 Control Plane without OVS:

```bash
./scripts/es2k/config-cross-recipe.sh \
    --host=../hostdeps --deps=//opt/ipdk/deps \
    --no-ovs --sde=//opt/p4/p4sde \
    --prefix=//opt/ipdk/p4cp

cmake --build build -j8 --target install
```

The `--no-ovs` option excludes OVS support.

## Build with Stratum Only

To build P4 Control Plane without OVS or the Kernel Monitor:

```bash
./scripts/es2k/config-cross-recipe.sh \
    --host=../hostdeps --deps=//opt/ipdk/deps \
    --no-ovs --no-krnlmon --sde=//opt/p4/p4sde \
    --prefix=//opt/ipdk/p4cp

cmake --build build -j8 --target install
```
