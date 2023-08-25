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

# Linux Networking with ECMP (DPDK)

This document explains how to run Linux networking with ECMP enabled for underlay connectivity.

## Topology

![ECMP Toplogy](dpdk-ecmp-topology.png)

Notes about topology:

- VHOST ports, TAP ports, Physical LINK ports are created by GNMI CLI and LINK port is binded to the DPDK target
- VLAN 1, VLAN 2, .... VLAN N created using Linux commands and are on top a TAP port. These VLAN ports should be equal to number of VM's that are     spawned.
- br-int, VxLAN0 ports are created using ovs-vsctl command provided by the networking recipe and all the VLAN ports are attached to br-int using ovs-vsctl command.
- TEP port is of type dummy, created using ip utility command and this port is used as the VxLAN tunnel termination port.

System under test will have above topology running the networking recipe. Link Partner can have either the networking recipe or legacy OVS or kernel VxLAN. Both the Physical ports from System under test and Link Partner should be connected back to back. Note the limitations below before setting up the topology.

## Create P4 artifacts

- Install p4c compiler from p4lang/p4c(<https://github.com/p4lang/p4c>) repository and follow the readme for procedure
- Set the environment variable OUTPUT_DIR to the location where artifacts should be generated and where p4 files are available
  eg. export OUTPUT_DIR=/root/ovs/p4proto/p4src/linux_networking/
- Generate the artifacts using p4c-dpdk installed in previous step using command below:

  ```bash
  p4c-dpdk --arch pna --target dpdk --p4runtime-files $OUTPUT_DIR/p4Info.txt --bf-rt-schema $OUTPUT_DIR/bf-rt.json --context $OUTPUT_DIR/context.json -o $OUTPUT_DIR/linux_networking.spec linux_networking.p4
  ```

- Modify sample lnw.conf file available in $IPDK_RECIPE/p4src/linux_networking/ to specify absolute path of the artifacts (json and spec files)
- Generate binary execuatble using tdi-pipeline builder command below:

  ```bash
  tdi_pipeline_builder --p4c_conf_file=lnw.conf --bf_pipeline_config_binary_file=lnw.pb.bin
  ```

## Limitations

Current SAI implementation for the networking recipe has following limitations:

- Always all VHOST user ports need to be configured first and only then TAP ports/physical ports
- VM's Spawned on top of VHOST user ports should be UP and running, interfaces with in VM should be brought up before loading the "forwarding pipeline" (limitation by target)
- TAP port created for the corresponding link port should be created using "gnmi-ctl control port creation for the link port"
eg: gnmi-ctl set "device:physical-device,name:PORT0,pipeline-name:pipe,mempool-name:MEMPOOL0,control-port:TAP1,mtu:1500,pci-bdf:0000:18:00.0,packet-dir=network,port-type:link"
- All VLAN interfaces created on top of TAP ports, should always be in lowercase format "vlan+vlan_id"
Ex: vlan1, vlan2, vlan3 …. vlan4094
- br-int port, vxlan0 port and adding vlan ports to br-int need to be done after loading the pipeline.
- VxLAN destination port should always be standard port. i.e., 4789. (limitation by p4 parser)
- Only VNI 0 is supported.
- We are not supporting any ofproto rules which would not allow for FDB learning on OVS.
- Make sure underlay connectivity for both the nexthops is established before configuring multipath to reach the link partner. When using FRR, the routing protocol will establish underlay connectivity and redistribute routes.

## Steps to create the topology

### 1. Bind the physical port (Port 0 and Port 1) to user-space IO driver

- Load uio and vfio-pci driver

  ```bash
    modprobe uio
    modprobe vfio-pci
  ```

- Bind the devices to DPDK using dpdk-devbind.py script

  ```bash
  cd $SDE_INSTALL/bin
  ./dpdk-devbind.py --bind=vfio-pci <pci_bdf> eg: ./dpdk-devbind.py --bind=vfio-pci 0000:18:00.0
  ```

 *Note*: pci_bdf can be obtained using lspci command. Check if device is binded correctly using ./dpdk-devbind.py -s (Refer to the section "Network devices using DPDK-compatible driver")

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

Note: Specify the pci-bdf of the devices binded to user-space in step 1. Corresponding control port for physical link port will be created if control port attribute is specified.

### 5. Create two TAP ports

```bash
gnmi-ctl set "device:virtual-device,name:TAP0,pipeline-name:pipe,mempool-name:MEMPOOL0,mtu:1500,packet-dir:host,port-type:TAP"
gnmi-ctl set "device:virtual-device,name:TAP3,pipeline-name:pipe,mempool-name:MEMPOOL0,mtu:1500,packet-dir:host,port-type:TAP"
```

Note:

- Pkt-dir parameter is to specify the direction of traffic. It can take 2 values - host/network. Value 'host' specifies that traffic on this port will be internal(within the host). Value 'network' specifies that a particular port can receive traffic from network.
- Ensure that no. of ports created should be power of 2 to satisfy DPDK requirements and when counting no. of ports, count control ports created along with physical link port(eg.: TAP1 and TAP2)

### 6. Spawn two VM's on vhost-user ports created in step 3, start the VM's and assign IP's

```bash
ip addr add 99.0.0.1/24 dev eth0
ip link set dev eth0 up
ip addr add 99.0.0.2/24 dev eth0
ip link set dev eth0 up
```

### 7. Configure tunnel termination port of type dummy

```bash
ip link add dev TEP1 type dummy
```

### 8. Bring up the TAP and dummy interfaces

```bash
    ip link set dev TAP0 up
    ip link set dev TAP1 up
    ip link set dev TAP2 up
    ip link set dev TAP3 up
    ip link set dev TEP1 up
```

### 9. Set the pipeline

```bash
    p4rt-ctl set-pipe br0 lnw.pb.bin p4Info.txt
```

### 10. Start and run ovs-vswitchd server and ovsdb-server

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
    --remote=db:Open_vSwitch,Open_vSwitch,manager_options \
    --pidfile --detach

sudo $P4CP_INSTALL/sbin/ovs-vswitchd --detach --no-chdir \
    unix:$RUN_OVS/var/run/openvswitch/db.sock \
    --mlockall --log-file=/tmp/ovs-vswitchd.log

sudo $P4CP_INSTALL/bin/ovs-vsctl --db \
    unix:$RUN_OVS/var/run/openvswitch/db.sock show

sudo $P4CP_INSTALL/bin/ovs-vsctl add-br br-int
ifconfig br-int up
```

### 11. Configure VXLAN port

Using one of the TAP ports for tunnel termination.

```bash
    sudo $P4CP_INSTALL/bin/ovs-vsctl add-port br-int vxlan1 -- set interface vxlan1 type=vxlan options:local_ip=40.1.1.1 options:remote_ip=30.1.1.1 options:dst_port=4789
```

Note:

- VXLAN destination port should always be standard port. i.e., 4789. (limitation by p4 parser)
- Remote IP is on another network and route to reach this can be configured statically (refer section 14.1) or dynamically learn via routing protocols supported by FRR (refer section 14.2)

### 12. Configure VLAN ports on TAP0 and add them to br-int

```bash
    ip link add link TAP0 name vlan1 type vlan id 1
    ip link add link TAP0 name vlan2 type vlan id 2
    sudo $P4CP_INSTALL/bin/ovs-vsctl add-port br-int vlan1
    sudo $P4CP_INSTALL/bin/ovs-vsctl add-port br-int vlan2
    ip link set dev vlan1 up
    ip link set dev vlan2 up
```

Note:

- All VLAN interfaces should be created on top of TAP ports, and should always be in lowercase format "vlan+vlan_id. (Eg: vlan1, vlan2, vlan3 …. vlan4094)

### 13. Configure rules to push and pop VLAN from vhost 0 and 1 ports to TAP0 port (vhost-user and vlan port mapping)

Note: Port number used in p4rt-ctl commands are target datapath indexes(unique identifier for each port) which can be queried using following commands below. With current SDE implementation, both tdi-portin-id and tdi-portout-id are the same.

```bash
    gnmi-ctl get "device:virtual-device,name:net_vhost0,tdi-portin-id"
    gnmi-ctl get "device:virtual-device,name:net_vhost0,tdi-portout-id"

    Target DP index of control TAP port will be Target DP index of corresponding physical port + 1. If the ports are created in the order mentioned in the above step, target datapath indexes will be:

                   Port name          Target datapath index
                   vhost-user-0(VM1) - 0
                   vhost-user-1(VM2) - 1
                   phy-port0         - 2
                   TAP1              - 3
                   phy-port1         - 4
                   TAP2              - 5
                   TAP0              - 6
                   TAP3              - 7
```

- **Rule for: Any tx control packet from VM1(TDP 0), pipeline should add a VLAN tag 1 and send it to TAP0 port(TDP 6)**

  ```bash
  p4rt-ctl add-entry br0 linux_networking_control.handle_tx_control_pkts_table "istd.input_port=0,action=linux_networking_control.push_vlan_fwd(6,1)"
  ```

  **Rule for: Any tx control packet from VM2(TDP 1), pipeline should add a VLAN tag 2 and send it to TAP0 port(TDP 6)**

  ```bash
  p4rt-ctl add-entry br0 linux_networking_control.handle_tx_control_pkts_table "istd.input_port=1,action=linux_networking_control.push_vlan_fwd(6,2)"
  ```

- **Rule for: Any tx control packet from TAP0 port(TDP 6) with VLAN tag 1, pipeline should pop the VLAN tag and send it to VM1(TDP 0)**

  ```bash
  p4rt-ctl add-entry br0 linux_networking_control.handle_tx_control_vlan_pkts_table "istd.input_port=6,local_metadata.vlan_id=1,action=linux_networking_control.pop_vlan_fwd(0)"
  ```

- **Rule for: Any tx control packet from TAP0 port(TDP 6) with VLAN tag 2, pipeline should pop the VLAN tag and send it to VM2(TDP 1)**

  ```bash
  p4rt-ctl add-entry br0 linux_networking_control.handle_tx_control_vlan_pkts_table "istd.input_port=6,local_metadata.vlan_id=2,action=linux_networking_control.pop_vlan_fwd(1)"
  ```

### 14. Configure rules for control packets coming in and out of physical port

- **Rule for: Any rx control packet from phy port0(TDP 2) should be sent it to it corresponding control port TAP1(TDP 3)**

  ```bash
    p4rt-ctl add-entry br0 linux_networking_control.handle_rx_control_pkts_table "istd.input_port=2,action=linux_networking_control.set_control_dest(3)"
  ```

- **Rule for: Any rx control packet from phy port1(TDP 4) should be sent it to it corresponding control port TAP2(TDP 5)**

  ```bash
    p4rt-ctl add-entry br0 linux_networking_control.handle_rx_control_pkts_table "istd.input_port=4,action=linux_networking_control.set_control_dest(5)"
  ```

- **Rule for: Any tx control packet from control TAP1 port(TDP 3) should be sent it to it corresponding physical port phy port0(TDP 2)**

  ```bash
    p4rt-ctl add-entry br0 linux_networking_control.handle_tx_control_pkts_table "istd.input_port=3,action=linux_networking_control.set_control_dest(2)"
   ```

- **Rule for: Any tx control packet from control TAP2 port(TDP 5) should be sent it to it corresponding physical port phy port1(TDP 4)**

  ```bash
    p4rt-ctl add-entry br0 linux_networking_control.handle_tx_control_pkts_table "istd.input_port=5,action=linux_networking_control.set_control_dest(4)"
  ```

### 15. Configure ECMP for underlay connectivity

- Rule: To reach link partner multiple paths are configured, packets can be hashed to any port where the nexthop's ARP is learnt
- Nexthop is selected based on a 5-tuple parameter: source IPv4 address, destination IPv4 address, protocol type, UDP source port and UDP destination port of the overlay packet.

  - 15.1 Option 1: Configure static routes.

    ```bash
    ip addr add 40.1.1.1/24 dev TEP1
    ip addr add 50.1.1.1/24 dev TAP1
    ip addr add 60.1.1.1/24 dev TAP2
    ip route add 30.1.1.1 nexthop via 50.1.1.2 dev TAP1 weight 1 nexthop via 60.1.1.2 dev TAP2 weight 1
    ```

  - 15.2 Option 2: Learn dynamic routes via FRR (iBGP route distribution)

    - 15.2.1 Install FRR

      - Install FRR via default package manager, like "apt install frr" for Ubuntu /"dnf install frr" for Fedora.
      - If not, refer to official FRR documentation available at <https://docs.frrouting.org/en/latest/installation.html> and install according to your distribution.

    - 15.2.2 Configure FRR

      - Modify /etc/frr/daemons to enable bgpd daemon
      - Restart FRR service. systemctl restart frr
      - Start VTYSH process, which is a CLI for user configuration.
      - Set below configuration on the DUT (host1) for Multipath scenario.

        ```bash
        interface TAP1
        ip address 50.1.1.1/24
        exit
        !
        interface TAP2
        ip address 60.1.1.1/24
        exit
        !
        interface TEP1
        ip address 40.1.1.1/24
        exit
        !
        router bgp 65000
        bgp router-id 40.1.1.1
        neighbor 50.1.1.2 remote-as 65000
        neighbor 60.1.1.2 remote-as 65000
        !
        address-family ipv4 unicast
          network 40.1.1.0/24
        exit-address-family
        ```

      - Once Peer is also configured, we should see neighbors 50.1.1.2 and 60.1.1.2 ARP's are learnt on DUT (host1) and also route learnt on the kernel.

        ```bash
        30.1.1.0/24 nhid 72 proto bgp metric 20
          nexthop via 60.1.1.2 dev TAP2 weight 1
          nexthop via 50.1.1.2 dev TAP1 weight 1
        ```

### 16. Test the ping scenarios

- Underlay ping for both ECMP nexthop's
- Ping between VM's on the same host
- Underlay ping for VxLAN tunnel termination port
- Overlay ping: Ping between VM's on different hosts and validate hashing
