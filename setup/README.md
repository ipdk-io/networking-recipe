# Stratum Dependencies

Stratum is the component of `infrap4d` that implements the P4Runtime and gNMI
(OpenConfig) services.

This directory allows you to build and install the third-party libraries
that Stratum requires.

<!-- markdownlint-disable-next-line -->
## We're Moving!

The Stratum dependencies are being relocated to their own repository,
<https://github.com/ipdk-io/stratum-deps>.

This allows the dependencies to be updated independently of the Networking
Recipe (P4 Control Plane).

### Development

New development is currently taking place in the `stratum-deps` repository.

See the
[change history](https://github.com/ipdk-io/stratum-deps/blob/main/docs/change-history.md)
for more information.

### Transition

We plan to phase out the `networking-recipe/setup` directory toward the end
of October 2023.

If you would like to start using `stratum-deps` before then,
[version 1.2.0](https://github.com/ipdk-io/stratum-deps/tree/v1.2.0)
is a good place to start.

### Documentation

The `stratum-deps` repository includes updated versions of relevant sections
of the user manual, plus new documentation on the helper scripts.

See the repository's
[README file](https://github.com/ipdk-io/stratum-deps/blob/main/README.md)
for links to the documentation.
