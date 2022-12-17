# Building IPDK networking recipe for ES2K target

## Overview

This document describes how to build and run IPDK networking recipe on ES2K target.

## Setup

### Build P4 SDE for ES2K

```bash
git clone https://github.com/intel-innersource/networking.ethernet.acceleration.vswitch.p4-sde.p4-driver.git -b main --recursive p4-driver
```

For build instructions, refer to [P4 SDE Readme](https://github.com/intel-innersource/networking.ethernet.acceleration.vswitch.p4-sde.p4-driver/blob/main/README#building-and-installing)

### Install basic utilities

```bash
For Fedora distro: yum install libatomic libnl3-devel openssl-devel
For Ubuntu distro: apt install libatomic1 libnl-route-3-dev libssl-dev
pip3 install -r requirements.txt
```

### Build and install infrap4d dependencies

```bash
git clone --recursive https://github.com/intel-innersource/networking.ipu.mountevans.network-recipes.networking-recipe.git ipdk.recipe
cd ipdk.recipe
export IPDK_RECIPE=`pwd`
cd $IPDK_RECIPE/setup
cmake -B build -DCMAKE_INSTALL_PREFIX=<dependency install path> [-DUSE_SUDO=ON]
cmake --build build [-j<njobs>]
```

*Note*: If running as non-root user, provide `-DUSE_SUDO=ON` option to cmake
config.

### Build Networking Recipe

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

*Note*: By default, make-all.sh will create the `install` directory under the
networking recipe. User can specify a different directory using `--prefix`
option to make-all.sh. The following examples assume default `install`
directory for the executables. If not, user will need to specify the
appropriate path instead of ./install.

### Run Networking recipe

#### Set up the environment required by infrap4d

*Note*: `sudo` option is required when running copy_config_files.sh since
we are creating directories and copying config file at system level.

```bash
sudo ./scripts/es2k/copy_config_files.sh $IPDK_RECIPE $SDE_INSTALL
```

#### Set hugepages required for ES2K

Run the hugepages script.

```bash
sudo ./scripts/es2k/set_hugepages.sh
```

#### Export all environment variables to sudo user

```bash
alias sudo='sudo PATH="$PATH" HOME="$HOME" LD_LIBRARY_PATH="$LD_LIBRARY_PATH" SDE_INSTALL="$SDE_INSTALL"'
```

#### Run the infrap4d daemon

```bash
cd $IPDK_RECIPE
sudo ./install/sbin/infrap4d
```
 *Note*: By default, infrap4d runs in detached mode. If user wants to run infrap4d in attached mode, use --nodetach option.
All infrap4d logs are by default logged under /var/log/stratum.
All P4SDE logs are logged in p4_driver.log under $IPDK_RECIPE.
All OVS logs are logged under /tmp/ovs-vswitchd.log.

### Run a sample program

Open a new terminal to set the pipeline and try the sample P4 program.
Set up the environment and export all environment variables to sudo user.

```bash
source ./scripts/setup_env.sh $IPDK_RECIPE $SDE_INSTALL $DEPEND_INSTALL
./scripts/copy_config_files.sh $IPDK_RECIPE $SDE_INSTALL
alias sudo='sudo PATH="$PATH" HOME="$HOME" LD_LIBRARY_PATH="$LD_LIBRARY_PATH" SDE_INSTALL="$SDE_INSTALL"'
```
