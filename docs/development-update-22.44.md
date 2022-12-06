# Development Update 22.44

## Overview

This is an interim update ("code drop") to give the validation team
an opportunity to take the DPDK code out for a test drive.

## Project Name

Note that the P4-OVS Split Architecture has a new name: **P4 Control Plane**.

## What's Changed

### Highlights

- Restructured Stratum to support TDI variants
- Integrated DPDK support from P4-OVS
- Disabled unsupported port attributes in DPDK
- Updated OVS to newer release
- Replaced `gnmi_cli` with `gnmi-ctl` in DPDK build

### TDI variants

When we compared the Stratum files in P4-OVS with their Tofino equivalents,
we found significant changes in the ChassisManager, Hal, Main, and Switch
classes, as well as SdeWrapper and YangParseTreePaths.

We addressed the first four classes by creating *dpdk* and *tofino*
subdirectories under the *tdi* directories, and giving each target its
own implementation of the class.

For SdeWrapper, we refactored `tdi_sde_wrapper.cc` (\~3800 lines) into
multiple, smaller source files, isolating the target-specific methods
in a single file. We then created separate versions of this file for
the DPDK and Tofino targets, and moved them to the target-specific
subdirectories

For YangParseTreePaths, we refactored `yang_parse_tree_paths.cc`
(originally \~4100 lines, increasing to \~5600 lines in P4-OVS) into
multiple, smaller source files, separating out sections of code that
had been heavily modified. We then created alternate versions for DPDK.
See [Refactoring YangParseTreePaths](../stratum/stratum/stratum/docs/refactoring_yang_parse_tree_paths.md)
for details.

This restructuring allowed us to port DPDK code from P4-OVS with minimal
impact on the existing Tofino code.

### DPDK support

The P4 Control Plane software can now be built for either Tofino or
DPDK. Features implemented by Stratum (P4Runtime and OpenConfig) should
be on par with P4-OVS.

Additional development work is required to support the kernel monitor
and to allow OVS to communicate with *infrap4d*.

To build DPDK under CMake, you must first set the `SDE_INSTALL`
environment variable to point to the root of the P4-DPDK install tree.
Then, from the top-level directory of the networking recipe, enter

```bash
./make-all --target=dpdk
```

to perform the build.

If you are familiar with conventional Stratum builds, you can build the
DPDK version of Stratum by setting `SDE_INSTALL` or `DPDK_INSTALL` to
the root of the P4-DPDK install tree. Then change to the root of the
stratum submodule (`stratum/stratum` under the top of the networking recipe)
and enter:

```bash
bazel build //stratum/hal/bin/tdi/dpdk:stratum_dpdk --define target=dpdk
```

You can build the Tofino version by issuing one of the above commands,
substituting `tofino` for `dpdk`.

### DPDK port attributes

The following leaves are not applicable to DPDK, and are no longer
supported in the DPDK build:

- auto-negotiate
- forwarding-viable
- hardware-port
- health-indicator
- last-change
- loopback-mode
- negotiated-port-speed
- oper-status
- port-speed
- system-priority

The following leaves have been renamed, to reduce ambiguity or
for consistency with other leaves:

- `host` is now `host-name`
- `hotplug` is now `qemu-hotplug-mode`
- `qemu-vm-mac` is now `qemu-vm-mac-address`

### OvS Update

OvS has been updated from January 2022 to a more recent version.

## gnmi-ctl

For DPDK builds, we have replaced `gnmi_cli` with `gnmi-ctl`, which is
the new name for the modified version of gnmi_cli that was part of P4-OVS.

For Tofino builds, we install the unmodified version of `gnmi_cli`.

## Known Issues

- The Teardown test cases in `dpdk_hal_test` are currently failing.

- When tunnel is enabled, it is expected to have total size of the tunnel
  packet less than or equal to 1514 Bytes. To match this size, user need to
  adjust overlay network interface MTU size not more than 1450 Bytes.
  Underlay fragmentation is not support, so we need to make sure packet is
  with in the MTU size of the underlay port.

- For any udp/tcp packets from overlay network, if checksum issues are
  noticed on interfaces which are of type VIRTIO-NET, it is recommended to
  disable checksum using below command.
  $ ethtool --offload <netdev-name> rx off tx off

- pna_tcp_connection_tracking demonstrates the PNA add_on_miss feature and
  flow aging for auto learn flows. It supports partial implementation of TCP
  state machine.

## Coming Attractions

- Update Stratum to a more recent version
- Integrate Kernel Monitor
- Overhaul DPDK port parameter handling
