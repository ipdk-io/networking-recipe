# Development Update 23.21

## Overview

This is an update (code drop) to incorporate changes made to support
the Intel IPU E2100.

## Breaking Changes

- The `make-all.sh` helper script has been rewritten and its options have
  changed. The new script is not backward compatible with the old script.
  See [make-all script](#make-all-script) for details.

- `infrap4d` now defaults to secure (TLS) mode, which requires that you
  provide a certificate or override secure mode on startup. See
  [Security improvements](#security-improvements) for more information.

- P4 Control Plane currently uses a modified fork of the `p4runtime` repository.
  See [P4runtime fork](#p4runtime-fork) for more information.

## What's Changed

### Highlights

- New ES2K target
- Redesigned make-all.sh script
- P4Runtime fork
- Default build target
- Improved security

### ES2K Target

This update adds support for a new target, the Intel IPU E2100.

The new target is referred to in software as "ES2K". There are plans to
change this label to something less ASIC-specific prior to the product's
actual release.

As the third target implemented, the first engineered as an IPU, and the first
to require cross-compilation, ES2K has had an impact throughout the codebase.

Component changes:

- Add ES2K TDI target to Stratum
- Add ES2K support to Kernel Monitor
- Add ES2K support to ovs-p4rt

Build system changes:

- Support separate host and target dependencies
- Support SYSROOT and STAGING directories
- Support cross-compilation with aarch64 toolchain file
- Move P4 driver definitions to include files
- Improve RPATH (RUNPATH) support
- Support variations in system library locations
- Add `make-cross-ovs.sh` and `config-cross-recipe.sh` helper scripts
  to assist with cross-compiling P4 Control Plane
- Add infrastructure for Kernel Monitor unit tests

Setup changes:

- Allow dependencies to be built for the Host system (full or minimal)
  or cross-compiled for the Target system
- Remove GMOCK_GBL from dependencies
- Add cmake option to suppress download
- Add cmake option to specify CXX standard for Abseil and gRPC
- Add `make-cross-deps.sh` and `make-host-deps.sh` helper scripts to assist
  with building host and cross-compiled dependencies

### make-all script

The `make-all.sh` helper script has been rewritten to be more consistent
and provide better control over the underlying build.

Note that options have changed and the redesigned script is not backward
compatible with its predecessor.

The new script displays command-line help (`--help`, `-h`) and cmake parameters
(`--dry-run`, `-n`), allows paths to be specified via environment variables
or command-line parameters, controls commonly-used build options, and supports
cross compilation.

```text
doctor@tardis:~/recipe$ ./make-all.sh -h

Configure and build P4 Control Plane software

Paths:
  --deps=DIR       -D  Target dependencies directory []
  --host=DIR       -H  Host dependencies directory []
  --ovs=DIR        -O  OVS install directory [ovs/install]
  --prefix=DIR     -P  Install directory prefix [install]
  --sde=DIR        -S  SDE install directory []
  --toolchain=FILE -T  CMake toolchain file []

Options:
  --coverage           Measure unit test code coverage
  --dry-run        -n  Display cmake parameter values and exit
  --help           -h  Display this help text
  --jobs=NJOBS     -j  Number of build threads (Default: 8)
  --no-krnlmon         Exclude Kernel Monitor
  --no-ovs             Exclude OVS support
  --target=TARGET      Target to build (dpdk|es2k|tofino) [DPDK]

Configurations:
  --debug              Debug configuration
  --minsize            MinSizeRel configuration
  --reldeb             RelWithDebInfo configuration (default)
  --release            Release configuration

  Default config is RelWithDebInfo (--reldeb)

Environment variables:
  CMAKE_TOOLCHAIN_FILE - Default toolchain file
  DEPEND_INSTALL - Default target dependencies directory
  HOST_INSTALL - Default host dependencies directory
  OVS_INSTALL - Default OVS install directory
  SDE_INSTALL - Default SDE install directory
  ```

### P4Runtime fork

P4 Control Plane currently uses a fork of the `p4runtime` repository to
support mirroring for ES2K. The development team is working on a replacement
methodology. We will revert to using the standard `p4runtime` repository once
the revision is in place.

A corresponding version of the P4Runtime python module is available in the
`setup` directory as `p4runtime-1.3.0-py3-none-any.whl`.

### Default build target

The default target for the P4 Control Plane build has been changed from
`Tofino` to `DPDK`.

### Security improvements

`infrap4d` is now secure by default when built for the DPDK and ES2K targets.
The P4Runtime and gNMI ports are opened with TLS enabled, and require that a
certificate be provided. This can be overridden on startup by specifying
`-grpc_open_insecure_mode=true` on the `infrap4d` command line.

See the [security guide](/guides/security/security-guide.md) for more information.

`sgnmi_cli` is a gNMI client tool that defaults to secure mode. See the
[sgnmi_cli guide](/clients/sgnmi_cli.md) for more information.

## Component Changes

### Kernel Monitor

The Kernel Monitor has been updated to support ES2K.

krnlmon is enabled by default for DPDK and ES2K builds. It can be suppressed
by specifying `--no-krnlmon` on the `make-all.sh` command line. It can also
be disabled at runtime by specifying `-disable_krnlmon` on the `infrap4d`
command line.

### OVS

Open vSwitch (OVS) and the OVS P4Runtime Library (`ovs-p4rt`) are enabled by
default for DPDK and ES2K builds. They can be suppressed by specifying
`--no-ovs` on the `make-all.sh` command line.

When OVS is enabled, P4 Control Plane builds Open vSwitch with P4 extensions.
It also builds a library (`ovs-p4rt`) that is linked with `ovs-vswitchd` to
allow OVS to issue P4Runtime requests to `infrap4d`.

ovs-p4rt has been extended to support ES2K.

### Stratum

The `TdiPortManager` class has been extracted from `TdiSdeInterface` and
moved to a separate header file. The `PortEvent` class is now part of the
`TdiPortManager` class.

The ES2K, DPDK, and Tofino targets have their own target-specific PortManager
subclasses and mocks. The `PortConfigParams` and `HotplugConfigParams` classes
are now part of the `DpdkPortManager` class.

## Documentation Changes

The `docs` directory has been reorganized and a number of files have been
renamed.

- Documentation for the command-line clients is in the `clients` subdirectory.
- The target-specific setup guides and the [security guide](/guides/security/security-guide.md)
  are in the `guides` subdirectory.

## Security Fixes

`c-ares` has been updated to v1.19.0 to address CVE-2022-4904.

## Coming Attractions

- Rename 'ES2K' to a more generic label
  - Will affect file, directory, and class names
- Add Stratum and Kernel Monitor unit tests
- Add/revise documentation
- Update Stratum to a more recent version
