# Development Update 22.47

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

### DPDK port attributes

The DPDK chassis manager has been extensively revised.

- Moved Constant definitions to a separate hader file (`dpdk_port_constants.h`).
- Extracted port configuration management to a new `DpdkPortConfig` class.
- Removed support for the speed_bps, autoneg, fec_mode, and loopback port
  parameters, and reduced support for the mtu parameter.
- Normalized the log messages issued when port parameters are set.

### TdiPortManager class

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

### Kernel Monitor

The Kernel Monitor (`krnlmon`) receives RFC 3549 messages from the Linux
kernel over a Netlink socket when changes are made to the kernel networking
data structures. It updates the P4 tables defined by `linux_networking.p4`
to reflect these changes.

### OvS extensions

- _make-all.sh_
- _ovs build_
- _sidecar_
- _ovs-vswitchd_

## New Directories

- `p4src` contains p4 files and documentation
- `ptf-tests` contains end-to-end tests

## Bug Fixes

- Fixed a pervasive memory leak in TDI

## Coming Attractions

- Implement DpdkPortManager class
- Update Stratum to a more recent version
