<!--
/*
 * Copyright 2022-2023 Intel Corporation.
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at:
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
- -->

# Linux Networking for DPDK

This document explains how to run Linux networking scenario.

## Topology

![Linux Networking Topology](dpdk-lnw-topology.png)

Notes about topology:

- VHOST ports, TAP ports, Physical LINK ports are created by the GNMI CLI,
  and LINK port is bound to the DPDK target.
- VLAN 1, VLAN 2, .... VLAN N created using Linux commands and are on top of a
  TAP port. These VLAN ports should be equal to number of VM's that are spawned.
- br-int, VxLAN0 ports are created using ovs-vsctl command provided by the
  networking recipe and all the VLAN ports are attached to br-int using the
  ovs-vsctl command.

System under test will have above topology running the networking recipe.
Link Partner can have either the networking recipe or legacy OVS or kernel
VxLAN.  Note the limitations below before setting up the topology.

## Create P4 artifacts

- Install p4c compiler from p4lang/p4c (<https://github.com/p4lang/p4c>)
  repository and follow the readme for procedure.

- Set the OUTPUT_DIR environment variable to the location where artifacts
  should be generated and p4 files are available. For example:

  ```bash
  export OUTPUT_DIR=$IPDK_RECIPE/p4src/linux_networking/
  ```

- Generate the artifacts using p4c-dpdk installed in previous step using
  the following command:

  ```bash
  p4c-dpdk --arch pna --target dpdk \
      --p4runtime-files $OUTPUT_DIR/p4Info.txt \
      --bf-rt-schema $OUTPUT_DIR/bf-rt.json \
      --context $OUTPUT_DIR/context.json \
      -o $OUTPUT_DIR/linux_networking.spec \
      $OUTPUT_DIR/linux_networking.p4
  ```

- Modify sample lnw.conf file available in $IPDK_RECIPE/p4src/linux_networking/
  to specify absolute path of the artifacts (json and spec files).

- Generate binary file using the following tdi-pipeline builder command:

  ```bash
  tdi_pipeline_builder --p4c_conf_file=lnw.conf --bf_pipeline_config_binary_file=lnw.pb.bin
  ```

## Creating the topology

The [gnmi-ctl](/clients/gnmi-ctl) and [p4rt-ctl](/clients/p4rt-ctl) utilities
used below are in the $P4CP_INSTALL/bin directory.
They should be run with `sudo`.

### 1. Bind physical port (Port 0 and Port 1) to user-space IO driver

Load uio and vfio-pci driver:

```bash
modprobe uio
modprobe vfio-pci
```

Bind the devices to DPDK using the dpdk-devbind.py script:

```bash
cd $SDE_INSTALL/bin
./dpdk-devbind.py --bind=vfio-pci PCI-BDF
```

For example,

```bash
./dpdk-devbind.py --bind=vfio-pci 0000:18:00.0
```

PCI-BDF can be obtained using the lspci command.
Check if the device is bound correctly using `./dpdk-devbind.py -s`.
<!-- (See the section "Network devices using DPDK-compatible driver") -->

### 2. Export environment variables and start infrap4d

```bash
alias sudo='sudo PATH="$PATH" HOME="$HOME" LD_LIBRARY_PATH="$LD_LIBRARY_PATH" SDE_INSTALL="$SDE_INSTALL"'

sudo $P4CP_INSTALL/bin/infrap4d
```

### 3. Create two VHOST user ports

```bash
gnmi-ctl set "device:virtual-device,name:net_vhost0,host-name:host1,device-type:VIRTIO_NET,queues:1,socket-path:/tmp/vhost-user-0,packet-dir:host,port-type:LINK"

gnmi-ctl set "device:virtual-device,name:net_vhost1,host-name:host2,device-type:VIRTIO_NET,queues:1,socket-path:/tmp/vhost-user-1,packet-dir:host,port-type:LINK"
```

### 4. Create two physical link ports with control port

```bash
gnmi-ctl set "device:physical-device,name:PORT0,control-port:TAP1,pci-bdf:0000:18:00.0,packet-dir:network,port-type:link"

gnmi-ctl set "device:physical-device,name:PORT1,control-port:TAP2,pci-bdf:0000:18:00.1,packet-dir:network,port-type:link"
```

Note: Specify the pci-bdf of the devices bound to user-space in step 1.
Corresponding control port for physical link port will be created if
control port attribute is specified.

### 5. Create two TAP ports

```bash
gnmi-ctl set "device:virtual-device,name:TAP0,pipeline-name:pipe,mempool-name:MEMPOOL0,mtu:1500,packet-dir:host,port-type:TAP"

gnmi-ctl set "device:virtual-device,name:TAP3,pipeline-name:pipe,mempool-name:MEMPOOL0,mtu:1500,packet-dir:host,port-type:TAP"
```

Note:

- Pkt-dir parameter is to specify the direction of traffic.
It can take two values: host/network.
Value 'host' specifies that traffic on this port will be internal (within the host).
Value 'network' specifies that a particular port can receive traffic from network.

- The number of ports created should be a power of two to satisfy DPDK requirements.
When counting the number of ports, count control ports created along with
physical link port (e.g. TAP1 and TAP2).

### 6. Spawn two VM's on vhost-user ports created in step 3, start VM's, and assign IP addresses

```bash
ip addr add 99.0.0.1/24 dev eth0
ip link set dev eth0 up
ip addr add 99.0.0.2/24 dev eth0
ip link set dev eth0 up
```

### 7. Bring up the TAP or dummy interfaces

#### Option 1: Use one of the TAP ports as tunnel termination and assign IP address to the TAP port

```bash
ip link set dev TAP0 up
ip addr add 40.1.1.1/24 dev TAP1
ip link set dev TAP1 up
ip link set dev TAP2 up
ip link set dev TAP3 up
```

#### Option 2: Create a dummy port and use it for tunnel termination

Route to reach dummy port will be statically configured on peer or this
route will be re-distributed to the peer via routing protocols available
from FRR.

```bash
ip link add dev TEP1 type dummy

ip link set dev TAP0 up
ip link set dev TAP1 up
ip link set dev TAP2 up
ip link set dev TAP3 up
ip link set dev TEP1 up
```

### 8. Set the pipeline

```bash
p4rt-ctl set-pipe br0 lnw.pb.bin p4Info.txt
```

### 9. Run ovs-vswitchd server and ovsdb-server

Kill any existing ovs process if running.

```bash
mkdir -p $P4CP_INSTALL/var/run/openvswitch
rm -rf $P4CP_INSTALL/etc/openvswitch/conf.db

sudo $P4CP_INSTALL/bin/ovsdb-tool create \
    $P4CP_INSTALL/etc/openvswitch/conf.db \
    $P4CP_INSTALL/share/openvswitch/vswitch.ovsschema

export RUN_OVS=$P4CP_INSTALL

sudo $P4CP_INSTALL/sbin/ovsdb-server \
    --remote=punix:$RUN_OVS/var/run/openvswitch/db.sock \
    --remote=db:O
    pen_vSwitch,Open_vSwitch,manager_options \
    --pidfile --detach

sudo $P4CP_INSTALL/sbin/ovs-vswitchd --detach --no-chdir \
    unix:$RUN_OVS/var/run/openvswitch/db.sock --mlockall \
    --log-file=/tmp/ovs-vswitchd.log

sudo $P4CP_INSTALL/bin/ovs-vsctl \
    --db unix:$RUN_OVS/var/run/openvswitch/db.sock show

sudo $P4CP_INSTALL/bin/ovs-vsctl add-br br-int

ifconfig br-int up
```

### 10. Configure VXLAN port

#### Option 1: When one of the TAP ports is used for tunnel termination

```bash
sudo $P4CP_INSTALL/bin/ovs-vsctl add-port br-int vxlan1 -- \
    set interface vxlan1 type=vxlan options:local_ip=40.1.1.1 \
    options:remote_ip=40.1.1.2 options:dst_port=4789
```

#### Option 2: When a dummy port is used for tunnel termination

Remote IP is on a different network. Route to reach peer needs to be
statically configured or learned via FRR.

```bash
sudo $P4CP_INSTALL/bin/ovs-vsctl add-port br-int vxlan1 --\
     set interface vxlan1 type=vxlan options:local_ip=40.1.1.1 \
     options:remote_ip=30.1.1.1 options:dst_port=4789
```

Note: VXLAN destination port should always be standard port, i.e. 4789.
(limitation of p4 parser)

### 11. Configure VLAN ports on TAP0 and add them to br-int

```bash
ip link add link TAP0 name vlan1 type vlan id 1
ip link add link TAP0 name vlan2 type vlan id 2
sudo $P4CP_INSTALL/bin/ovs-vsctl add-port br-int vlan1
sudo $P4CP_INSTALL/bin/ovs-vsctl add-port br-int vlan2
ip link set dev vlan1 up
ip link set dev vlan2 up
```

VLAN interfaces should be created on top of TAP ports, and should always be
in lowercase format "vlan+vlan_id".

### 12. Configure rules to push and pop VLAN from vhost 0 and 1 ports to TAP0 port (vhost-user and vlan port mapping)

Note: Port numbers used in p4rt-ctl commands are target datapath indexes
(unique identifier for each port) which can be queried using the
commands below. In the current SDE implementation, tdi-portin-id and
tdi-portout-id are the same.

```bash
gnmi-ctl get "device:virtual-device,name:net_vhost0,tdi-portin-id"
gnmi-ctl get "device:virtual-device,name:net_vhost0,tdi-portout-id"
```

Target DP index of control TAP port will be Target DP index of the
corresponding physical port + 1.  If the ports are created in the order
specified in the above step, target datapath indexes will be:

| Port name          | DP index |
| ------------------ | :------: |
| vhost-user-0 (VM1) | 0        |
| vhost-user-1 (VM2) | 1        |
| phy-port0          | 2        |
| TAP1               | 3        |
| phy-port1          | 4        |
| TAP2               | 5        |
| TAP0               | 6        |
| TAP3               | 7        |

For any tx control packet from VM1 (TDP 0), pipeline should add a VLAN tag
1 and send it to TAP0 port (TDP 6):

```bash
p4rt-ctl add-entry br0 linux_networking_control.handle_tx_control_pkts_table \
  "istd.input_port=0,action=linux_networking_control.push_vlan_fwd(6,1)"
```

For any tx control packet from VM2 (TDP 1), pipeline should add a VLAN tag
2 and send it to TAP0 port (TDP 6):

```bash
p4rt-ctl add-entry br0 linux_networking_control.handle_tx_control_pkts_table \
  "istd.input_port=1,action=linux_networking_control.push_vlan_fwd(6,2)"
```

For any tx control packet from TAP0 port (TDP 6) with VLAN tag 1, pipeline
should pop the VLAN tag and send it to VM1 (TDP 0):

```bash
p4rt-ctl add-entry br0 linux_networking_control.handle_tx_control_vlan_pkts_table \
  "istd.input_port=6,local_metadata.vlan_id=1,action=linux_networking_control.pop_vlan_fwd(0)"
```

For any tx control packet from TAP0 port (TDP 6) with VLAN tag 2, pipeline
should pop the VLAN tag and send it to VM2 (TDP 1):

```bash
p4rt-ctl add-entry br0 linux_networking_control.handle_tx_control_vlan_pkts_table \
  "istd.input_port=6,local_metadata.vlan_id=2,action=linux_networking_control.pop_vlan_fwd(1)"
```

### 13. Configure rules for control packets to or from physical port

Any rx control packet from phy port0 (TDP 2) should be sent to corresponding
control port TAP1 (TDP 3):

```bash
p4rt-ctl add-entry br0 linux_networking_control.handle_rx_control_pkts_table \
  "istd.input_port=2,action=linux_networking_control.set_control_dest(3)"
```

Any rx control packet from phy port1 (TDP 4) should be sent to corresponding
control port TAP2 (TDP 5):

```bash
p4rt-ctl add-entry br0 linux_networking_control.handle_rx_control_pkts_table \
  "istd.input_port=4,action=linux_networking_control.set_control_dest(5)"
```

Any tx control packet from control TAP1 port (TDP 3) should be sent to corresponding physical port phy port0 (TDP 2):

```bash
p4rt-ctl add-entry br0 linux_networking_control.handle_tx_control_pkts_table \
  "istd.input_port=3,action=linux_networking_control.set_control_dest(2)"
```

Any tx control packet from control TAP2 port (TDP 5) should be sent to corresponding physical port phy port1 (TDP 4):

```bash
p4rt-ctl add-entry br0 linux_networking_control.handle_tx_control_pkts_table \
  "istd.input_port=5,action=linux_networking_control.set_control_dest(4)"
```

### 14. Configure routes only when dummy port is used for tunnel termination

#### Option 1: Configure static route

```bash
ip addr add 40.1.1.1/24 dev TEP1
ip addr add 50.1.1.1/24 dev TAP1
ip route add 30.1.1.1 nexthop via 50.1.1.2 dev TAP1
```

#### Option 2: Learn dynamic routes via FRR (iBGP route distribution)

Install FRR:

- Install FRR via default package manager, like `apt install frr` for Ubuntu
  or `dnf install frr` for Fedora.

- If not, see the [official FRR documentation](https://docs.frrouting.org/en/latest/installation.html)
  and install according to your distribution.

Configure FRR:

- Modify /etc/frr/daemons to enable bgpd daemon
- Restart FRR service: `systemctl restart frr`
- Start VTYSH process, which is a CLI provided by FRR for user configuration
- Set the following configuration on the DUT (host1) for single-path scenario:

  ```bash
  interface TAP1
  ip address 50.1.1.1/24
  exit
  !
  interface TEP1
  ip address 40.1.1.1/24
  exit
  !
  router bgp 65000
  bgp router-id 40.1.1.1
  neighbor 50.1.1.2 remote-as 65000
  !
  address-family ipv4 unicast
    network 40.1.1.0/24
  exit-address-family
  ```

- Once Peer is also configured, we should see neighbor 50.1.1.2 is learnt on
DUT (host1) and also route learnt on the kernel.

  ```bash
  30.1.1.0/24 nhid 54 via 50.1.1.2 dev TAP1 proto bgp metric 20
  ```

### 15. Test the ping scenarios

- Ping between VM's on the same host
- Underlay ping
- Overlay ping: Ping between VM's on different hosts

## Limitations

Current SAI enablement for the networking recipe has following limitations:

- Always all VHOST user ports need to be configured first and only then TAP ports/physical ports.
- TAP port created for the corresponding link port should be created using
  "gnmi-ctl control port creation got the link port". For example,

  ```bash
  gnmi-ctl set "device:physical-device,name:PORT0,pipeline-name:pipe,mempool-name:MEMPOOL0,control-port:TAP1,mtu:1500,pci-bdf:0000:18:00.0,packet-dir=network,port-type:link
  ```

- All VLAN interfaces created on top of TAP ports, should always be in lowercase format "vlan+vlan_id"
Ex: vlan1, vlan2, vlan3, ... vlan4094
- br-int port, vxlan0 port and adding vlan ports to br-int need to be done after loading the pipeline.
- VxLAN destination port should always be standard port. i.e., 4789. (limitation by p4 parser)
- Only VNI 0 is supported.
- We are not supporting any ofproto rules which would not allow for FDB learning on OVS.
