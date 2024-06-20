# ovsp4rt scenarios

## OVS Component

### get_fdb_data

- fdb_info.mac_addr := _mac_addr_
- fdb_info.bridge_id := _bridge_id_
- fdb_info.vlan_info.port_vlan_mode := _vlan_mode_
- fdb_info.vlan_info.port_vlan := _port_vlan_

if port.is_tunnel:

- fdb_info.is_tunnel := true
- get underlay_ifindex
- get underlay_tnl
- fdb_info.tnl_info.ifindex := underlay_ifindex
- fdb_info.tnl_info.dst_port := underlay_tnl.dst_port
- fdb_info.tnl_info.vni := underlay_tnl.vni
- fdb_info.tnl_info.tunnel_type := _tunnel_type_
- if underlay_tnl.ipv6_src:
  - fdb_info.tnl_info.local_ip.family := AF_INET6
  - fdb_info.tnl_info.local_ip.ip.v6addr := underlay_tnl.ipv6_src
  - fdb_info.tnl_info.remote_ip.family := AF_INET6
  - fdb_info.tnl_info.remote_ip.ip.v6addr := underlay_tnl.ipv6_dst
- else:
  - fdb_info.tnl_info.local_ip.family := AF_INET
  - fdb_info.tnl_info.local_ip.ip.v4addr := underlay_tnl.ipv4_src
  - fdb_info.tnl_info.remote_ip.family := AF_INET
  - fdb_info.tnl_info.remote_ip.ip.v4addr := underlay_tnl.ipv4_dst

else:

- if port_name == "vlan":
  - fdb_info.src_port := _src_port_
  - fdb_info.rx_src_port := _rx_src_port_

### mac_learning_expire

- initialize fdb_info:
  - fdb_info.mac_addr := _mac_addr_
  - fdb_info.is_vlan := true
  - fdb_info.bridge_id := _bridge_id_
- [ovsp4rt_config_fdb_entry](#ovsp4rt_config_fdb_entry)
  - insert_entry = false
- inintialize ip_info:
  - ip_info.src_ip_addr.family := AF_INET
  - ip_info.src_ip_addr.ip.v4addr.s_addr := _src_
  - ip_info.dst_ip_addr.ip.family := AF_INTE
  - ip_info.dst_ip_addr.ip.v4addr.s_addr := _dst_
- [ovsp4rt_config_ip_mac_map_entry](#ovsp4rt_config_ip_mac_map_entry)
  - insert_entry = false

### update_ip_mac_map_info

(to be documented)

### xlate_normal

- [get_fdb_data](#get_fdb_data)
- [ovsp4rt_config_fdb_entry](#ovsp4rt_config_fdb_entry)
  - insert_entry = true
- [update_ip_mac_map_info](#update_ip_mac_map_info)
- [ovsp4rt_config_ip_mac_map_entry](#ovsp4rt_config_ip_mac_map_entry)
  - insert_entry = true

## OVSP4RT Component

### ovsp4rt_config_fdb_entry

if deleting:

- if GetL2ToTunnelV4TableEntry:
  - learn_info.is_tunnel := true
- else if GetL2ToTunnelV6TableEntry:
  - learn_info.is_tunnel := true

if learn_info.is_tunnel:

- if inserting:
  - if GetFdbTunnelTableEntry:
    - return
- ConfigFdbTunnelTableEntry
- ConfigL2TunnelTableEntry
- ConfigFdbSmacTableEntry
  - if inserting:
    - if GetFdbVlanTableEntry:
      - return
    - ConfigFdbRxVlanTableEntry
    - GetTxAccVsiTableEntry
    - extract src_port
    - learn_info.src_port := src_port
  - ConfigFdbTxVlanTableEntry
  - ConfigFdbSmacTableEntry

### ovsp4rt_config_ip_mac_map_entry

(to be documented)
