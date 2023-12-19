# Enabling the Communication Channel

The Communication channel permits different compute complexes on the IPU to
communicate with each other. This document explains how to enable communication
between the ACC and the Host. This allows the P4Runtime client `p4rt-ctl`
running on the Host to communicate with infrap4d running on the ACC.

Ports used for communication channels are defined by the node policy on IMC.

## 1 Modify the custom package config script on IMC

Modify the `load_custom_pkg.sh` script to specify comm_vports.

```bash
[root@ipu-imc /]# cd /work/scripts
[root@ipu-imc scripts]# cat load_custom_pkg.sh
#!/bin/sh
CP_INIT_CFG=/etc/dpcp/cfg/cp_init.cfg
echo "Checking for custom package..."
if [ -e p4_custom.pkg ]; then
    echo "Custom package p4_custom.pkg found. Overriding default package"
    cp  p4_custom.pkg /etc/dpcp/package/
    rm -rf /etc/dpcp/package/default_pkg.pkg
    ln -s /etc/dpcp/package/ p4_custom.pkg /etc/dpcp/package/default_pkg.pkg
    sed -i 's/sem_num_pages = 1;/sem_num_pages = 25;/g' $CP_INIT_CFG
    sed -i "s/comm_vports = ((\[5,0\],\[4,0\]))\;/comm_vports = ((\[5,0\],\[4,0\]),(\[0,3\],\[4,2\]))\;/g" $CP_INIT_CFG
else
    echo "No custom package found. Continuing with default package"
    sed -i "s/comm_vports = ((\[5,0\],\[4,0\]))\;/comm_vports = ((\[5,0\],\[4,0\]),(\[0,3\],\[4,2\]))\;/g" $CP_INIT_CFG

fi
```
This will enable communication between IMC-ACC and Host-ACC.

## 2. Reboot the IMC

```bash
root@mev-imc:~# reboot
```

If IMC is rebooted successfully, ACC comes up with a statically assigned IP address
 `192.168.0.2` to the eth0 network interface. You can access ACC from IMC over an
SSH session using this IP address.

## 3. Load the IDPF driver on Host

```bash
modprobe idpf
```

## 4. Identify Communication Channel Netdev

### 4.1 List the interfaces

Log on to IMC using `minicom IMC` or ssh from the host over IP address
`100.0.0.100` and run `cli_client`.

root@mev-imc# ./usr/bin/cli_client -q -c

 fn_id      | host_id  |  Is port a VF | VSI ID  | Vport ID    | is VF created   | Is VF enabled   | MAC Address
 ---------- | -------- | ------------- | ------- | ----------- | --------------- | --------------  | -----------
 fn_id: 0x0 | host_id: 0x0 | is_vf: no | vsi_id: 0x1 | vport_id 0x0 | is_created: yes | is_enabled: yes | mac addr: 00:01:00:00:03:14
 fn_id: 0x0 | host_id: 0x0 | is_vf: no | vsi_id: 0xb | vport_id 0x1 | is_created: yes | is_enabled: yes | mac addr: 00:0b:00:01:03:14
 fn_id: 0x0 | host_id: 0x0 | is_vf: no | vsi_id: 0xc | vport_id 0x2 | is_created: yes | is_enabled: yes | mac addr: 00:0c:00:02:03:14
 fn_id: 0x0 | host_id: 0x0 | is_vf: no | vsi_id: 0xd | vport_id 0x3 | is_created: yes | is_enabled: yes | mac addr: 00:0d:00:03:03:14  → (Host, vport 3)
 fn_id: 0x4 | host_id: 0x4 | is_vf: no | vsi_id: 0x2 | vport_id 0x0 | is_created: yes | is_enabled: yes | mac addr: 00:00:00:00:03:18
 fn_id: 0x4 | host_id: 0x4 | is_vf: no | vsi_id: 0x8 | vport_id 0x1 | is_created: yes | is_enabled: yes | mac addr: 00:08:00:01:03:18
 fn_id: 0x4 | host_id: 0x4 | is_vf: no | vsi_id: 0x9 | vport_id 0x2 | is_created: yes | is_enabled: yes | mac addr: 00:09:00:02:03:18 → (ACC, vport 2)
 fn_id: 0x4 | host_id: 0x4 | is_vf: no | vsi_id: 0xa | vport_id 0x3 | is_created: yes | is_enabled: yes | mac addr: 00:0a:00:03:03:18
 fn_id: 0x5 | host_id: 0x5 | is_vf: no | vsi_id: 0x3 | vport_id 0x0 | is_created: yes | is_enabled: yes | mac addr: 00:00:00:00:03:19
 fn_id: 0x5 | host_id: 0x5 | is_vf: no | vsi_id: 0x5 | vport_id 0x1 | is_created: yes | is_enabled: yes | mac addr: 00:05:00:01:03:19
 fn_id: 0x5 | host_id: 0x5 | is_vf: no | vsi_id: 0x6 | vport_id 0x2 | is_created: yes | is_enabled: yes | mac addr: 00:06:00:02:03:19
 fn_id: 0x5 | host_id: 0x5 | is_vf: no | vsi_id: 0x7 | vport_id 0x3 | is_created: yes | is_enabled: yes | mac addr: 00:07:00:03:03:19

### 4.2 Find ACC vport MAC address

```bash
root@mev-imc:/usr/bin/# ./cli_client -q -c | awk '{if(($2 == 0x4) && ($4 == 0x4) && ($10 == 0x2)) {print $17}}'
```

This should provide ACC vport MAC address: eg: 00:09:00:02:03:18

### 4.3 Find Host vport MAC address

```bash
root@mev-imc:/usr/bin/# ./cli_client -q -c | awk '{if(($2 == 0x0) && ($4 == 0x0) && ($10 == 0x3)) {print $17}}'
```

This should provide Host vport MAC address: eg: 00:0d:00:03:03:14

If Host ports are not seen, reload the `IDPF` driver on host using

```bash
rmmod idpf
modprobe idpf
```

## 5. Configure IP addresses

Once the ports are identified, assign the IP addresses to these interfaces.
Avoid IP collision when assigning IP addresses, 192.168.x.x network is assigned
for IMC and ACC communication by default.

Example:

On Host, check if the `IDPF` driver is loaded using lsmod

```bash
lsmod | grep idpf
# ens801f0d3 is the port with MAC addr 00:0d:00:03:03:14
nmcli device set ens801f0d3 managed no
ip addr add 5.5.5.10/24 dev ens801f0d3
```

On ACC, check if the `IDPF` driver is loaded using lsmod

```bash
lsmod | grep idpf
# enp0s1f0d2 is the port with MAC addr 00:09:00:02:03:18
nmcli device set enp0s1f0d2 managed no
ip addr add 5.5.5.5/24 dev enp0s1f0d2
```

## 6. Check communication between ACC and Host

From ACC:

```bash
ping 5.5.5.10
```

If node policy is correctly configured and the P4 package loaded has support for
communication channel, ping should work.
