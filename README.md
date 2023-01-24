# IPDK Networking Recipe (P4 Control Plane)

## Overview

The IPDK Networking Recipe (originally P4-OVS Split Architecture)
modularizes P4-OVS and reduces coupling between its components, making the
code easier to maintain and more suitable for upstreaming. It moves the
P4-specific components of the integrated architecture of P4-OVS to a separate
process called infrap4d.

![Networking Recipe Architecture](docs/images/networking-recipe-architecture.png)

## infrap4d

Infrap4d integrates Stratum, the Kernel Monitor (krnlmon), Switch Abstraction
Interface (SAI), Table Driven Interface (TDI), and a P4 target driver into a
separate process (daemon).

![Infrap4d Architecture](docs/images/infrap4d-architecture.png)

## Stratum

Stratum is an open-source silicon-independent switch operating system.
It is a component of Infrap4d that provides the P4Runtime and gNMI/Openconfig
capabilities for P4 flow rule offloads and port configuration offloads.
Stratum is augmented with a new tdi platform layer that processes P4rt and
gNMI requests and interacts with the underlying P4 target driver through TDI.
A new ipdk platform layer provides IPDK-specific replacements for several
TDI modules that allow it to handle configuration differences between IPUs
and the switches for which Stratum was developed.

## Table Driven Interface (TDI)

TDI (Table Driven Interface) provides a target-agnostic interface to the
driver for a P4-programmable device. It is a set of APIs that enable
configuration and management of P4 programmable and fixed functions of a
backend device in a uniform and dynamic way. Different targets like bmv2
and P4-DPDK can choose to implement their own backends for different P4
and non-P4 objects but can share a common TDI. Stratum talks to the
target-specific driver through the TDI front-end interface.

## Kernel Monitor (krnlmon)

The Kernel Monitor receives RFC 3549 messages from the Linux Kernel over a
Netlink socket when changes are made to the kernel networking data structures.
It listens for network events (link, address, neighbor, route, tunnel, etc.)
and issues calls to update the P4 tables via SAI and TDI. The kernel monitor
is an optional component of infrap4d.

## Switch Abstraction Interface (SAI)

Switch Abstraction Interface (SAI) defines a vendor-independent interface
for switching ASICs.

## Interfaces

### P4Runtime

The P4Runtime API is a control plane specification for managing the
data plane elements of a device defined or described by a P4 program.

### gNMI

gRPC Network Management Interface (gNMI) is a gRPC-based protocol to manage
network devices.

## Clients

1. `ovs-p4rt`: A library (C++ with a C interface) that allows ovs-vswitchd
   and ovsdb-server to communicate with the P4Runtime Server in infrap4d
   via gRPC. It is used to program (insert/modify/delete) P4 forwarding
   tables in the pipeline.

2. `p4rt-ctl`: A Python-based P4Runtime client which talks to the P4Runtime
   Server in infrap4d via gRPC, to program the P4 pipeline and insert/delete
   P4 table entries.

3. `gnmi-ctl`: A gRPC-based C++ network management interface client to handle
   port configurations and program fixed functions in the P4 pipeline.

## Download

To download the source code for the Networking Recipe:

```bash
git clone --recursive https://github.com/ipdk-io/networking-recipe
```

## Targets

The IPDK Networking Recipe can be built to support different targets.
See the target-specific instructions for information on how to set up,
build, and use a particular target.

| Target | Instructions |
| ------ | ------------ |
| dpdk   | [IPDK Networking Recipe for DPDK](https://github.com/ipdk-io/networking-recipe/blob/main/docs/ipdk-dpdk.md) |
| tofino | [IPDK Networking Recipe for Tofino](https://github.com/ipdk-io/networking-recipe/blob/main/docs/ipdk-tofino.md) |

## make-all.sh

The `make-all.sh` script provides a convenient way to build the
Networking Recipe for a specific target.

```bash
./make-all.sh [--ovs] -target <target>
```

### General options

| Parameter | Value | Description |
| --------- | ----- | ----------- |
| `--prefix` |  _path_ | Path to the directory in which build artifacts should be installed. Sets the  `CMAKE_INSTALL_DIR` CMake variable. Default value is `./install`. |
| `--sde-install` | _path_ | Path to install directory for the target driver (SDE). Sets the `SDE_INSTALL_DIR` CMake variable. Defaults to the value of the `SDE_INSTALL` environment variable. |
| `--target` | _target_ | Target to build for (`dpdk` or `tofino`). Sets the `DPDK_TARGET` or `TOFINO_TARGET` CMake variable. Currently defaults to `tofino`. |

Parameter names may be abbreviated to any shorter form as long as it is unique.

### Developer options

| Parameter | Value | Description |
| --------- | ----- | ----------- |
| `--clean` | | Remove main _build_ and _install_ directories, then build. |
| `--debug` | | Build with debug configuration. |
| `--dep-install` | _path_ | Path to an optional install directory for dependency libraries. Sets the `DEPEND_INSTALL_DIR` CMake variable. Defaults to the value of the `DEPEND_INSTALL` environment variable. |
| `--develop` | | Create separate build and install trees for OVS (`ovs/build` and `ovs/install`). The `--clean` option does not remove these directories. This allows you to do a clean build of the non-OVS code without having to rebuild OVS. |
| `--no-ovs` | Disable support for Open vSwitch (OvS). |
| `--ovs` | | Enable support for Open vSwitch (OvS). |

These options are primarily of interest to developers working on the recipe.

## Note

The build files, CMake variables, environment variables, and `make-all`
script are under active development and are expected to change.
