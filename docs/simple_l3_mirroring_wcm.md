# simple_l3_mirroring_wcm

## Overview
This document describes how to build pb.bin file, load the pipeline and rules to program for the given P4 example on ES2000 target.
Refer to ipdk-es2k.md as a reference document for 'P4C compilation', 'cpt packaging', component version details and other test execution details.

## Lookup table 
ExactMatchLookupTable is new P4 extern object type introduced to define profiles where key/keys and corresponding data field(s)
are present. It differs from the existing Match-Action table in the sense that, it doesn't have any action associated with it. 
Ideally this table can be configured with Exact or Ternary lookup keys. 
This new table is defined as a user defined extern object type for the PNA architecture. The corresponding .proto changes that are 
required to support this new table construct are introduced in p4runtime submodule, and the same needs to be open-sourced. 

## SDE reference for p4 and README_OVS
p4 program 
https://github.com/intel-innersource/networking.ethernet.acceleration.vswitch.p4-sde.p4-driver/tree/main/mev_reference_p4_files/simple_l3_mirror_wcm

README_OVS
https://github.com/intel-innersource/networking.ethernet.acceleration.vswitch.p4-sde.p4-driver/blob/main/mev_reference_p4_files/simple_l3_mirror_wcm/README_OVS

## Setup

### Install Wheel Package and p4Runtime files using p4runtime-1.3.0-py3-none-any.whl
a) uninstall existing p4runtime
b) Install wheel package
   #pip install wheel
c) install p4Runtime files using "p4runtime-1.3.0-py3-none-any.whl" wheel file
   #pip install p4runtime-1.3.0-py3-none-any.whl
	
### Pipeline build procedure:
a) Handcraft the 'simple_l3_mirror_wcm.conf' file by following the sample conffile 'mev_sample.conf' provided in the '/mev_reference_p4_files' directory.
b) Manually create the empty tofino.bin file in the same path as that is referred in 'config' parameter of the conf file. 
This file is required for the ovs_pipeline_builder utility.
   Ex:[root@host ~]# touch tofino.bin 
c) Run the pipeline builder utility
./install/bin/tdi_pipeline_builder --p4c_conf_file=$OUTPUT_DIR/simple_l3_mirror_wcm.conf \
    --bf_pipeline_config_binary_file=$OUTPUT_DIR/simple_l3_mirror_wcm.pb.bin

#### set/load the pipeline:
./install/bin/p4rt-ctl set-pipe br0 $OUTPUT_DIR/simple_l3_mirror_wcm.pb.bin $OUTPUT_DIR/p4Info.txt

#### Rules programming:
Add rule:
--------
./install/bin/p4rt-ctl add-lut-entry br0 my_control.mir_prof "mir_prof_key=18,data:vport_id=1,mode=0,port_dest_type=0,dest_id=1,func_valid=1,store_vsi=0"
./install/bin/p4rt-ctl add-entry bro my_control.i_fwd "hdrs.mac[vmeta.common.depth].da=0x000000000940,action=my_control.send(17)"
./install/bin/p4rt-ctl add-entry bro my_control.l3_l4_match "hdrs.ipv4[vmeta.common.depth].src_ip=192.168.1.0/255.255.255.0,hdrs.ipv4[vmeta.common.depth].dst_ip=192.168.2.0/255.255.255.0,priority=1,action=my_control.mirror_and_send(18)"

Delete rule:
-----------
./install/bin/p4rt-ctl del-lut-entry br0 my_control.mir_prof "mir_prof_key=18"
./install/bin/p4rt-ctl del-entry bro my_control.i_fwd "hdrs.mac[vmeta.common.depth].da=0x000000000940"
./install/bin/p4rt-ctl del-entry bro my_control.l3_l4_match "hdrs.ipv4[vmeta.common.depth].src_ip=192.168.1.0/255.255.255.0,hdrs.ipv4[vmeta.common.depth].dst_ip=192.168.2.0/255.255.255.0,priority=1"

#### Test traffic from link partner to ES2000
scapy packet : Generate tx traffic from CVL interface (ens2f0)
sendp(Ether(dst="00:00:00:00:09:40", src="9e:ba:ce:98:d9:d3")/IP(src="192.168.1.201", dst="192.168.2.198")/UDP(sport=1000, dport=2000)/Raw(load="0"*50), iface='ens2f0')

##### capture traffic using tcpdump on idpf interface on host.
tcpdump -XXi <idpf iface> #you will see there are two similar packets (one is original and second is mirrored packet).


