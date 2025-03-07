<!-- markdownlint-disable MD041 -->
[![P4CP build checks](https://github.com/ipdk-io/networking-recipe/actions/workflows/pipeline.yml/badge.svg)](https://github.com/ipdk-io/networking-recipe/actions/workflows/pipeline.yml)
[![P4CP lint checks](https://github.com/ipdk-io/networking-recipe/actions/workflows/linters.yml/badge.svg)](https://github.com/ipdk-io/networking-recipe/actions/workflows/linters.yml)
<!-- markdownlint-enable MD041 -->
# P4 Control Plane

This repository is the superproject for P4 Control Plane.
It is the successor to P4-OVS.

- also called the IPDK Networking Recipe
- formerly known as P4-OVS Split Architecture

See the [Overview](https://ipdk.io/p4cp-userguide/overview/overview.html)
section of the User Guide for a description of its components.

## Major changes

### Linux Networking

The Networking Recipe now supports version 3 of the ES2K Linux Networking
P4 Program. Support for version 2 has been discontinued.

### `ovs-p4rt` interface

The public functions in the ovs-p4rt API have been renamed to make them
more consistent with C programming conventions.

### `docs` directory

The documentation files have moved from the `docs` directory to the
<https://github.com/ipdk-io/p4cp-userguide> repository.

### `setup` directory

The Stratum dependencies have moved from the `setup` directory
to the <https://github.com/ipdk-io/stratum-deps> repository.

## Source code

To download the source for P4 Control Plane:

```bash
git clone --recursive https://github.com/ipdk-io/networking-recipe
```

## Documentation

See the link in the **About** panel on the right-hand side of the
[home page](https://github.com/ipdk-io/networking-recipe).
