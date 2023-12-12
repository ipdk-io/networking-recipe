# Tofino Setup Guide

This document explains how to build, install, and run the P4 Control Plane
software on Tofino hardware.

Similar steps apply when running on the Tofino simulation model. Please see
Tofino-specific documentation for more details.

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

Clone IPDK networking-recipe:

```bash
git clone --recursive git@github.com:ipdk-io/networking-recipe.git p4cp.recipe
cd p4cp.recipe
export P4CP_RECIPE=`pwd`
```

Clone the repository used to build the Stratum dependencies:

```bash
git clone --recursive https://github.com/ipdk-io/stratum-deps.git
```

### Intel P4Studio

Get Intel P4Studio SDE. The untarred directory is henceforth referred to as
**sde**. If using an Intel Tofino Reference platform, please also download
the BSP package.

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

See the [OpenSSL security guide](/guides/security/openssl-guide.md)
for OpenSSL version and EOL information.

### Build and install stratum dependencies

```bash
cd stratum-deps
cmake . -DCMAKE_INSTALL_PREFIX=/usr/local/
make
ldconfig
```

### Build Intel P4Studio SDE

The below steps are minimal. For a more detailed installation procedure,
please refer to the Intel P4Studio SDE Installation Guide.

```bash
cd sde/p4studio
./p4studio profile apply profiles/ipdk.yaml --bsp-path **path-to-bsp**
cd ..
export SDE_INSTALL=sde/install
```

> Note: To test with tofino-model, edit the `profiles/ipdk.yaml` file and
change the `asic` configuration from `true` to `false`.

Not that `bsp-path` is not required for tofino-model.

### Build Networking Recipe

```bash
$P4CP_RECIPE/make-all.sh --target=tofino --deps=/usr/local
```

## Run

```bash
cd P4CP_RECIPE
cp $SDE_INSTALL/share/bf_switchd/zlog-cfg $P4CP_RECIPE/zlog-cfg-cur
mkdir -p /etc/stratum/
mkdir -p /var/log/stratum/

LD_LIBRARY_PATH=$P4CP_RECIPE/install/lib/:$SDE_INSTALL/lib \
    ./install/sbin/infrap4d \
    -chassis_config_file=./stratum/stratum/stratum/hal/config/x86-64-accton-wedge100bf-32x-r0/chassis_config.pb.txt \
    -tdi_switchd_cfg=$SDE_INSTALL/share/p4/targets/tofino/tofino_skip_p4.conf \
    -tdi_sde_install=$SDE_INSTALL
```

> Note: To test with tofino-model, please use **./stratum/stratum/stratum/hal/config/barefoot-tofino-model/chassis_config.pb.txt** as the chassis_config_file.

## Set forwarding pipeline

Following are optional steps to perform set forwarding pipeline.

The following steps describe generating a p4info and pipeline configuration
required to perform a set forwarding pipeline from a controller.

Tofino only supports the TNA architecture. The bf-p4c compiler can still
generate PSA compatible P4info files.

The tna_exact_match P4 example is used to describe these steps. The same
steps will apply for any TNA program.

```bash
cd sde/build
make tna_exact_match
make install
```

The above commands will generate 4 files. The last three files are combined
together by the TDI pipeline builder to generate a single bin file to be
pushed by the controller.

These files can be found in install/share/tofinopd/tna_exact_match/ directory.

- tna_exact_match.p4info.pb.txt
- tofino.bin
- context.json
- bf-rt.json

A conf file is also generated. This file holds the location of the above
files relative to SDE_INSTALL.

### Using tdi_pipeline_builder

To generate the bin file for the controller.

```bash
cd $SDE_INSTALL
LD_LIBRARY_PATH=$P4CP_RECIPE/install/lib/:$SDE_INSTALL/lib \
    $P4CP_RECIPE/install/bin/tdi_pipeline_builder \
    -p4c_conf_file=$SDE_INSTALL/share/p4/targets/tofino/tna_exact_match.conf \
    -bf_pipeline_config_binary_file=$P4CP_RECIPE/tna_exact_match.pb.bin
```
