# Compiling P4 Programs for ES2K

## 1. Overview

This document explains how to install and use the Intel&reg; IPU E2100 P4 Compiler
(`p4c`) to compile a P4 program to build artifacts including the .pkg that can be 
deployed on ES2K. 

The list of supported host OS'es are listed in "Supported Operating Systems" section in the IPU SWG.

## 2. Build and run the P4 Tools container
The p4 compiler and cpt are p4 tools which can be installed by installing the 
prebuilt RPMs for P4 tools for Rocky Linux 9.2. Refer to 
"Getting Started Guide with P4 on P4 Tools Container" in the IPU SWG for 
instructions on building the P4Tools container and launching it.
The p4 tools will be available inside the container.


### 2.1 Launch the P4Tools container and confirm the tool versions

```bash
Launch the P4Tools container on the host.
[user@host P4Tools]$ sudo  docker exec -it /<p4_tools_container_name> /bin/bash

Inside the container, verify the compiler version.
[root@a54d354e447e /]# p4c --version
p4c 1.2.3.7 (SHA:  BUILD: release)

```

The compiler should now be ready for use.

## 3. Build Reference P4 programs

The `p4-programs` directory in the SDK folder contains a number of sample
P4 programs. The SDK is contained in a tar file intel-ipu-sdk-source-code-<$VERSION>.tgz.
Extract the p4-programs from the tarball.

```bash
[root@host ~]# ls -l $INTEL-IPU-SDK-<VERSION>/tools/pipeline/p4-programs
total 1100
-rw-r--r--.  1 user user     79 Dec 13 15:54 CONTENTS
drwxr-xr-x. 10 user user   4096 Dec 13 15:54 fxp-cxp-features
drwxr-xr-x. 10 user user   4096 Dec 13 15:54 fxp-net-scenarios
drwxr-xr-x.  7 user user    106 Dec 13 15:54 layer-2-forwarding
drwxr-xr-x.  8 user user    116 Dec 13 15:54 layer-3-forwarding
-rw-r--r--.  1 user user  81529 Dec 23 03:09 Makefile
-rw-r--r--.  1 user user   1494 Dec 13 15:54 mev_sample.conf
drwxr-xr-x.  2 user user    131 Dec 13 15:54 testsuite

```

Each P4 program has its own directory and an accompanying README file that
provides instructions on how to configure the IPU pipeline.

## 3.1 Compiling a P4 Program

Use `p4c` i.e the compiler driver to compile and build packages. 
We will be using one of the reference programs mentioned above as an example -
p4-programs/layer-3-forwarding/l3-fwd_p2p. The Makefile contains 
the exact commands to  build the artifacts and .pkg for all the reference programs.

```bash
# 
1. Copy the entire p4-programs directory from the host to the P4Tools container
#sudo docker cp p4-programs/ <p4Tools container id>:/opt/

2. Set environment variables in the container.
# Set the following env variables  in the container prior to building
[root@a54d354e447e p4-programs]# which p4c
/opt/p4-tools/p4c/bin/p4c
[root@a54d354e447e p4-programs] export P4C_PATH=/opt/p4-tools/p4c/bin
export CPT_PATH=/opt/p4-tools/cpt/bin
export PATH=$P4C_PATH:$CPT_PATH:$PATH
export LD_LIBRARY_PATH=$P4C_PATH/../lib:/usr/local/lib:$LD_LIBRARY_PATH
export PREV_ARTIFACT_DIR=



3. Compile l3-fwd_p2p,  all artifacts will be generated in artifacts directory.
[root@a54d354e447e ~]# cd /opt/p4-programs
[root@a54d354e447e p4-programs]# make l3-fwd_p2p
# Compiling the /opt/p4-programs/layer-3-forwarding/l3-fwd_p2p/l3-fwd_p2p.p4 and generating the runtime files and assembly...
/opt/p4-tools/p4c/bin/p4c --target idpf --arch pna -I/opt/p4-tools/p4c/bin/../lib -I/opt/p4-tools/p4c/bin/../share/p4c/p4include 
-I/opt/p4-tools/p4c/bin/../share/p4c/idpf-lib --package-by-domain --p4runtime-files ./artifacts/l3-fwd_p2p/p4Info.txt --save-temps -Xp4c 
"--Wdisable --no-pedantic --context ./artifacts/l3-fwd_p2p/context.json --bfrt ./artifacts/l3-fwd_p2p/bf-rt.json" 
--save-temps --npic --format csr --pkg-version 1.2 --pkg-name "FXP Package" 
-Xassembler ".cpt_ver.s" /opt/p4-programs/layer-3-forwarding/l3-fwd_p2p/l3-fwd_p2p.p4 
-o artifacts/l3-fwd_p2p

 4. All the artifacts including the .pkg will be in the artifacts directory.
[root@a54d354e447e p4-programs]# ls -lrt artifacts/l3-fwd_p2p/
total 2884
-rw-r--r--. 1 root root 950608 Dec 22 23:57 l3-fwd_p2p_0b32dac62c9b4c18b3213e04a6bb8c5b.pkgo
-rw-r--r--. 1 root root   3045 Dec 23 00:18 p4Info.txt
-rw-r--r--. 1 root root   8866 Dec 23 00:18 bf-rt.json
-rw-r--r--. 1 root root  30390 Dec 23 00:18 context.json
-rw-r--r--. 1 root root  37269 Dec 23 00:18 l3-fwd_p2p.s
-rw-r--r--. 1 root root 950608 Dec 23 00:18 l3-fwd_p2p_b222a542c1474685bd70a36994d16101.pkgo
-rw-r--r--. 1 root root 950608 Dec 23 00:18 l3-fwd_p2p.pkg 

```
These files are called _P4 artifacts_.


## Deploying P4 programs
Please see [Deploying P4 Programs](deploying-p4-programs.md)
for details about deployment.
