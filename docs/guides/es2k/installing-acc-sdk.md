# Installing the ACC SDK

## Overview

The AARCH64 Software Development Kit (SDK) allows you to use an X86 host
computer to build P4 Control Plane for use on the ARM Compute Complex
(ACC) of the Intel&reg; IPU E2100.

The SDK includes the following:

1. GCC C/C++ toolchain. Runs on an X86-64 Linux host and generates binary
   executables and libraries for an aarch64 target platform.
2. Target directory tree (sysroot). Provides header files and binaries
   (libraries and executables) for the aarch64 target platform.
3. ES2K P4SDE. Contains binaries and header files specific to the
   Intel&reg; IPU E2100.

## Obtain the ACC-RL SDK

You will need to obtain the software packages for the Intel&reg; IPU E2100
from the manufacturer.

The ACC-RL SDK package is a tarball whose name ends in `acc-rl-sdk`, such
as `mev-hw-b0-ci-ts.release.3921-acc-rl-sdk.tgz`.

## Choose an Install Location

You will need to choose a location for the SDK on your development computer.
The examples in this document assume you are logged in as user `peabody` and
that you plan to install the SDK in your home directory.

## Unpack the ACC-RL SDK

```bash
$ tar xzf mev-hw-b0-ci-ts.release.3921-acc-rl-sdk.tgz
$ pushd acc-rl/images/acc-sdk
$ ./install_acc_rl_sdk.sh -d ~/acc-sdk
You are about to install the SDK to "/home/peabody/acc-sdk". Proceed [Y/n]? y
Extracting SDK.............[redacted].............done
Setting it up...done
SDK has been successfully set up and is ready to be used.
Each time you wish to use the SDK in a new shell session, you need to source the environment setup script e.g.
 $ . /home/peabody/acc-sdk/environment-setup-aarch64-intel-linux-gnu
$ popd
```

## Key Components

### Layout

The  ACC-RL SDK is laid out as follows:

```text
acc-sdk/
|-- aarch64-intel-linux-gnu
|   |-- aarch64-intel-linux-gnu
|   |-- bin
|   |-- build.log.bz2
|   |-- include
|   |-- lib
|   |-- libexec
|   `-- share
`-- environment-setup-aarch64-intel-linux-gnu
```

### Toolchain

The cross-compiler suite is in the `aarch64-intel-linux-gnu/bin` directory.

### Sysroot

The `sysroot` directory is in the inner `aarch64-intel-linux-gnu` directory:

```text
aarch64-intel-linux-gnu/aarch64-intel-linux-gnu/
├── bin
├── debug-root
├── include
├── lib
├── lib64
└── sysroot
```

### P4SDE

The ES2K P4SDE is in the `sysroot/opt/p4` directory:

```text
sysroot/opt
|-- p4
|   `-- p4sde
|       |-- bin
|       |-- include
|       |-- lib
|       |-- lib64
|       `-- share
`-- rh
```
