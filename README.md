<!-- markdownlint-disable MD041 -->
[![P4CP build checks](https://github.com/ipdk-io/networking-recipe/actions/workflows/pipeline.yml/badge.svg)](https://github.com/ipdk-io/networking-recipe/actions/workflows/pipeline.yml)
<!-- markdownlint-enable MD041 -->
# P4 Control Plane

This repository is the superproject for P4 Control Plane.
It is the successor to P4-OVS.

- also called the IPDK Networking Recipe
- formerly known as P4-OVS Split Architecture

P4 Control Plane modularizes P4-OVS and reduces coupling between its
components, making the code easier to maintain and more suitable for
upstreaming.

It moves the P4-specific components of the architecture from `ovs-vswitchd`
to a separate process called `infrap4d`.

See the [Overview](https://ipdk.io/p4cp-userguide/overview/overview.html)
section of the User Guide for a description of the components.

## Major changes

### ovs-p4rt interface

The functions that ovs-p4rt publishes for use by OVS have been renamed
to make them more consistent with C programming conventions.

### Setup directory

The Stratum dependencies have moved from the `setup` directory
to a new <https://github.com/ipdk-io/stratum-deps> repository.

This change allows the dependencies to be downloaded and built independently
of the Networking Recipe (P4 Control Plane).
It also makes them easier to maintain.

See the [README file](https://github.com/ipdk-io/stratum-deps/blob/main/README.md)
and
[Transition Guide](https://github.com/ipdk-io/stratum-deps/blob/main/docs/transition-guide.md)
in the `stratum-deps` repository for more information.

## Source code

To download the source code for P4 Control Plane:

```bash
git clone --recursive https://github.com/ipdk-io/networking-recipe
```

## Documentation

- [P4 Control Plane User Guide](https://ipdk.io/p4cp-userguide/)

