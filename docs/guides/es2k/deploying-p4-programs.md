# Deploying P4 Programs for ES2K

## 1. Overview

This document explains how to deploy the P4 artifacts on hardware flexible
pipeline (FXP).

Note: This document assumes that you have followed
[Compiling P4 programs guide](https://github.com/ipdk-io/networking-recipe/blob/main/docs/guides/es2k/compiling-p4-programs.md)
to compile and generate artifacts required for deployment.

## 2. Deploying on Hardware Flexible Pipeline (FXP)

Data Path Control Plane (DPCP) starts with a default P4 package. To load a
custom P4 package follow below steps:

### 2.1 Interrupt default startup routine

Reboot IMC and type ``N`` when the following message is shown on IMC console::

```text
start ipumgmtd and auxiliary script [Y/N] \
(default start both automatically after 10 seconds)?
```

### 2.2 Copy the custom P4 package

Copy the custom P4 package (.pkg) in `/etc/dpcp/package` directory and
overwrite the `default_pkg.pkg`.

For example, replace `default_pkg.pkg` with `simple_l3_l4_pna.pkg`

```bash
root@mev-imc:/etc/dpcp/package# ls -lrt /etc/dpcp/package/
total 2364
-rw-r--r-- 1 root root  963032 Jan  1 04:56 simple_l3_l4_pna.pkg
-rw-r--r-- 1 root root 1450456 Jun  8  2023 e2100-default-1.0.3.0.pkg
drwxr-xr-x 2 root root       0 Jun  8  2023 runtime_files
drwxr-xr-x 3 root root       0 Jun  8  2023 ref_pkg
lrwxrwxrwx 1 root root      25 Jun  8  2023 default_pkg.pkg -> e2100-default-1.0.3.0.pkg
root@mev-imc:/etc/dpcp/package# cp simple_l3_l4_pna.pkg default_pkg.pkg
```

If COMMS channel support is required, follow
[Enable COMMS channel](https://github.com/ipdk-io/networking-recipe/blob/main/docs/guides/es2k/enabling-comms-channel.md)
before proceeding to next step.

### 2.3 Start the IMC

Run the IMC start-up script.

```bash
root@mev-imc:~# /etc/init.d/run_default_init_app
```

By default, `cpf_host` parameter in `/etc/dpcp/cfg/cp_init.cfg` is set to 4 which
enables ACC. If the start-up script is executed successfully, ACC comes up with a
statically assigned IP address `192.168.0.2` to the eth0 network interface.
You can access ACC from IMC over an SSH session using this IP address.

To start infrap4d on ACC, follow [ES2K Running Infrap4d guide](https://github.com/ipdk-io/networking-recipe/blob/main/docs/guides/es2k/running-infrap4d.md)
