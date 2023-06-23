# Run P4 Control Plane on Intel IPU E2100

- [1. Overview](#1-overview)
- [2. Running P4 Control Plane](#2-running-p4-control-plane)
	- [2.1 Running on Arm Compute Complex (ACC)](#21-running-on-acc)
		- [2.1.1 Extract the tarball to get P4 libraries](#211-extract-tarball)
		- [2.1.2 Set environment variables and hugepages](#212-set-env-and-hugepages)
		- [2.1.3 Install prerequisite packages](#213-install-prerequisites)
		- [2.1.4 Install TLS certificates](#214-install-tls-certificates)
		- [2.1.5 Start infrap4d](#215-start-infrap4d)
		- [2.1.6 Build P4 programs](#216-build-p4-programs)
		- [2.1.7 Generate forwarding pipeline binary](#217-generate-fwd-pipeline-bin)
			- [2.1.7.1 Prepare configuration files](#2171-prepare-config-files)
			- [2.1.7.2 Generate binary](#2172-generate-binary)
		- [2.1.8 Set the pipeline](#218-set-the-pipeline)
		- [2.1.9 Configure forwarding rule to receive traffic](#219-configure-rules)
		- [2.1.10 Test traffic from link partner to E2100](#2110-test-traffic)

## 1. Overview

This document explains how to install, and run the P4 Control Plane
software on Intel IPU E2100 hardware.

Please see IPU SDK documentation for more details about platform and core
software bringup.

## 2. Running P4 Control Plane

### 2.1 Running on Arm Compute Complex (ACC)

#### 2.1.1 Extract the tarball to get P4 libraries

Libraries are available as a tarball on ACC under /opt.
Login to ACC and untar the `p4.tar.gz`

```bash
[root@mev-acc-rl opt]# cd /opt
[root@mev-acc-rl opt]# tar -xf p4.tar.gz
[root@mev-acc-rl opt]# cd /opt/p4/
[root@mev-acc-rl p4]# ls -lr
total 8
drwxr-xr-x 2 root root 4096 Nov 17 09:18 p4sde
drwxr-xr-x 2 root root 4096 Nov 17 09:18 p4-cp-nws
```

p4sde and p4-cp-nws can be found under `/opt/p4/p4sde` and `/opt/p4/p4-cp-nws` respectively.

#### 2.1.2 Set environment variables and hugepages

Note: no_proxy is set to avoid issues with gRPC server connection.
`sudo` option is required when running copy_config_files.sh since
we are creating directories and copying config file at system level.

```bash
export SDE_INSTALL=/opt/p4/p4sde
export P4CP_INSTALL=/opt/p4/p4-cp-nws
export DEPEND_INSTALL=$P4CP_INSTALL
export no_proxy=<company_proxy>,localhost,127.0.0.1,192.168.0.0/16,\
<Docker container network>/12
export NO_PROXY=<company_proxy>,localhost,127.0.0.1,192.168.0.0/16,\
<Docker container network>/12

# Setup environment
bash $P4CP_INSTALL/sbin/setup_env.sh $P4CP_INSTALL $SDE_INSTALL $DEPEND_INSTALL
sudo $P4CP_INSTALL/sbin/copy_config_files.sh $P4CP_INSTALL $SDE_INSTALL
```

#### 2.1.3 Install prerequisite packages

Note: This should be handled as part of ACC bringup
Install python packages:

```bash
python3 -m pip install --upgrade pip
python3 -m pip install grpcio
python3 -m pip install ovspy
python3 -m pip install netaddr
python3 -m pip install protobuf==3.20.1
python3 -m pip install /usr/share/stratum/es2k/p4runtime-1.3.0-py3-none-any.whl
pip3 install pyelftools
```

#### 2.1.4 Install TLS certificates

See [Install TLS Certificates guide](https://github.com/ipdk-io/networking-recipe/blob/main/docs/guides/install-tls-certificates.md) for step by step guide to generate
and install TLS certificates

#### 2.1.5 Start infrap4d

```bash
infrap4d

# Confirm infrap4d process is running
ps -ef | grep infrap4d
root 2406 1 99 03:34 ? 01:20:03 infrap4d
```

Few important options available for infrap4d process:

 -detach (Run infrap4d in detached mode) type: bool default: true.

 -disable_krnlmon (Run infrap4d without krnlmon support) type: bool
      default: false

 -external_stratum_urls (Comma-separated list of URLs for server to listen to
for external calls from SDN controller, etc.) type: string
default: "0.0.0.0:9339,0.0.0.0:9559"

 -local_stratum_url (URL for listening to local calls from stratum stub.)
type: string default: "localhost:9559"

See infrap4d --help for more options.

#### 2.1.6 Build P4 programs

See [Build P4 Programs guide](https://github.com/ipdk-io/networking-recipe/blob/main/docs/guides/build-p4-programs.md) for step by step guide for building P4
artifacts and P4 package

#### 2.1.7 Generate forwarding pipeline binary

##### 2.1.7.1 Prepare configuration files

- Create tofino.bin

   ```bash
   export OUTPUT_DIR=/usr/share/mev_reference_p4_files/simple_l3_l4_pna
   cd $OUTPUT_DIR
   touch tofino.bin
   ```

- Handcraft the configuration file `/usr/share/mev_reference_p4_files/mev_sample.conf`
  with following parameters:

   - `pcie_bdf`

      1. Get PCI BDF of LAN Control Physical Function (CPF) device with device
      ID 1453 on ACC

        ```bash
        lspci | grep 1453
        00:01.6 Ethernet controller: Intel Corporation Device 1453 (rev 11)
        ```

        Value of `pcie_bdf` should be `00:01.6`

   - `iommu_grp_num`

      1. Get the iommu group number

        ```bash
        cd $SDE_INSTALL/bin/
        ./vfio_bind.sh 8086:1453
        Device: 0000:00:01.6 Group = 5
        ```

        Value of `iommu_grp_num` should be `5`

   - `vport`

      1. Number of vports supported are from [0-6].
      For example: `vport=[0-1]`

   - `eal_args` : Parameters used by DPDK

      1. Support values for `--proc-type` are `primary` and `auto`
      Note: In case of multi-process setup which is supported in docker
      environment, --proc-type can be used to specify the process type.
      Refer `IPDK_Multi_process_guide.md` for more details

   - `cfgqs-idx`

      1. We give options to each process (primary or secondary) to request
      numbers of configure Queues. Admin must set cfgqs-idx between `0-15`,
      recommended option when running only 1 process. Plan and split config
      queues between multi-process. For example, to configure one cfgq; use
      `cfgqs-idx: "1"`.Supported index numbers are from 0 to 15.

   - Specify the absolute paths for  bfrt-config, context, config and tofino.bin.

##### 2.1.7.2 Generate binary

Use TDI pipeline builder to combine the artifacts generated by the p4c-pna-xxp
compiler to generate a single bin file to be pushed from the P4Runtime client
`p4rt-ctl`.

```bash
$P4CP_INSTALL/bin/tdi_pipeline_builder \
    --p4c_conf_file=/usr/share/mev_reference_p4_files/mev_sample.conf \
    --bf_pipeline_config_binary_file=$OUTPUT_DIR/simple_l3_l4_pna.pb.bin
```

#### 2.1.8 Set the pipeline

Set the forwarding pipeline config using P4Runtime Client `p4rt-ctl` set-pipe
command

```bash
$P4CP_INSTALL/bin/p4rt-ctl set-pipe br0 $OUTPUT_DIR \
/simple_l3_l4_pna.pb.bin $OUTPUT_DIR/simple_l3_l4_pna.p4info.txt
```

#### 2.1.9 Configure forwarding rule to receive traffic

Add a forwarding rule to receive traffic on VSI-1 (base offset 16 + VSI ID 1) \
when the key matches.

```bash
$P4CP_INSTALL/bin/p4rt-ctl add-entry br0 \
MainControlImpl.l3_l4_match_rx "hdrs.ipv4[vmeta.common.depth].protocol=0x11,\
vmeta.common.port_id=0,istd.direction=0,\
hdrs.ipv4[vmeta.common.depth].src_ip="192.168.1.10",\
hdrs.ipv4[vmeta.common.depth].dst_ip="192.168.1.20",\
hdrs.udp[vmeta.common.depth].sport=1000,\
hdrs.udp[vmeta.common.depth].dport=2000,\
action=MainControlImpl.send(17)"
```

Note: See [p4rt-ctl Readme](https://github.com/ipdk-io/networking-recipe/blob/main/docs/clients/p4rt-ctl.rst) for more information on the `p4rt-ctl` utility.

#### 2.1.10 Test traffic from link partner to E2100

Send a packet from a physical port of link partner to the E2100 using scapy, and
listen on the netdev corresponding to VSI-1 using `tcpdump`.

```text
sendp(Ether\
(dst="6e:80:97:ae:1e:4e", src="00:00:00:09:03:14")/IP(src="192.168.1.10",\
 dst="192.168.1.20")/UDP(sport=1000, dport=2000)/Raw(load="0"*50),\
 iface='enp175s0f0') # where enp175s0f0 is link partner interface
```
