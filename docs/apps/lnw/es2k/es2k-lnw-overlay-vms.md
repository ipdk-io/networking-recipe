<!--

 Copyright 2024 Intel Corporation.

 SPDX-License-Identifier: Apache-2.0

 Licensed under the Apache License, Version 2.0 (the "License");
 you may not use this file except in compliance with the License.
 You may obtain a copy of the License at:

 http://www.apache.org/licenses/LICENSE-2.0

 Unless required by applicable law or agreed to in writing, software
 distributed under the License is distributed on an "AS IS" BASIS,
 WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 See the License for the specific language governing permissions and
 limitations under the License.

- -->

# Linux Networking with Overlay VMs

This document explains how to run the Linux networking scenario on ES2K with 8 overlay VMs.

## Topology

![Linux Networking Topology](es2k-lnw-topology.png)

See [Linux Networking for ES2K](es2k-linux-networking.md)
for more details on this feature.

Prerequisites:

- Download `hw-p4-programs` TAR file specific to the build and extract it to get `fxp-net_linux-networking-v2` p4 artifacts. Go through `Limitations` specified in `README` and bringup the setup accordingly.
- Follow steps mentioned in [Deploying P4 Programs for E2100](/guides/es2k/deploying-p4-programs) for bringing up IPU with a custom P4 package.
Modify `load_custom_pkg.sh` with following parameters for linux_networking package:

```text
    sed -i 's/sem_num_pages = 1;/sem_num_pages = 25;/g' $CP_INIT_CFG
    sed -i 's/lem_num_pages = 1;/lem_num_pages = 10;/g' $CP_INIT_CFG
    sed -i 's/acc_apf = 4;/acc_apf = 16;/g' $CP_INIT_CFG

```

- Download `IPU_Documentation` TAR file specific to the build and refer to `Getting Started Guide` on how to install compatible `IDPF driver` on host. Once an IDPF driver is installed, bring up SRIOV VF by modifying the `sriov_numvfs` file present under one of the IDPF network devices. Example as below

  ```bash
  echo 16 > /sys/class/net/ens802f0/device/sriov_numvfs
  ```

Notes about topology:

- VMs are spawned on top of each VF. Each VF will have a port representor in ACC. P4 runtime rules are configured to map VFs and their corresponding port representors.
- Each physical port will have a port representor in ACC. P4 runtime rules are configured to map VFs and its corresponding port representors.
- Each physical port will have a corresponding APF netdev on HOST. Create port representors in ACC for each HOST APF netdev. These APF netdev on HOST will receive unknown traffic for applications to act on.
- All port representors should be part of an OvS bridge. Based on topology, these OvS bridges will just perform bridging or TEP termination bridges which are used to enable underlay connectivity for VxLAN traffic.
- OvS bridges, VxLAN ports are created using ovs-vsctl command provided by the networking recipe and all the port representors are attached to OvS bridge using ovs-vsctl command.
- This config has:
  - 8 Overlay VFs
  - 8 Port representors in ACC for the above 8 Overlay VFs
  - 2 physical ports
  - 2 Port representors in ACC for the above 2 physical ports
  - 2 APF netdev on HOST
  - 2 Port representors in ACC for the above 2 HOST APF netdevs

System under test will have above topology running the networking recipe. Link Partner can have the networking recipe or legacy OvS or kernel VxLAN. Refer to the limitation section in [Linux Networking for E2100](es2k-linux-networking.md) before setting up the topology.

## Creating the topology

Follow steps mentioned in [Running Infrap4d on Intel E2100](/guides/es2k/running-infrap4d.md) for starting `infrap4d` process and creating protobuf binary for `fxp-net_linux-networking-v2` p4 program.

### Port Mapping

These VSI values can be checked with `/usr/bin/cli_client -q -c` command on IMC. This command provides VSI ID, Vport ID, and corresponding MAC addresses for all

- IDPF netdevs on ACC
- VFs on HOST
- IDPF netdevs on HOST (if IDPF driver loaded by you on HOST)
- Netdevs on IMC

| Overlay VFs | Overlay VFs VSI ID | ACC port representor | ACC port representor VSI ID |
| ------------| ------------------ | -------------------- | --------------------------- |
| ens802f0v0  | (0x1b) 27 | enp0s1f0d1 | (0x09) 9  |
| ens802f0v1  | (0x1c) 28 | enp0s1f0d2 | (0x0a) 10 |
| ens802f0v4  | (0x1d) 29 | enp0s1f0d3 | (0x0b) 11 |
| ens802f0v3  | (0x1e) 30 | enp0s1f0d4 | (0x0c) 12 |
| ens802f0v2  | (0x1f) 31 | enp0s1f0d5 | (0x0d) 13 |
| ens802f0v11 | (0x20) 32 | enp0s1f0d6 | (0x0e) 14 |
| ens802f0v10 | (0x21) 33 | enp0s1f0d7 | (0x0f) 15 |
| ens802f0v9  | (0x22) 34 | enp0s1f0d8 | (0x10) 16 |

| Physical port | Physical port ID  | ACC Port presenter   | ACC Port presenter VSI ID |
| ------------- | ----------------- | -------------------- | ------------------------- |
| Phy port 0    | (0x0) 0 | enp0s1f0d9  | (0x11) 17 |
| Phy port 1    | (0x1) 1 | enp0s1f0d11 | (0x13) 19 |

| APF netdev    | APF netdev VSI ID | ACC Port presenter   | ACC Port presenter VSI ID |
| ------------- | ----------------- | -------------------- | ------------------------- |
| ens802f0d1    | (0x18) 24 | enp0s1f0d10 | (0x12) 18 |
| ens802f0d2    | (0x19) 25 | enp0s1f0d12 | (0x14) 20 |

(NOTE: Above port names and its VSI ID's may different from setup to setup, configure accordingly)

### Set the forwarding pipeline

Once the application is started, set the forwarding pipeline config using
P4Runtime Client `p4rt-ctl` set-pipe command

```bash
$P4CP_INSTALL/bin/p4rt-ctl set-pipe br0 $OUTPUT_DIR/fxp-net_linux-networking-v2.pb.bin \
    $OUTPUT_DIR/p4info.txt
```

Note: Assuming `fxp-net_linux-networking-v2.pb.bin` and `p4info.txt`
along with other P4 artifacts are created as per the steps mentioned in the previous section.

### Configure VSI Group and add a netdev

Add all ACC port representors to VSI group 1. VSI group 1 is dedicated for this configuration,
execute below devmem commands on IMC.

```bash
# SEM_DIRECT_MAP_PGEN_CTRL: LSB 11-bit is for vsi which need to map into vsig
devmem 0x20292002a0 64 0x8000050000000009

# SEM_DIRECT_MAP_PGEN_DATA_VSI_GROUP : This will set vsi
# (set in SEM_DIRECT_MAP_PGEN_CTRL register LSB) into VSIG-1.
devmem 0x2029200388 64 0x1

# SEM_DIRECT_MAP_PGEN_CTRL: LSB 11-bit is for vsi which need to map into vsig
devmem 0x20292002a0 64 0xA000050000000009
```

Note: Here VSI 9 has been used as one of the ACC port representors and added to VSI group 1. For this use case add all 16 IDPF interfaces created on ACC. Modify this VSI based on your configuration.

### Start OvS as a separate process

Enhanced OvS is used as a control plane for source MAC learning of overlay VM's. This OvS binary is available as part of ACC build and should be started as a separate process.

```bash
export RUN_OVS=/opt/p4/p4-cp-nws

rm -rf $RUN_OVS/etc/openvswitch
rm -rf $RUN_OVS/var/run/openvswitch 
mkdir -p $RUN_OVS/etc/openvswitch/
mkdir -p $RUN_OVS/var/run/openvswitch


ovsdb-tool create $RUN_OVS/etc/openvswitch/conf.db \
        $RUN_OVS/share/openvswitch/vswitch.ovsschema

ovsdb-server \
        --remote=punix:$RUN_OVS/var/run/openvswitch/db.sock \
        --remote=db:Open_vSwitch,Open_vSwitch,manager_options \
        --pidfile --detach

ovs-vsctl --no-wait init

mkdir -p /tmp/logs
ovs-vswitchd --pidfile --detach --mlockall \
        --log-file=/tmp/logs/ovs-vswitchd.log

ovs-vsctl set Open_vSwitch . other_config:n-revalidator-threads=1
ovs-vsctl set Open_vSwitch . other_config:n-handler-threads=1

ovs-vsctl  show
```

### Create Overlay network

Option 1: Create VFs on HOST and spawn VMs on top of those VFs.
Example: Below config is provided for one VM, and considering each VM is in one VLAN. Extend this to 8 VMs.

```bash
# VM1 configuration
telnet <VM1 IP> <VM1 port>
ip link add link <Netdev connected to VF1> name <Netdev connected to VF1>.10 type vlan id 10
ip addr add 101.0.0.1/24 dev <Netdev connected to VF1>.10
ifconfig <Netdev connected to VF> up
ifconfig <Netdev connected to VF>.10 up
```

Option 2: If we are unable to spawn VM's on top of the VF's, we can leverage kernel network namespaces.
Move each VF to a network namespace and assign IP addresses.
Example: Below config is provided for one VM, and considering each namespace is in one VLAN. Extend this to 8 namespaces.

```bash
ip netns add VM0
ip link set <VF1 port> netns VM0
ip netns exec VM0 ip link add link <VF1 port> name <VF1 port>.10 type vlan id 10
ip netns exec VM0 ip addr add 101.0.0.1/24 dev <VF1 port>.10
ip netns exec VM0  ifconfig <VF1 port> up
ip netns exec VM0  ifconfig <VF1 port>.10 up
```

### Configure rules for mapping between Overlay VF and ACC port representor

Configure rules to send overlay packets from a VM to its respective port representors.

Refer above port mapping for overlay VF to ACC port representor mapping. Here sample commands are shown for a single overlay network, configure similar mapping for remaining VFs.

Example:

- Overlay VF1 has a VSI value 27
- Corresponding port representor VSI value 9
- If a VSI is used as an action, add an offset of 16 to the VSI value

```bash
# Create a source port for an overlay VF (VSI-27). Source port action can be any value.
# For simplicity add 16 to VSI ID.
 p4rt-ctl add-entry br0 linux_networking_control.tx_source_port_v4 \
     "vmeta.common.vsi=27,zero_padding=0,action=linux_networking_control.set_source_port(43)"

# Create a mapping between overlay VF (VSI-27/source port-43) and ACC port representor (VSI-9)
 p4rt-ctl add-entry br0 linux_networking_control.source_port_to_pr_map \
     "user_meta.cmeta.source_port=43,zero_padding=0,action=linux_networking_control.fwd_to_vsi(25)"

 p4rt-ctl add-entry br0 linux_networking_control.tx_acc_vsi \
     "vmeta.common.vsi=9,zero_padding=0,action=linux_networking_control.l2_fwd_and_bypass_bridge(43)"

# Create a mapping for traffic to flow between VSIs (VSI-27/source port-43) and (VSI-9)
 p4rt-ctl add-entry br0 linux_networking_control.vsi_to_vsi_loopback \
     "vmeta.common.vsi=9,target_vsi=27,action=linux_networking_control.fwd_to_vsi(43)"

 p4rt-ctl add-entry br0 linux_networking_control.vsi_to_vsi_loopback \
     "vmeta.common.vsi=27,target_vsi=9,action=linux_networking_control.fwd_to_vsi(25)"

```

### Configure rules for mapping between Physical port and ACC port representor

Configure rules to send ingress packets from a physical port to its respective port representors.

Refer above port mapping for physical port to ACC port representor mapping. Here sample commands are shown for a single physical port, configure similar mapping for remaining physical ports.

Example:

- Physical port 0 port id is 0
- Corresponding port representor VSI value 17
- If a VSI is used as an action, add an offset of 16 to the VSI value

```bash
# Create a source port for a physical port (Phy port-0). Source port action can be any value.
# For simplicity consider the same value as phy port id.
 p4rt-ctl add-entry br0 linux_networking_control.rx_source_port \
     "vmeta.common.port_id=0,zero_padding=0,action=linux_networking_control.set_source_port(0)"

# Create a mapping between physical port (Phy port 0/src port 0) and ACC port representor (VSI-17)
  p4rt-ctl add-entry br0 linux_networking_control.rx_phy_port_to_pr_map \
      "vmeta.common.port_id=0,zero_padding=0,action=linux_networking_control.fwd_to_vsi(33)"

 p4rt-ctl add-entry br0 linux_networking_control.source_port_to_pr_map \
     "user_meta.cmeta.source_port=0,zero_padding=0,action=linux_networking_control.fwd_to_vsi(33)"

 p4rt-ctl add-entry br0 linux_networking_control.tx_acc_vsi \
     "vmeta.common.vsi=17,zero_padding=0,action=linux_networking_control.l2_fwd_and_bypass_bridge(0)"
```

### Configure rules for mapping between APF netdev on HOST and ACC port representor

Configure rules to send APF netdev on HOST to its respective port representors.

Refer above port mapping for APF netdev on HOST to ACC port representor mapping. Here sample commands are shown for APF netdev on HOST, configure similar mapping for remaining APF netdevs on HOST.

Example:

- APF netdev 1 on HOST has a VSI value 24
- Corresponding port representor VSI value 18
- If a VSI is used as an action, add an offset of 16 to the VSI value

```bash
# Create a source port for an overlay VF (VSI-24). Source port action can be any value.
# For simplicity add 16 to VSI ID.
 p4rt-ctl add-entry br0 linux_networking_control.tx_source_port_v4 \
     "vmeta.common.vsi=24,zero_padding=0,action=linux_networking_control.set_source_port(40)"


# Create a mapping between overlay VF (VSI-24/source port-40) and ACC port representor (VSI-18)
 p4rt-ctl add-entry br0 linux_networking_control.source_port_to_pr_map \
     "user_meta.cmeta.source_port=40,zero_padding=0,action=linux_networking_control.fwd_to_vsi(34)"

 p4rt-ctl add-entry br0 linux_networking_control.tx_acc_vsi \
     "vmeta.common.vsi=17,zero_padding=0,action=linux_networking_control.l2_fwd_and_bypass_bridge(0)"

# Create a mapping for traffic to flow between VSIs (VSI-24/source port-40) and (VSI-18)
 p4rt-ctl add-entry br0 linux_networking_control.vsi_to_vsi_loopback \
      "vmeta.common.vsi=18,target_vsi=24,action=linux_networking_control.fwd_to_vsi(40)"

 p4rt-ctl add-entry br0 linux_networking_control.vsi_to_vsi_loopback \
     "vmeta.common.vsi=24,target_vsi=18,action=linux_networking_control.fwd_to_vsi(34)"
```

### Configure supporting p4 runtime tables

For TCAM entry configure LPM LUT table

```bash
 p4rt-ctl add-entry br0 linux_networking_control.ipv4_lpm_root_lut \
     "user_meta.cmeta.bit32_zeros=4/255.255.255.255,priority=65535,action=linux_networking_control.ipv4_lpm_root_lut_action(0)"
```

Create a dummy LAG bypass table for all 8 hash indexes

```bash
 p4rt-ctl add-entry br0  linux_networking_control.tx_lag_table \
     "user_meta.cmeta.lag_group_id=0,hash=0,action=linux_networking_control.bypass"

 p4rt-ctl add-entry br0  linux_networking_control.tx_lag_table \
     "user_meta.cmeta.lag_group_id=0,hash=1,action=linux_networking_control.bypass"

 p4rt-ctl add-entry br0  linux_networking_control.tx_lag_table \
     "user_meta.cmeta.lag_group_id=0,hash=2,action=linux_networking_control.bypass"

 p4rt-ctl add-entry br0  linux_networking_control.tx_lag_table \
     "user_meta.cmeta.lag_group_id=0,hash=3,action=linux_networking_control.bypass"

 p4rt-ctl add-entry br0  linux_networking_control.tx_lag_table \
     "user_meta.cmeta.lag_group_id=0,hash=4,action=linux_networking_control.bypass"

 p4rt-ctl add-entry br0  linux_networking_control.tx_lag_table \
     "user_meta.cmeta.lag_group_id=0,hash=5,action=linux_networking_control.bypass"

 p4rt-ctl add-entry br0  linux_networking_control.tx_lag_table \
     "user_meta.cmeta.lag_group_id=0,hash=6,action=linux_networking_control.bypass"

 p4rt-ctl add-entry br0  linux_networking_control.tx_lag_table \
     "user_meta.cmeta.lag_group_id=0,hash=7,action=linux_networking_control.bypass"
```

### Create integration bridge and add ports to the bridge

Create OvS bridge, VxLAN tunnel and assign overlay VFs port representor to individual bridges.
Reference provided for single overlay network, repeat similar steps for other VFs.

Each bridge has:

- One overlay PR which is Native-tagged to a specific VLAN
- One VxLAN port which is Native-untagged to the VLAN and with different remote TEP and VNI

```bash
ovs-vsctl add-br br-1
ovs-vsctl add-port br-1 enp0s1f0d1 tag=10 vlan_mode=native-tagged
ovs-vsctl add-port br-1 vxlan1 tag=10 vlan_mode=native-untagged -- set interface vxlan1  type=vxlan \
    options:local_ip=10.1.1.1 options:remote_ip=10.1.1.2 options:key=10 options:dst_port=4789
```

Note: Here we are creating a VxLAN tunnel with VNI 10, you can create any VNI for tunneling.

### Underlay configuration

Create TEP termination bridge, add physical port's port representor and APF netdev port representor.
Reference provided for underlay network, repeat similar steps for multiple underlay networks.

```bash
ovs-vsctl add-br br-tun-1
ovs-vsctl add-port br-tun-1 enp0s1f0d9
ovs-vsctl add-port br-tun-1 enp0s1f0d10
```

Configure underlay IP address on the TEP termination port, route to reach remote IP is on termination bridge, and add change routes to reach remote IP.

```bash
# Create a dummy port and add TEP local IP
ip link add dev TEP10 type dummy
ifconfig  TEP10 10.1.1.1/24 up

# On termiantion bridge, configure an IP to reach remote TEP, and modify route to include nexthop details.
ifconfig br-tun-1 1.1.1.1/24 up
ip route change 10.1.1.0/24 via 1.1.1.2 dev br-tun-1
```

### Test the ping scenarios

- Underlay ping
- Overlay ping: Ping between VMs on different hosts
