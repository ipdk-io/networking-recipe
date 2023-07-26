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

## Install the Stratum Dependencies

See [Building ES2K Dependencies for the ACC](../../../setup/building-es2k-stratum-deps.md)
for instructions.

You will need the Host depencies, Target dependencies, and Build environment
setup file, in order to cross-compile P4 Control Plane for the ACC.

## Fetch the Source Code

If you have not already done so, you will to get a copy of the IPDK
networking-recipe repository and its submodules:

```bash
git clone --recursive https://github.com/ipdk-io/networking-recipe.git ipdk.recipe
```

You may substitute your own local directory name for `ipdk.recipe`.

## Build P4 Control Plane

### Build with OVS

```bash
./make-cross-ovs.sh --prefix=//opt/ipdk/ovs
```

```bash
./config-cross-recipe.sh --host=../hostdeps \
    --deps=//opt/ipdk/deps --ovs=//opt/ipdk/ovs --prefix=//opt/ipdk/p4cp \
    --sde=//opt/p4/p4sde
```

```bash
cmake --build build -j8 --target install
```

### Build without OVS

```bash
./config-cross-recipe.sh --host=../hostdeps \
    --deps=//opt/ipdk/deps --no-ovs --prefix=//opt/ipdk/p4cp \
    --sde=//opt/p4/p4sde
```

```bash
cmake --build build -j8 --target install
```
