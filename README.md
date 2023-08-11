# P4 Control Plane

P4 Control Plane modularizes P4-OVS and reduces coupling between its
components, making the code easier to maintain and more suitable for
upstreaming.
It moves the P4-specific components of the architecture to a separate
process called `infrap4d`.

P4 Control Plane is also called the Networking Recipe, and was formerly
known as P4-OVS Split Architecture.

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
| dpdk   | [DPDK Setup Guide](docs/guides/setup/dpdk-setup-guide.md) |
| es2k   | [ES2K Setup Guide](docs/guides/setup/es2k-setup-guide.md) |
| tofino | [Tofino Setup Guide](docs/guides/setup/tofino-setup-guide.md) |

## Documentation

- [P4 Control Plane User Guide](https://ipdk.io/p4cp-userguide/)
