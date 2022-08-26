# networking-recipe

IPDK Networking Recipe

## Download

```bash
git clone https://github.com/ipdk-io/networking-recipe
cd networking-recipe
git submodule update --init --recursive
```

## Patch

The OVS changes depend on modifications to the OVSDB schema that cannot be
upstreamed in their present form. In order to build OVS, you will need to
apply a patch.

```bash
pushd ovs/ovs
git patch ../../patch/vswitch-schema.patch
popd
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
