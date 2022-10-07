# Building the IPDK Networking Recipe

## Download

```bash
git clone https://github.com/ipdk-io/networking-recipe
cd networking-recipe
git submodule update --init --recursive
```

## Setup

Define the path to the SDE install directory.

```bash
export SDE_INSTALL=<path>
```

## Build

```bash
./make-all.sh [--target {tofino|dpdk}] [--prefix <prefix>]
```

Defaults:
* `--target` currently defaults to `tofino`
* `--prefix` currently defaults to `./install`

See [Building IPDK for Tofino](docs/ipdk-tofino.md) for more information.

Support for the DPDK target is currently **under construction**.
