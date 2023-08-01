# ES2K Setup Guide

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
cd $IPDK_RECIPE
mkdir install
export P4CP_INSTALL=`pwd`/install
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

#### Set up the environment required by infrap4d

*Note*: `sudo` option is required when running copy_config_files.sh since
we are creating directories and copying config file at system level.

```bash
source $P4CP_INSTALL/sbin/setup_env.sh $P4CP_INSTALL $SDE_INSTALL $DEPEND_INSTALL
sudo $P4CP_INSTALL/sbin/copy_config_files.sh $P4CP_INSTALL $SDE_INSTALL
```

#### Set hugepages required for ES2K

Run the hugepages script.

```bash
sudo $P4CP_INSTALL/sbin/set_hugepages.sh
```

#### Export all environment variables to sudo user

```bash
alias sudo='sudo PATH="$PATH" HOME="$HOME" LD_LIBRARY_PATH="$LD_LIBRARY_PATH" SDE_INSTALL="$SDE_INSTALL"'
```

#### Modify the conf file

Update `/usr/share/stratum/es2k/es2k_skip_p4.conf` with CPF BDF as per your setup.
User can get CPF BDF by running `lspci | grep 1453`
Note: max vport you can pass here from [0-6] "eal-args": "--lcores=1-2 -a <cpf_bdf>,vport=[0-1] -- -i --rxq=1 --txq=1 --hairpinq=1 --hairpin-mode=0x0"

Eg. "eal-args": "--lcores=1-2 -a 00:01.6,vport=[0-1] -- -i --rxq=1 --txq=1 --hairpinq=1 --hairpin-mode=0x0"

We give options to each process to request numbers of configure queues before starting the process

Admin must set cfgqs-idx between 0-15. See the latest es2k_skip_p4.conf for the exact syntax.
If you want 16 cfgqs, use "cfgqs-idx": "0-15". Specify the range as a string. Index numbers from 0 to 15 are supported.
In multi process environment, user should plan and split the queues between primary and secondary processes and specify
the range with cfgqs-idx parameter. Total number of queues split between processes should not exceed 16.

#### Run the infrap4d daemon

```bash
cd $IPDK_RECIPE
sudo $P4CP_INSTALL/sbin/infrap4d
```

*Note*: By default, infrap4d runs in detached mode. If you want to run in
attached mode, specify the --nodetach command-line option.

- All infrap4d logs are by default logged under `/var/log/stratum`.
- All P4SDE logs are logged in `p4_driver.log` under `$IPDK_RECIPE`.
*-All OVS logs are logged under `/tmp/ovs-vswitchd.log`.

### Run a sample program

Open a new terminal to set the pipeline and try the sample P4 program.
Set up the environment and export all environment variables to sudo user.

```bash
source $P4CP_INSTALL/sbin/setup_env.sh $P4CP_INSTALL $SDE_INSTALL $DEPEND_INSTALL
$P4CP_INSTALL/sbin/copy_config_files.sh $P4CP_INSTALL $SDE_INSTALL
alias sudo='sudo PATH="$PATH" HOME="$HOME" LD_LIBRARY_PATH="$LD_LIBRARY_PATH" SDE_INSTALL="$SDE_INSTALL"'
```

#### Create IPDF netdevs

After installing ATE Kernel on HOST machine, install the following drivers
to bind the network devices (netdevs) to the E2100 target.

```bash
modprobe auxiliary 
modprobe idpf
```

#### Create P4 artifacts

- P4 programs are available in the P4 SDE repository.

- Obtain a copy of the P4 Compiler for ES2K and install it on your
  development system.

- Set the environment variable OUTPUT_DIR to the location where artifacts
  should be generated and where p4 files are available

```bash
export OUTPUT_DIR=/root/p4-driver/mev_reference_p4_files/simple_l3_l4_pna
```

- Generate the artifacts using the p4c compiler installed in the previous step:

```bash
p4c-pna-xxp -I/usr/lib -I/usr/share/p4c/p4include -I/usr/share/p4c/idpf-lib \
            $OUTPUT_DIR/simple_l3_l4_pna.p4 -o $OUTPUT_DIR/simple_l3_l4_pna.s \
            --p4runtime-files $OUTPUT_DIR/simple_l3_l4_pna.p4info.txt \
            --context $OUTPUT_DIR/simple_l3_l4_pna.context.json \
            --bfrt $OUTPUT_DIR/simple_l3_l4_pna.bfrt.json
```

*Note*: The above commands will generate three files
(`simple_l3_l4_pna.p4info.txt`, `simple_l3_l4_pna.bfrt.json`, and
`simple_l3_l4_pna.context.json`).

- Modify `simple_l3_l4_pna.conf` to specify the correct paths for
  bfrt-config, context, and config.

- Create a dummy tofino.bin file, which is needed by tdi_pipeline_builder.

```bash
touch tofino.bin
```

- TDI pipeline builder combines the artifacts generated by the p4c compiler
  to generate a single bin file to be pushed from the controller.
  Generate binary executable using the TDI pipeline builder command below:

```bash
$P4CP_INSTALL/bin/tdi_pipeline_builder \
    --p4c_conf_file=$OUTPUT_DIR/simple_l3_l4_pna.conf \
    --bf_pipeline_config_binary_file=$OUTPUT_DIR/simple_l3_l4_pna.pb.bin
```

#### Set forwarding pipeline

```bash
sudo $P4CP_INSTALL/bin/p4rt-ctl set-pipe br0 $OUTPUT_DIR/simple_l3_l4_pna.pb.bin $OUTPUT_DIR/simple_l3_l4_pna.p4info.txt
```

#### Configure forwarding rule to receive traffic

Add a forwarding rule to receive traffic on VSI-1 (base offset 16 + VSI ID 1) \
when the key matches.

```bash
sudo  $P4CP_INSTALL/bin/p4rt-ctl add-entry br0 MainControlImpl.l3_l4_match_rx \
    "hdrs.ipv4[vmeta.common.depth].protocol=0x11,vmeta.common.port_id=0,istd.direction=0,
    hdrs.ipv4[vmeta.common.depth].src_ip="192.168.1.10",hdrs.ipv4[vmeta.common.depth].dst_ip="192.168.1.20",
    hdrs.udp[vmeta.common.depth].sport=1000,hdrs.udp[vmeta.common.depth].dport=2000,action=MainControlImpl.send(17)"
```

See the [p4rt-ctl client guide](/clients/p4rt-ctl.rst) for more information
on the `p4rt-ctl` utility.

#### Test traffic from link partner to E2100

Send a packet from a physical port of link partner to the E2100 using any
traffic generator application, and listen on the netdev corresponding to VSI-1
using `tcpdump`.

```text
sendp(Ether(dst="6e:80:97:ae:1e:4e", src="00:00:00:09:03:14")/IP(src="192.168.1.10", dst="192.168.1.20")/UDP(sport=1000, dport=2000)/Raw(load="0"*50), iface='ens78f0')
```
