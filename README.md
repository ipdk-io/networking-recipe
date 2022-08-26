# networking-recipe

IPDK Networking Recipe (P4-OVS Split Architecture)

## Background

The initial implementation of P4-OVS was both physically and logically
monolithic.

- Two foreign components (Stratum and an RFC 3594 Kernel Monitor) were added
  to a fork of the Open vSwitch repository.

- OvS was modified to build these components and include them in `ovs-vswitchd`.

- The Stratum initialization code (`hal.cc` and `main_bfrt.cc`) was adapted
  to create two new modules (`bf_interface.cc` and `p4_service_interface.cc`)
  that OvS could use to start the P4 Runtime and gNMI services.
  
- Extensive changes were made to other parts of the Stratum code, to
  support a P4-enabled IPDK hardware or software switch.

## Split Architecture

The Split Architecture is an effort to modularize P4-OVS, making it easier
to upstream and maintain.

- Remove foreign components from the OvS repository, leaving only the logic
  needed to support P4. Make the changes suitable for upstreaming to the
  parent project.

- Reengineer the Stratum modifications to be non-breaking. Make the changes
  suitable for upstreaming to the parent project.

- Make the Kernel Monitor a separate component. Remove OvS dependencies.
  Refactor for modularity and to support unit testing.

- Create a component that combines Stratum, the Kernel Monitor, TDI, and the
  P4 driver into a separate daemon (`infrap4d`).

- Create a sidecar component (`ovs-p4rt`) that can be linked into
  `ovs-vswitchd` to allow OvS to communicate with `infrap4d`.

- Create a superproject (`networking-recipe`) to incorporate the components
  and orchestrate the overall build.

- Incorporate changes to the OvS and Stratum components that have been
  made in P4-OVS since the initial split was done.

- Remove OvS and Stratum changes that are not applicable to the initial
  release, to limit the amount of unused code.

- Bring the OvS and Stratum code up to date, rebasing the P4OVS changes
  onto the heads of the original projects.

- Upstream the OvS and Stratum changes to their parent projects.

Implementation will be done incrementally.

## Changes from P4-OVS

- The `external`, `p4runtime`, `stratum`, and `unit_test` directories will
  be removed from the `ovs` repository.

- The `p4proto` directory will be stripped down and most of its contents
  will be removed. The local Stratum files will move back into Stratum
  itself. Some of the changes will be discarded. Platform-specific changes
  will become part of the new `ipdk` and `tdi` platforms. `bf_interface`
  and `p4_service_interface` will revert to the platform-specific `hal`
  and `main` files.

- OvS will no longer launch the Kernel Monitor or the Stratum P4 and Config
  Monitoring services.

- OvS will no longer be able to make direct calls to the Kernel Monitor,
  the P4 switch driver, or TDI. All communication will be by means of C
  calls to the OvS sidecar (`ovs-p4rt`), which will communicate with
  `infrap4d` via gRPC requests.

## Repositories  

- OvS development will take place on the `split-arch` branch of the
  `ipdk-io/ovs` repository.

- Stratum development will take place on the `split-arch` branch of the
  `ipdk-io/stratum-dev` repository.

- Kernel Monitor development will take place on the `main` branch of the
  `ipdk-io/krnlmon` repository.

- Superproject development will take place on the `main` branch of the
  `ipdk-io/networking-recipe` repository.
