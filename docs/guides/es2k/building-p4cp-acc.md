# Building P4 Control Plane for the ACC

This document explains how to build P4 Control Plane for the ARM Compute
Complex (ACC) of the Intel&reg; IPU E2100.

## Prepare the System

There are several things to do before you can build P4 Control Plane.

- Install CMake 3.15 or above

  Avoid versions 3.24 and 3.25. They cause the dependencies build to fail.

- Install OpenSSL 1.1

  Note that P4 Control Plane is not compatible with BoringSSL.

- Install the ACC SDK

  See [Installing the ACC SDK](installing-acc-sdk.md) for instructions.

## Install Stratum Dependencies

See [Building Stratum Dependencies for the ACC](../../../setup/building-es2k-stratum-deps.md)
for instructions.

You will need the Host depencies, Target dependencies, and Build environment
setup file, in order to cross-compile P4 Control Plane for the ACC.

## Fetch Source Code

If you have not already done so, you will to get a copy of the IPDK
networking-recipe repository and its submodules:

```bash
git clone --recursive https://github.com/ipdk-io/networking-recipe.git ipdk.recipe
```

You may substitute your own local directory name for `ipdk.recipe`.

## Build with OVS

### Building OVS

The distribution includes a helper script (`make-cross-ovs.sh`) that can be
used to build OVS for P4 Control Plane.

- The `--help` (`-h`) option lists the parameters the helper script supports

- The `--dry-run` (`-n`) option displays the parameter values without
  running CMake

To build OVS and install it in the sysroot directory under `/opt/ipdk/ovs`:

```bash
./make-cross-ovs.sh --prefix=//opt/ipdk/ovs
```

### Configuring P4 Control Plane

The distribution includes a helper script (`config-cross-deps.sh`) that
can be used to configure CMake to build P4 Control Plane.

- The `--help` (`-h`) option lists the parameters the helper script supports

- The `--dry-run` (`-n`) option displays the parameter values without
  running CMake

To configure P4 Control Plane to build with OVS:

```bash
./config-cross-recipe.sh \
    --host=../hostdeps --deps=//opt/ipdk/deps \
    --ovs=//opt/ipdk/ovs --sde=//opt/p4/p4sde \
    --prefix=//opt/ipdk/p4cp
```

### Building P4 Control Plane

The final step is to use CMake to perform the build:

```bash
cmake --build build -j8 --target install
```

## Build without OVS

To build without OVS, skip the OVS build step and specify the `--no-ovs`
option when you configure the P4 Control Plane build:

```bash
./config-cross-recipe.sh \
    --host=../hostdeps --deps=//opt/ipdk/deps \
    --no-ovs --sde=//opt/p4/p4sde \
    --prefix=//opt/ipdk/p4cp
```

Now use CMake to perform the build:

```bash
cmake --build build -j8 --target install
```
