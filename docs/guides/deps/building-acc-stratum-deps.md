# Building Dependencies for the ACC

This document explains how to build the Stratum dependencies for the
ARM Compute Complex (ACC) of the Intel&reg; IPU E2100.

> **Note**: To build the dependencies for a target other than ES2K, see
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

There are a couple of things to do before you build the dependencies:

- Install CMake 3.15 or above

  Avoid versions 3.24 and 3.25. There is an issue in cmake that causes the
  Protobuf build to fail. This problem was fixed in version 3.26.

- Install OpenSSL 3.x

  Note that P4 Control Plane is not compatible with BoringSSL.

## 3. Getting the Source Code

The script to build the Stratum dependencies is in the stratum-deps
repository.

To clone the repository:

```bash
git clone https://github.com/ipdk-io/stratum-deps.git
```

The source code for the dependencies is not part of the distribution.
It is downloaded by the build script.

## 4. Defining the Build Environment

See [Defining the ACC Build Environment](/guides/es2k/defining-acc-environment.md)
for directions on creating the environment setup file.

## 5. Building the Host Dependencies

First, decide where to install the Host dependencies. This location (the
"install prefix") must be specified when you configure the build.

It is recommended that you *not* install the Host dependencies in `/usr` or
`/usr/local`. It will be easier to rebuild or update the dependencies if
their libraries are not mingled with other libraries.

The `scripts` subdirectory includes a helper script (`make-host-deps.sh`) that
can be used to build the Host dependencies.

- The `--help` (`-h`) option lists the parameters the helper script supports

- The `--dry-run` (`-n`) option displays the parameter values without
  running CMake

> **Note:** The Host and Target build environments are mutually incompatible.
  You must ensure that the [target build environment variables](/guides/es2k/defining-acc-environment.md)
  are undefined before you build the Host dependencies.

### User build

To install the dependencies in a user directory:

```bash
./scripts/make-host-deps.sh --prefix=PREFIX
```

PREFIX might something like `~/hostdeps`.

The source files will be downloaded and built, and the results will be
installed in the specified directory.

### System build

To install the Host dependencies in a system directory, log in as `root`
or build from an account that has `sudo` privilege.

```bash
./scripts/make-host-deps.sh --prefix=PREFIX --sudo
```

PREFIX might be something like `/opt/ipdk/x86deps`.

The script only uses `sudo` when installing libraries. Omit the parameter
if you are running as `root`.

## 6. Building the Target Dependencies

You will need to pick an install location for the target dependencies.
This will typically be under the sysroot directory structure. For
example, the `opt` subdirectory will become the root-level `/opt`
directory when the file structure is copied to the E2100 file system.

The `scripts` subdirectory includes a helper script (`make-cross-deps.sh`)
that can be used to build the Target dependencies.

- The `--help` (`-h`) option lists the parameters the helper script supports

- The `--dry-run` (`-n`) option displays the parameter values without
  running CMake

You will need to provide the helper script with the path to the Host
dependencies (`--host`) as well as the install prefix (`--prefix`).

### Target build

Source the file that defines the
[target build environment variables](/guides/es2k/defining-acc-environment.md).

```bash
source es2k-setup.env
```

Remove the `build` directory from the previous build.

```bash
rm -fr build
```

Now run the build script:

```bash
./scripts/make-cross-deps.sh --host=HOSTDEPS --prefix=PREFIX
```

`HOSTDEPS` is the path to the Host dependencies, e.g., `~/p4cp-dev/hostdeps`.

`PREFIX` is the install prefix for the Target dependencies. Here, you might
specify something like `//opt/ipdk/deps`.

The `//` at the beginning of the prefix path is a shortcut provided by
the helper script. It will be replaced with the sysroot directory path.
