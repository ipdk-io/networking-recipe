<!-- markdownlint-disable MD041 -->
[![P4 Control Plane CI Pipeline](https://github.com/ipdk-io/networking-recipe/actions/workflows/pipeline.yml/badge.svg)](https://github.com/ipdk-io/networking-recipe/actions/workflows/pipeline.yml)
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

The Stratum dependencies component has formally moved from the `setup`
directory to a new <https://github.com/ipdk-io/stratum-deps> repository.

See the [README file](setup/README.md) in the `setup` directory
for more information.

## Source code

To download the source code for P4 Control Plane:

```bash
git clone --recursive https://github.com/ipdk-io/networking-recipe
```

## Documentation

- [P4 Control Plane User Guide](https://ipdk.io/p4cp-userguide/)
