<!-- markdownlint-disable MD026 -->
# We've Moved!
<!-- markdownlint-enable MD026 -->

The Stratum dependencies have formally moved from the `setup` directory
to a new <https://github.com/ipdk-io/stratum-deps> repository.

This change allows the dependencies to be downloaded and built independently
of the Networking Recipe (P4 Control Plane).
It also makes them easier to maintain.

See the [README file](https://github.com/ipdk-io/stratum-deps/blob/main/README.md)
in the `stratum-deps` repository for more information.

## Transition

We are retaining the `setup` directory for now, to allow users time to
to transition to the `stratum-deps` repository. Note that this is an older
version of the component and does not include any recent improvements.

We will be removing the `setup` directory in the near future.
You are encouraged to transition to `stratum-deps` as soon as possible.

See the
[Transition Guide](https://github.com/ipdk-io/stratum-deps/blob/main/docs/transition-guide.md)
for details.
