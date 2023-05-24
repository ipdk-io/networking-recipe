# Development Update 23.21

## Overview

This is an update ("code drop") to incorporate changes made to support
the Intel IPU 2100.

## What's Changed

### Highlights

- Implemented ES2K target
- Rewrote make-all.sh script
- Completed PortManager refactoring
- Forked P4Runtime repository
- Changed default target to DPDK
- Added `sgnmi_cli` client

### ES2K Target

This update adds support for a new target, the Intel IPU 2100.

The new target is referred to in software as "ES2K". There are plans to
change this label to something less ASIC-specific prior to the product's
actual release.

As the third target implemented, the first engineered as an IPU, and the first
to require cross-compilation, it has had an impact throughout the codebase.

Component changes:

- Add ES2K TDI target to Stratum
- Add ES2K support to Kernel Monitor
- Add ES2K support to ovs-p4rt

Build system changes:

- Support separate host and target dependencies
- Support SYSROOT and STAGING directories
- Implement aarch64 toolchain file
- Move P4 SDK/SDE definitions to individual include files
- Improve RPATH (RUNPATH) support
- Support variations in system library locations
- Add `make-cross-ovs.sh` and `config-cross-recipe.sh` help scripts to
  assist with cross-compiling P4 Control Plane

Setup changes:

- Allow dependencies to be built and installed for the Host system (full
  or minimal) or cross-compiled for the Target system
- Remove GMOCK_GBL from dependencies
- Add CMake option to suppress download
- Allow CXX standard to be specified for Abseil and gRPC
- Add `make-cross-deps.sh` and `make-host-deps.sh` helper scripts to assist
  with building host and cross-compiled dependencies

### make-all.sh

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

### PortManager classes

This update completes the process of extracting the `TdiPortManager` class
from `TdiSdeInterface`. `TdiPortManager` now has its own header file, and
the ES2K, DPDK, and Tofino targets have their own target-specific subclasses
and mocks.

## Components

### Kernel Monitor

The Kernel Monitor as been updated to support ES2K.

krnlmon is enabled by default for DPDK and ES2K builds. It can be suppressed
by specifying `--no-krnlmon` on the `make-all.sh` command line.

### OVS

Open vSwitch (OVS) and the OVS P4Runtime Library (`ovs-p4rt`) are enabled by
default for DPDK and ES2K builds. They can be suppressed by specifying
`--no-ovs` on the `make-all.sh` command line.

When OVS is enabled, P4 Control Plane builds Open vSwitch with P4 extensions.
It also builds a library (`ovs-p4rt`) that is linked with `ovs-vswitchd` to
allow OVS to issue P4Runtime requests to `infrap4d`.

ovs-p4rt has been extended to support ES2K.

## Documentation Changes

The `docs` directory has been reorganized and a number of files have been
renamed.

## Security Fixes

`c-ares` has been updated to v1.19.0 to address CVE-2022-4904.

## Coming Attractions

- Rename 'ES2K' to a more generic label
  - Will effect file, directory, and class names
- Update Stratum to a more recent version
