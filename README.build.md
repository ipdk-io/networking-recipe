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
./make-all.sh
```

The `make-all` script creates a separate install tree for OVS.

This allows you to rebuild the non-OVS code without having to rebuild OVS.
