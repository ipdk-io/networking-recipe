# Running Infrap4d on Intel IPU E2100

This document explains how to install and run `infrap4d`
on Intel IPU E2100 hardware. It is assumed that you have followed the
[Deploying P4 programs guide](deploying-p4-programs.md)
to install the P4 package on FXP during IMC boot-up.

## 1. Extract P4 libraries from tarball

Log in to ACC and untar the `p4.tar.gz` tarball available under `/opt`

```bash
[root@mev-acc-rl opt]# cd /opt
[root@mev-acc-rl opt]# tar -xf p4.tar.gz
[root@mev-acc-rl opt]# cd /opt/p4/
[root@mev-acc-rl p4]# ls -lr
total 8
drwxr-xr-x 2 root root 4096 Nov 17 09:18 p4sde
drwxr-xr-x 2 root root 4096 Nov 17 09:18 p4-cp-nws
```

p4sde and p4-cp-nws binaries and libraries can be found under `/opt/p4/p4sde`
and `/opt/p4/p4-cp-nws` respectively.

## 2. Set up the environment

Note: `no_proxy` is set to avoid issues with the gRPC server connection.
The `sudo` option is required when running copy_config_files.sh since
we are creating directories and copying config files at a system level.

```bash
export SDE_INSTALL=/opt/p4/p4sde
export P4CP_INSTALL=/opt/p4/p4-cp-nws
export DEPEND_INSTALL=$P4CP_INSTALL
export no_proxy=<system_proxy>,localhost,127.0.0.1,192.168.0.0/16,\
<Docker container network>/12
export NO_PROXY=<system_proxy>,localhost,127.0.0.1,192.168.0.0/16,\
<Docker container network>/12
unset http_proxy
unset https_proxy

# Set up the environment
bash $P4CP_INSTALL/sbin/setup_env.sh $P4CP_INSTALL $SDE_INSTALL $DEPEND_INSTALL
sudo $P4CP_INSTALL/sbin/copy_config_files.sh $P4CP_INSTALL $SDE_INSTALL
```

## 3. Install TLS certificates

See [Using TLS Certificates](/guides/security/using-tls-certificates.md)
for step by step guide for generating and installing TLS certificates

## 4. Generate forwarding pipeline binary

Download `hw-p4-programs` TAR file specific to the release build and extract it to get p4 artifacts.
This document explains with `l3-fwd_sem` as a reference P4 program and the P4 artifacts are copied to `/opt/p4/l3-fwd_sem`

### Copy P4 artifacts to ACC

Copy `bf-rt.json`, `context.json`, `p4info.txt` to the ACC. See
[Compiling P4 programs](compiling-p4-programs.md)
for instructions on generating these files manually without downloading from
release build.

### Create ipu.bin

```bash
export OUTPUT_DIR=/opt/p4/l3-fwd_sem
cd $OUTPUT_DIR
touch ipu.bin
```

### Prepare configuration file

Handcraft the configuration file `/usr/share/stratum/es2k/es2k_skip_p4.conf`
with the following parameters:

- `pcie_bdf`

   Get PCI BDF of LAN Control Physical Function (CPF) device with device
   ID 1453 on ACC

   ```bash
   lspci | grep 1453
   00:01.6 Ethernet controller: Intel Corporation Device 1453 (rev 11)
   ```

   The value of `pcie_bdf` should be `00:01.6`

- `iommu_grp_num`

   Get the iommu group number

   ```bash
   cd $SDE_INSTALL/bin/
   ./vfio_bind.sh 8086:1453
   Device: 0000:00:01.6 Group = 5
   ```

   The value of `iommu_grp_num` should be `5`

- `vport`

   The number of vports supported is from [0-6].
   For example: `vport=[0-1]`

- `eal_args`

   Support values for `--proc-type` are `primary` and `auto`
   Note: In case of multi-process setup which is supported in docker
   environment, `--proc-type` can be used to specify the process type.

- `cfgqs-idx`

   We give options to each process (primary or secondary) to request
   numbers of configure queues. Admin must set cfgqs-idx between `"0-15"`,
   recommended option when running only 1 process. Plan and split config
   queues between multi-processes. For example, to configure two cfgq; use
   `cfgqs-idx: "0-1"`. Supported index numbers are from 0 to 15.

- `program-name`

   Specify the name of P4 program. For l3-fwd_sem example, replace
   `P4-PROGRAM-NAME` with `l3-fwd_sem`

- `p4_pipeline_name`

   Specify the name of P4 pipeline. For l3-fwd_sem example, replace
   `P4-PIPELINE-NAME` with `main`

- `bfrt-config`,  `context`, `config` and `path`

   Specify the absolute paths for the files. For l3-fwd_sem sample program:

   Replace `ABSOLUTE-PATH-TO-BFRT-JSON-FILE` with
   `/opt/p4/l3-fwd_sem/bf-rt.json`

   Replace `ABSOLUTE-PATH-TO-CONTEXT-JSON-FILE` with
   `/opt/p4/l3-fwd_sem/context.json`

   Replace `ABSOLUTE-PATH-TO-TOFINO-BIN-FILE` with
   `/opt/p4/l3-fwd_sem/ipu.bin`

   Replace `ABSOLUTE-PATH-FOR-JSON-FILES` with
   `/opt/p4/l3-fwd_sem`

The final es2k_skip_p4.conf for l3-fwd_sem sample program will look like:

```text
{
    "chip_list": [
    {
        "id": "asic-0",
        "chip_family": "mev",
        "instance": 0,
        "pcie_bdf": "0000:00:01.6",
        "iommu_grp_num": 5
    }
    ],
    "instance": 0,
    "cfgqs-idx": "0-15",
    "p4_devices": [
    {
        "device-id": 0,
        "fixed_functions" : [],
        "eal-args": "--lcores=1-2 -a 00:01.6,vport=[0-1] -- -i --rxq=1 --txq=1 --hairpinq=1 --hairpin-mode=0x0",
        "p4_programs": [
        {
            "program-name": "l3-fwd_sem",
            "bfrt-config": "/opt/p4/l3-fwd_sem/bf-rt.json",
            "p4_pipelines": [
            {
                "p4_pipeline_name": "main",
                "context": "/opt/p4/l3-fwd_sem/context.json",
                "config": "/opt/p4/l3-fwd_sem/ipu.bin",
                "pipe_scope": [
                    0,
                    1,
                    2,
                    3
                ],
                "path": "/opt/p4/l3-fwd_sem"
            }
            ]
        }
        ],
        "agent0": "lib/libpltfm_mgr.so"
    }
    ]
}
```

### Generate binary file

On ACC, use `tdi_pipeline_builder` available under `/opt/p4/p4-cp-nws/bin` to
combine the artifacts generated by the `p4c-pna-xxp` compiler and generate
forwarding pipeline binary.

```bash
$P4CP_INSTALL/bin/tdi_pipeline_builder \
    --p4c_conf_file=/usr/share/stratum/es2k/es2k_skip_p4.conf \
    --bf_pipeline_config_binary_file=$OUTPUT_DIR/l3-fwd_sem.pb.bin
```

## 5. Start Infrap4d

```bash
infrap4d

# Confirm infrap4d process is running
ps -ef | grep infrap4d
root 2406 1 99 03:34 ? 01:20:03 infrap4d
```

Other options available for the infrap4d process are:

 `-detach` (Run infrap4d in detached mode) type: bool default: true.

 `-disable_krnlmon` (Run infrap4d without krnlmon support) type: bool
      default: false

 `-external_stratum_urls` (Comma-separated list of URLs for server to listen to
for external calls from SDN controller, etc.) type: string
default: "0.0.0.0:9339,0.0.0.0:9559"

 `-local_stratum_url` (URL for listening to local calls from stratum stub.)
type: string default: "localhost:9559"

See infrap4d --help for more options.

## 6. Set the pipeline

Once the application is started, set the forwarding pipeline config using
P4Runtime Client `p4rt-ctl` set-pipe command

```bash
$P4CP_INSTALL/bin/p4rt-ctl set-pipe br0 $OUTPUT_DIR/l3-fwd_sem.pb.bin \
    $OUTPUT_DIR/l3-fwd_sem.p4info.txt
```

## 7. Configure forwarding rule

Add a forwarding rule to receive traffic on VSI-1 (base offset 16 + VSI ID 1)
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

See the [p4rt-ctl client guide](/clients/p4rt-ctl.rst) for more information
on the `p4rt-ctl` utility.

## 8. Send test traffic

Send a packet from a physical port of link partner to the E2100 using scapy, and
listen on the netdev corresponding to VSI-1 using `tcpdump`.

```text
sendp(Ether\
(dst="6e:80:97:ae:1e:4e", src="00:00:00:09:03:14")/IP(src="192.168.1.10",\
 dst="192.168.1.20")/UDP(sport=1000, dport=2000)/Raw(load="0"*50),\
 iface='enp175s0f0') # where enp175s0f0 is link partner interface
```
