# Building IPDK for Tofino

## Overview

This document describes how to build and run IPDK on Tofino HW. Similar steps apply when run on Tofino simulation model. Please refer to Tofino specific documentation for more details.

## Requirements

- Docker (Ubuntu 20.04 base image)
- Intel P4Studio
- Git

## Setup

The build and execution of infrap4d uses docker.

### Docker

Pull the Ubuntu 20.04 docker base image

```bash
docker pull ubuntu:20.04
```

Create the container

```bash
docker run --privileged --cap-add=ALL -it --name infrap4d --network host -d ubuntu:20.04 /bin/bash
```

### Git

Clone IPDK networking-recipe

```bash
git clone --recursive git@github.com:ipdk-io/networking-recipe.git ipdk.recipe
```

### Intel P4Studio

Get Intel P4Studio SDE. The untarred directory is henceforth referred to as **sde**. If using an Intel Tofino Reference platform, please also download the BSP package.

```bash
tar xvf bf-sde-9.11.0-cpr.tgz
```

## Build

Build is done within the infrap4d docker container created above.

```bash
docker exec -it infrap4d bash
```

### Install basic utilities

```bash
apt-get update
apt-get install sudo git cmake autoconf gcc g++ libtool python3 python3-dev python3-distutils iproute2 libssl-dev
```

### Build and install infrap4d dependencies

```bash
cd ipdk.recipe
export IPDK_RECIPE=`pwd`
cd $IPDK_RECIPE/setup
cmake . -DCMAKE_INSTALL_PREFIX=/usr/local/
make
ldconfig
```

### Build Intel P4Studio SDE

The below steps are minimal. For a more detailed installation procedure, please refer to the Intel P4Studio SDE Installation Guide.

```bash
cd sde/p4studio
./p4studio profile apply profiles/ipdk.yaml --bsp-path **path-to-bsp**
cd ..
export SDE_INSTALL=sde/install
```

> Note: To test with tofino-model, please edit profiles/ipdk.yaml file and change the **asic** configuration from true -> false. Additionally, a bsp-path would also not be required for tofino-model.

### Build Networking Recipe

```bash
cd $IPDK_RECIPE
./make-all.sh --dep-install /usr/local/ --sde-install $SDE_INSTALL --target=tofino
```

## Run

```bash
cd IPDK_RECIPE
cp $SDE_INSTALL/share/bf_switchd/zlog-cfg $IPDK_RECIPE/zlog-cfg-cur
mkdir -p /etc/stratum/
mkdir -p /var/log/stratum/

LD_LIBRARY_PATH=$IPDK_RECIPE/install/lib/:$SDE_INSTALL/lib \
    ./install/sbin/infrap4d \
    -chassis_config_file=./stratum/stratum/stratum/hal/config/x86-64-accton-wedge100bf-32x-r0/chassis_config.pb.txt \
    -tdi_switchd_cfg=$SDE_INSTALL/share/p4/targets/tofino/tofino_skip_p4.conf \
    -tdi_sde_install=$SDE_INSTALL
```

> Note: To test with tofino-model, please use **./stratum/stratum/stratum/hal/config/barefoot-tofino-model/chassis_config.pb.txt** as the chassis_config_file.

## Set forwarding pipeline

Following are optional steps to perform set forwarding pipeline.

The following steps describe generating a p4info and pipeline configuration required to perform a set forwarding pipeline from a controller.

Tofino only supports the TNA architecture. The bf-p4c compiler can still generate PSA compatible P4info files.

The tna_exact_match P4 example is used to describe these steps. The same steps will apply for any TNA program.

```bash
cd sde/build
make tna_exact_match
make install
```

The above commands will generate 4 files. The last 3 files are combined together by the TDI pipeline builder to generate a single bin file to be pushed from the controller.

These files can be found in install/share/tofinopd/tna_exact_match/ directory.

- tna_exact_match.p4info.pb.txt
- tofino.bin
- context.json
- bf-rt.json

A conf file is also generated. This file holds the location of the above files relative to SDE_INSTALL.

### Using tdi_pipeline_builder

To generate the bin file for the controller.

```bash
cd $SDE_INSTALL
LD_LIBRARY_PATH=$IPDK_RECIPE/install/lib/:$SDE_INSTALL/lib \
    $IPDK_RECIPE/install/bin/tdi_pipeline_builder \
    -p4c_conf_file=$SDE_INSTALL/share/p4/targets/tofino/tna_exact_match.conf \
    -bf_pipeline_config_binary_file=$IPDK_RECIPE/tna_exact_match.pb.bin
```
