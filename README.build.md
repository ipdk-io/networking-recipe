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
./make-all.sh --target tofino
```

## General options

| Parameter | Value | Description |
| --------- | ----- | ----------- |
| `--prefix` |  _path_ | Path to the directory in which build artifacts should be installed. Sets the  `CMAKE_INSTALL_PREFIX` CMake variable. Default value is `./install`. |
| `--sde-install` | _path_ | Path to install directory for the target driver (SDE). Sets the `SDE_INSTALL_PREFIX` CMake variable. Defaults to the value of the `SDE_INSTALL` environment variable. |
| `--target` | _target_ | Target to build for (`dpdk` or `tofino`). Sets the `DPDK_TARGET` or `TOFINO_TARGET` CMake variable. Currently defaults to `tofino`. |

Parameter names may be abbreviated to any shorter form as long as it is unique.

## Developer options

| Parameter | Value | Description |
| --------- | ----- | ----------- |
| `--clean` | | Remove main _build_ and _install_ directories, then build. |
| `--dep-install` | _path_ | Path to an optional install directory for dependency libraries. Sets the `DEPEND_INSTALL_PREFIX` CMake variable. Defaults to the value of the `DEPEND_INSTALL` environment variable. |
| `--develop` | | Create separate build and install trees for OVS (`ovs/build` and `ovs/install`). The `--clean` option does not remove these directories. This allows you to do a clean build of the non-OVS code without having to rebuild OVS. |

These options are primarily of interest to developers working on the recipe.

## Targets

| Target | Description |
| ------ | ----------- |
| dpdk   | _Under construction._ |
| tofino | See [Building IPDK for Tofino](docs/ipdk-tofino.md) for more information. |

## Note

The build files, CMake variables, environment variables, and `make-install`
script are under active development and are expected to change.
