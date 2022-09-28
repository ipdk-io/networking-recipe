Build IPDK on tofino
====================

# Overview
This document describes how to build and run IPDK on Tofino HW. Similar steps apply when run on Tofino simulation model. Please refer to Tofino specific documentation for more details.

# Requirements
- Docker (Ubuntu 20.04 base image)
- Intel P4Studio
- Git

# Setup
The build and execution of infrap4d uses docker.

## Docker

### Pull the Ubuntu 20.04 docker base image
> docker pull ubuntu:20.04

### Create container
> docker run --privileged --cap-add=ALL -it --name infrap4d --network host -d ubuntu:20.04 /bin/bash

## Git

### Clone IPDK networking-recipe
> git clone --recursive git@github.com:ipdk-io/networking-recipe.git ipdk.recipe

## Intel P4Studio
Get the Intel P4Studio SDE version 9.11. The untarred directory is henceforth referred to as **sde**. If using an Intel Tofino Reference platform, please also download the BSP package.

> tar xvf bf-sde-9.11.0-cpr.tgz

# Build
Build is done within the infrap4d docker container created above.

> docker exec -it infrap4d bash

### Install basic utilities
```
apt-get update
apt-get install sudo git cmake autoconf gcc g++ libtool python3 python3-dev python3-distutils
```

### Build and install infrap4d dependencies
```
cd ipdk.recipe
export IPDK_RECIPE=`pwd`
cd $IPDK_RECIPE/setup
cmake . -DCMAKE_INSTALL_PREFIX=/usr/local/
make
ldconfig
```

### Build Intel P4Studio SDE
The below steps are minimal. For a more detailed installation procedure, please refer to the Intel P4Studio SDE Installation Guide.
```
cd sde/p4studio
./p4studio profile apply profiles/ipdk.yaml --bsp-path **path-to-bsp**
cd ..
export SDE_INSTALL=sde/install
```
> Note: To test with tofino-model, please edit profiles/ipdk.yaml file and remove the **asic: true** configuration. Additionally, a bsp-path would also not be required for tofino-model.

### Build Networking Recipe
```
cd $IPDK_RECIPE
./make-all.sh --dep-install /usr/local/ --sde-install $SDE_INSTALL
```

# Run
```
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

# Set forwarding pipeline
Following are optional steps to perform set forwarding pipeline.

The following steps describe generating a p4info and pipeline configuration required to perform a set forwarding pipeline from a controller.

Tofino only supports the TNA architecture. The bf-p4c compiler can still generate PSA compatible P4info files.

The tna_exact_match P4 example is used to describe these steps. The same steps will apply for any TNA program.
```
cd sde/build
cmake .. -DP4FLAGS="--p4runtime-files p4info.txt --p4runtime-force-std-externs"
make tna_exact_match
make install
```
The additional **P4FLAGS** option is required to generate a p4info.txt file required for P4Runtime. The 2nd option (p4runtime-force-std-externs) will disable including any TNA specific externs within the generated p4info file.

The above commands will generate 4 files. The last 3 files are combined together by the TDI pipeline builder to generate a single bin file to be pushed from the controller.
 - p4info.txt
 - tofino.bin
 - context.json
 - bf-rt.json

A conf file is also generated. This file holds the location of the above files relative to SDE_INSTALL.

## Using tdi_pipeline_builder
To generate the bin file for the controller.
```
cd $SDE_INSTALL
LD_LIBRARY_PATH=$IPDK_RECIPE/install/lib/:$SDE_INSTALL/lib \
    $IPDK_RECIPE/install/bin/tdi_pipeline_builder \
    -p4c_conf_file=$SDE_INSTALL/share/p4/targets/tofino/tna_exact_match.conf \
    -bf_pipeline_config_binary_file=$IPDK_RECIPE/tna_exact_match.pb.bin
```

## Using p4rt-ctl
Please refer to this [link](https://github.com/ipdk-io/networking-recipe/blob/main/docs/executables.rst#p4rt-ctl-executable)
