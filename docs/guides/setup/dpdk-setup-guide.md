# DPDK Setup Guide

This document explains how to build, install, and run P4 Control Plane
for the DPDK target.

## Prerequisites

For Fedora:

```bash
yum install libatomic libnl3-devel
pip3 install -r requirements.txt
```

For Ubuntu:

```bash
apt install libatomic1 libnl-route-3-dev
pip3 install -r requirements.txt
```

See the [OpenSSL security guide](/guides/security/openssl-guide.md)
for information on installing OpenSSL.

## Build and install DPDK SDE

Clone the repository used to build the DPDK SDE:

```bash
git clone --recursive https://github.com/p4lang/p4-dpdk-target.git p4sde
```

Follow the build and installation instructions in the
[P4 SDE Readme](https://github.com/p4lang/p4-dpdk-target/blob/main/README.md#building-and-installing).

Remember the directory in which you install the DPDK SDE.
You will need it to define the `SDE_INSTALL` environment variable.

## Build and install stratum dependencies

Clone the repository used to build the Stratum dependencies:

```bash
git clone --recursive https://github.com/ipdk-io/stratum-deps.git
```

Now follow the instructions in the
[Building Host Dependencies](https://github.com/ipdk-io/stratum-deps/blob/main/docs/building-host-deps.md)
document.

Remember the directory in which you install the Stratum dependencies.
You will need it to define the `DEPEND_INSTALL` environment variable.

## Define environment variables

Define the following environment variables. They supply default values to
the build system and helper scripts.

| Variable | Definition |
| -------- | ---------- |
| `DEPEND_INSTALL` | Path to the directory in which the Stratum dependencies are installed. |
| `SDE_INSTALL` | Path to the directory in which the DPDK SDE is installed. |

## Build networking recipe

### Clone repository

Clone the repository used to build P4 Control Plane:

```bash
git clone --recursive https://github.com/ipdk-io/networking-recipe.git p4cp.recipe
cd p4cp.recipe
export P4CP_RECIPE=`pwd`
```

### Compile the recipe

```bash
$P4CP_RECIPE/make-all.sh --target=dpdk --rpath
```

By default, make-all.sh will create an `install` folder in the networking-recipe
directory in which to install the build artifacts. You can specify a different
directory by means of the `--prefix` parameter.

See the [make-all.sh](/scripts/make-all.rst) user guide for information
about the options of the `make-all.sh` helper script.

### Define P4CP_INSTALL

We recommend that you define the following environment variable:

| Variable | Definition |
| -------- | ---------- |
| `P4CP_INSTALL` | Path to the directory in which the P4 Control Plane build artifacts are installed. |

It is used throughout the remainder of this document.

## Run Infrap4d

### Set up the environment required by infrap4d

*Note*: `sudo` is required when running `copy_config_files.sh` since you are
copying files to system directories.

```bash
source $P4CP_INSTALL/sbin/setup_env.sh $P4CP_INSTALL $SDE_INSTALL $DEPEND_INSTALL
sudo $P4CP_INSTALL/sbin/copy_config_files.sh $P4CP_INSTALL $SDE_INSTALL
```

### Set hugepages required for DPDK

Run the hugepages script.

```bash
sudo $P4CP_INSTALL/sbin/set_hugepages.sh
```

### Export all environment variables to sudo user

```bash
alias sudo='sudo PATH="$PATH" HOME="$HOME" LD_LIBRARY_PATH="$LD_LIBRARY_PATH" SDE_INSTALL="$SDE_INSTALL"'
```

### Run the infrap4d daemon

By default, infrap4d runs in secure mode and expects certificates to be available in
a specific directory. For information on running infrap4d in insecure mode, or steps to generate TLS
certificates, see the [security guide](/guides/security/security-guide.md).

```bash
sudo $P4CP_INSTALL/sbin/infrap4d
```

 By default, infrap4d runs in detached mode. If you want to run
 infrap4d in attached mode, use the `--nodetach` option.

- All infrap4d logs are by default logged under /var/log/stratum.
- All P4SDE logs are logged in p4_driver.log under $P4CP_RECIPE.
- All OVS logs are logged under /tmp/ovs-vswitchd.log.

### Run a sample program

Open a new terminal to set the pipeline and try the sample P4 program.
Set up the environment and export all environment variables to sudo user.

```bash
source $P4CP_INSTALL/sbin/setup_env.sh $P4CP_INSTALL $SDE_INSTALL $DEPEND_INSTALL
$P4CP_INSTALL/sbin/copy_config_files.sh $P4CP_INSTALL $SDE_INSTALL
alias sudo='sudo PATH="$PATH" HOME="$HOME" LD_LIBRARY_PATH="$LD_LIBRARY_PATH" SDE_INSTALL="$SDE_INSTALL"'
```

### Create two TAP ports

```bash
sudo ./install/bin/gnmi-ctl set "device:virtual-device,name:TAP0,pipeline-name:pipe,mempool-name:MEMPOOL0,mtu:1500,port-type:TAP"
sudo ./install/bin/gnmi-ctl set "device:virtual-device,name:TAP1,pipeline-name:pipe,mempool-name:MEMPOOL0,mtu:1500,port-type:TAP"
ifconfig TAP0 up
ifconfig TAP1 up
```

 *Note*: See [gnmi-ctl client guide](/clients/gnmi-ctl.rst)
 for more information on the gnmi-ctl utility.

### Create P4 artifacts

- Clone the ipdk repo for scripts to build p4c and sample p4 program

```bash
git clone https://github.com/ipdk-io/ipdk.git --recursive ipdk-io
```

- Install p4c compiler from [p4c](https://github.com/p4lang/p4c) repository
  and follow the readme for procedure. Alternatively, refer to
  [p4c script](https://github.com/ipdk-io/ipdk/blob/main/build/networking/scripts/build_p4c.sh)

- Set the environment variable OUTPUT_DIR to the directory in which artifacts
  should be generated and where p4 files are available

```bash
export OUTPUT_DIR=/root/ipdk-io/build/networking/examples/simple_l3
```

- Generate the artifacts using the p4c compiler installed in the previous step:

```bash
mkdir $OUTPUT_DIR/pipe
p4c-dpdk --arch pna --target dpdk \
    --p4runtime-files $OUTPUT_DIR/p4Info.txt \
    --bf-rt-schema $OUTPUT_DIR/bf-rt.json \
    --context $OUTPUT_DIR/pipe/context.json \
    -o $OUTPUT_DIR/pipe/simple_l3.spec $OUTPUT_DIR/simple_l3.p4
```

The above commands will generate three files (p4Info.txt, bf-rt.json,
and context.json).

- Modify simple_l3.conf file to provide correct paths for bfrt-config, context,
  and config.

- TDI pipeline builder combines the artifacts generated by p4c compiler to
  generate a single bin file to be pushed from the controller.
  Generate binary executable using tdi-pipeline builder command below:

```bash
$P4CP_INSTALL/bin/tdi_pipeline_builder \
    --p4c_conf_file=$OUTPUT_DIR/simple_l3.conf \
    --bf_pipeline_config_binary_file=$OUTPUT_DIR/simple_l3.pb.bin
```

#### Set forwarding pipeline

```bash
sudo $P4CP_INSTALL/bin/p4rt-ctl set-pipe br0 $OUTPUT_DIR/simple_l3.pb.bin $OUTPUT_DIR/p4Info.txt
```

#### Configure forwarding rules

```bash
sudo  $P4CP_INSTALL/bin/p4rt-ctl add-entry br0 ingress.ipv4_host "hdr.ipv4.dst_addr=1.1.1.1,action=ingress.send(0)"
sudo  $P4CP_INSTALL/bin/p4rt-ctl add-entry br0 ingress.ipv4_host "hdr.ipv4.dst_addr=2.2.2.2,action=ingress.send(1)"
```

See [p4rt-ctl client guide](/clients/p4rt-ctl.rst) for more information on
the p4rt-ctl utility.

#### Test traffic between TAP0 and TAP1

Send packet from TAP 0 to TAP1 using scapy and listen on TAP1 using `tcpdump`.

```text
sendp(Ether(dst="00:00:00:00:03:14", src="a6:c0:aa:27:c8:2b")/IP(src="192.168.1.10", dst="2.2.2.2")/UDP()/Raw(load="0"*50), iface='TAP0')
```
