# Development Update 24.07

## Overview

This is an update (code drop) to incorporate changes made to support
the Intel IPU E2100.

## Breaking Changes

### Client Flags

- The Stratum `CredentialsManager` class and `gnmi_cli` tool have been aligned
  with the current upstream versions, and IPDK modifications have been
  revised to be more consistent with Stratum as a whole.

  The `-grpc_client_cert_req_type` flag is now `-client_cert_req_type`, and
  its values have been shortened. They are also now case-insensitive.

  | Old Name | New Name |
  | -------- | -------- |
  | NO_REQUEST_CLIENT_CERT | no_request |
  | REQUEST_CLIENT_CERT_NO_VERIFY | request_no_verify |
  | REQUEST_CLIENT_CERT_AND_VERIFY | request_and_verify |
  | REQUIRE_CLIENT_CERT_NO_VERIFY | require_no_verify |
  | REQUIRE_CLIENT_CERT_AND_VERIFY | require_and_verify |

  This change affects all clients that use CredentialsManager, including
  `gnmi_cli`, `sgnmi_cli`, and `gnmi-ctl`.

## What's Changed

## Component Changes

### Kernel Monitor

### OVS

### Stratum

Stratum has been updated to the latest version. All changes made between
3-Jun-2021 and 7-Dec-2023 have been downstreamed and integrated into
P4 Control Plane.

The majority of the changes were either global housekeeping (such as
formatting files with clang-format and buildifier) or do not affect
P4 Control Plane (such as updates to the `barefoot` platform, of which
there were many).

Changes include:

- Defaulting to `--test_output=errors` when running `bazel test`, so the
  the test log becomes part of the console output if there are errors.

- Adding support for the OpenConfig `/interfaces/interface[name=*]/state/id`
  path.

- Replacing the raw text proto string terminal `PROTO` with `pb`. This
  fixes compiler errors in newer versions of gcc and clang.

- Replacing P4Runtime controller management with controller code from the
  PINS project.

- Adding support for P4Runtime role configuration.

- Adding new unit tests and updating existing tests.

- Overhauling `lib/security` (credentials management).

- Improvements to log output.

For more information, see the commit comments in the upstream repository.

We have also made minimal updates to the `barefoot` platform so we can
build it with version 9.11.0 of the Barefoot SDE (the version used to
build the TDI `tofino` target).

Note that we have _not_ attempted to migrate changes made in the Barefoot
platform to the TDI platform (which is based on Barefoot).
This can be done later, if the need arises.

## Documentation Changes

## Security Fixes

## P4Runtime Fork

P4 Control Plane uses a fork of the `p4runtime` repository to add provisional
support for the PacketModMeter and DirectPacketModMeter externs.

We plan to revert to using the standard `p4runtime` repository when the
additions become standard.

Build artifacts for Python, Go, and C++ can be downloaded from the
[Releases page](https://github.com/ipdk-io/p4runtime-dev/releases)
of the repository. The current version is 2023.11.0.

If you wish to build the protobuf artifacts yourself, see the
[README file](https://github.com/ipdk-io/networking-recipe/blob/main/protobufs/README.md)
for instructions.

## Coming Attractions

- Combined Networking and IPsec recipe
- LAG-in-LACP mode
- Geneve tunnels
- Default actions
