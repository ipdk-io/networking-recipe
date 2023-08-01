# Development Update 22.50

## Overview

This is an interim update ("code drop") to give the validation team
an opportunity to work with the latest DPDK features.

## What's Changed

### Highlights

- Overhauled DPDK port parameter handling
- Implemented TdiPortManager class
- Updated `p4rt-ctl` client
- Integrated Kernel Monitor
- Added OvS extensions
- Added support for ECMP and FRR
- Added support for debug builds
- Improved dependency setup script

### DPDK port attributes

The DPDK chassis manager has been extensively revised.

- Moved Constant definitions to a separate hader file (`dpdk_port_constants.h`).
- Extracted port configuration management to a new `DpdkPortConfig` class.
- Removed support for the speed_bps, autoneg, fec_mode, and loopback port
  parameters, and reduced support for the mtu parameter.
- Normalized the log messages issued when port parameters are set.

### TdiPortManager class (Tofino only)

Extracted a new `TdiPortManager` class from `TdiSdeWrapper`, to interface
to the port management functions of the underlying P4 Driver. This class is
currently implemented for the Tofino target only.

### p4rt-ctl

Added the following commands:

- add-action-profile-member
- delete-action-profile-member
- get-action-profile-member
- add-action-profile-group
- delete-action-profile-group
- get-action-profile-member

The utility should now be on par with P4-OVS except counter support.

### Kernel Monitor (DPDK only)

The Kernel Monitor (`krnlmon`) receives RFC 3549 messages from the Linux
kernel over a Netlink socket when changes are made to the kernel networking
data structures. It updates the P4 tables defined by `linux_networking.p4`
to reflect these changes.

The Kernel Monitor is now built as part of `infrap4d` for DPDK targets.

### OvS extensions

- The `make-all.sh` script now has `--ovs` and `--no-ovs` options to specify
  whether to build with Open vSwitch support.

- The DPDK target builds a library (the _OVS Sidecar_) which it links with
  `ovs-vswitchd` to allow OVS to issue P4Runtime requests to `infrap4d`.

### ECMP and FRR support

The P4 program and the Kernel Monitor have been updated to support
Equal Cost Multipath (ECMP) and FRR (Free Range Routing). See the
[ECMP guide](https://github.com/ipdk-io/networking-recipe/blob/main/p4src/linux_networking/README_LINUX_NETWORKING_WITH_ECMP.md)
for more information.

### Debug builds

CMake builds the networking recipe in the CMake `RelWithDebInfo` (Release
With Debug Info) configuration by default. Specify the `--debug` parameter
to `make-all.sh` to build with debug symbols (`Debug` configuration).

## New Directories

- `krnlmon` contains the kernel monitor
- `ovs-p4rt` contains the OvS sidecar
- `p4src` contains p4 files and documentation
- `ptf-tests` contains end-to-end tests
- `scripts/dpdk` contains dpdk-specific scripts for setting up the environment

## Dependency setup script

The setup script (`setup/CMakeLists.txt`) fails when building Protobuf
with CMake version 3.24 and 3.25. The script now aborts if CMake version
is 3.24 or later.

The script now specifies CMAKE_PREFIX_PATH when building gRPC and Protobuf,
to ensure that they link against the newly built versions of other dependent
libraries.

## Documentation Updates

- The [p4rt-ctl](/clients/p4rt-ctl) and [gnmi-ctl](/clients/gnmi-ctl)
guides have been updated with lists of known issues.

- The [DPDK setup guide](/guides/setup/dpdk-setup-guide.md)
has been extensively revised.

## Security Fixes

The setup script now installs zlib v1.2.13, to address CVE-2018-25032 and
CVE-2022-37434. zlib is used by gRPC and Protobuf.

## Bug Fixes

- Fixed a pervasive memory leak in TDI.

- Test automation did not work with `infrap4d` because the daemon could not be
  started to run in the background. This problem has been fixed.

- P4Runtime was not returning the proper error when adding or deleting rules
  multiple times. The server now returns an appropriate error indicating
  duplicate or missing entries.

- gNMI now returns an error if the MTU parameter for a port is out of range.

## Coming Attractions

- Implement DpdkPortManager class
- Update Stratum to a more recent version
