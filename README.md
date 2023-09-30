# P4 Control Plane

This repository is the superproject for P4 Control Plane.
It is the successor to P4-OVS.

- also called the IPDK Networking Recipe
- formally known as P4-OVS Split Architecture

P4 Control Plane modularizes P4-OVS and reduces coupling between its
components, making the code easier to maintain and more suitable for
upstreaming.
It moves the P4-specific components of the architecture to a separate
process called `infrap4d`.

## Upcoming changes

The Stratum dependencies will be moving from the `setup` directory to their
their own repository toward the end of October 2023.

See the [README file](setup/README.md) for further information.

## Source code

To download the source code for P4 Control Plane:

```bash
git clone --recursive https://github.com/ipdk-io/networking-recipe
```

## Targets

P4 Control Plane can be configured at build time to support any one
of the following targets:

| Target | Instructions |
| ------ | ------------ |
| dpdk   | [DPDK Setup Guide](https://ipdk.io/p4cp-userguide/guides/setup/dpdk-setup-guide.html) |
| es2k   | [ES2K Setup Guide](https://ipdk.io/p4cp-userguide/guides/setup/es2k-setup-guide.html) |
| tofino | [Tofino Setup Guide](https://ipdk.io/p4cp-userguide/guides/setup/tofino-setup-guide.html) |

## Documentation

- [P4 Control Plane User Guide](https://ipdk.io/p4cp-userguide/)
