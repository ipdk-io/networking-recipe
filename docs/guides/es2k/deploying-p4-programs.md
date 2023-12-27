# Deploying P4 Programs for ES2K

## 1. Overview

This section explains how to deploy a P4 program on the hardware Flexible
Pipeline (FXP).

It is assumed that that you have already
[compiled the P4 program](compiling-p4-programs.md)
to generate the P4 artifacts required for deployment.

## 2. Deploying on Hardware Flexible Pipeline (FXP)

Data Path Control Plane (DPCP) starts with a default P4 package. To load a
custom P4 package follow below steps:

### 2.1 Copy the custom P4 package

Copy the custom P4 package (p4_custom.pkg) in `/work/scripts` directory to IMC.

### 2.2 Modify the script responsible for loading custom package

Replace the `p4_custom.pkg` with custom package name in `load_custom_pkg.sh` script.

Any modifications intended in node policy `cp_init.cfg` should be provided as part of
the same script.

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
else
    echo "No custom package found. Continuing with default package"
fi
```

If Communication Channel support is required,
[enable the communication channel](enabling-comm-channel.md)
before proceeding to the next step.

### 2.3 Reboot the IMC

```bash
root@mev-imc:~# reboot
```
Once the IMC reboots successfully, IPU is loaded with the custom P4 package.

By default, `cpf_host` parameter in node_policy is set to 4 which
enables ACC. If the IMC reboots successfully, ACC comes up with a
statically assigned IP address `192.168.0.2` to the eth0 network interface.
You can access ACC from IMC over an SSH session using this IP address.

You are now ready to [start infrap4d](running-infrap4d.md).
