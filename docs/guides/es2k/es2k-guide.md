# ES2K Setup Guide for P4 Control Plane

## Overview

This document explains how to install, build, and run P4 Control Plane
for the ES2K target.

## Setup

### Install basic utilities

```bash
For Fedora distro: yum install libatomic libnl3-devel openssl-devel
For Ubuntu distro: apt install libatomic1 libnl-route-3-dev libssl-dev
pip3 install -r requirements.txt
```

### Install P4 SDE

Obtain a copy of the SDE for the Intel IPU E2100 and install it on your
development system. A common install location is '/opt/p4/p4sde`.

### Build and install infrap4d dependencies

```bash
git clone --recursive https://github.com/ipdk-io/networking-recipe.git ipdk.recipe
cd ipdk.recipe
export IPDK_RECIPE=`pwd`
cd $IPDK_RECIPE/setup
cmake -B build -DCMAKE_INSTALL_PREFIX=<dependency install path> [-DUSE_SUDO=ON]
cmake --build build [-j<njobs>]
```

*Note*: If running as non-root user, provide `-DUSE_SUDO=ON` option to cmake
config.

### Build P4 Control Plane

#### Set environment variables

- export DEPEND_INSTALL=`absolute path for installing dependencies`
- export SDE_INSTALL=`absolute path for p4 sde install built in previous step`

```bash
source ./scripts/es2k/setup_env.sh $IPDK_RECIPE $SDE_INSTALL $DEPEND_INSTALL 
```

#### Compile the recipe

```bash
cd $IPDK_RECIPE
./make-all.sh --target=es2k
```

*Note*: By default, `make-all.sh` will create an `install` directory under the
networking recipe. User can specify a different directory using `--prefix`
option to make-all.sh. The following examples assume default `install`
directory for the executables. If not, user will need to specify the
appropriate path instead of ./install.

### Run Infrap4d

See [Run Infrap4d guide](https://github.com/ipdk-io/networking-recipe/blob/main/docs/guides/es2k/run-infrap4d.md) for step by step guide to run infrap4d on ACC on Intel IPU E2100
