# Compiling P4 Programs for ES2K

This document explains how to install and use the Intel&reg; IPU E2100 P4Tools
Container to compile a P4 program to build artifacts, including the `.pkg`
file, that can be deployed on ES2K.

The supported host OSes are listed in the "Supported Operating Systems"
section of the IPU Software User Guide (RDC Doc#778226), henceforth referred to
as the "IPU SWG".

## Build and run the P4Tools container

The tools required to compile and build a custom p4 package
can be installed by building the P4 Tools Container.

See "Getting Started Guide with P4 on P4 Tools Container" in the
IPU SWG for instructions on building and launching the P4 Tools container.

Once <your_p4_tools_container> is sucessfully built, proceed to the next step.

### Launch the P4Tools container

```bash
# Launch the P4Tools container on the host.
[user@host P4Tools] sudo docker exec -it <your_p4_tools_container> /bin/bash

# Inside the container, verify the p4 compiler version.
[root@a54d354e447e /] p4c --version
p4c 1.2.3.7 (SHA:  BUILD: release)
```

The `p4c` compiler should now be ready for use.

In previous versions of the SDK, `cpt` had to be invoked separately.
It is no longer necessary to do so.
`cpt` will be invoked automatically by specifying cpt flags to `p4c`.

## Build reference P4 programs

The `p4-programs` directory in the SDK folder contains a number of sample P4
programs. The SDK is contained in a tar file named
`intel-ipu-sdk-source-code-<$VERSION>.tgz`.

Extract the p4-programs from the tarball.

```bash
[root@host ~] ls -l $INTEL-IPU-SDK-<VERSION>/tools/pipeline/p4-programs
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

### Compiling a P4 Program

Use the `p4c` compiler driver to compile and build packages.
We will be using one of the reference programs mentioned above as an
example: `p4-programs/layer-3-forwarding/l3-fwd_p2p`.

The Makefile contains the exact commands to build the artifacts
for all the reference programs.

1. Copy the entire p4-programs directory from the host to the P4Tools
   container.

   ```bash
   sudo docker cp p4-programs/ <p4Tools container id>:/opt/
   ```

2. Set environment variables in the container prior to building.

   ```bash
   [root@a54d354e447e p4-programs] which p4c
   /opt/p4-tools/p4c/bin/p4c
   [root@a54d354e447e p4-programs] export P4C_PATH=/opt/p4-tools/p4c/bin
   export CPT_PATH=/opt/p4-tools/cpt/bin
   export PATH=$P4C_PATH:$CPT_PATH:$PATH
   export LD_LIBRARY_PATH=$P4C_PATH/../lib:/usr/local/lib:$LD_LIBRARY_PATH
   export PREV_ARTIFACT_DIR=
   ```

3. Compile l3-fwd_p2p. All artifacts will be generated in the artifacts
   directory.

   ```bash
   [root@a54d354e447e ~] cd /opt/p4-programs

   # Compiling /opt/p4-programs/layer-3-forwarding/l3-fwd_p2p/l3-fwd_p2p.p4
   # and generating the runtime files and assembly...
   [root@a54d354e447e p4-programs] make l3-fwd_p2p
   /opt/p4-tools/p4c/bin/p4c --target idpf --arch pna
       -I/opt/p4-tools/p4c/bin/../lib -I/opt/p4-tools/p4c/bin/../share/p4c/p4include
       -I/opt/p4-tools/p4c/bin/../share/p4c/idpf-lib --package-by-domain
       --p4runtime-files ./artifacts/l3-fwd_p2p/p4Info.txt --save-temps 
       -Xp4c "--Wdisable --no-pedantic --context ./artifacts/l3-fwd_p2p/context.json
       --bfrt ./artifacts/l3-fwd_p2p/bf-rt.json"
       --save-temps --npic --format csr --pkg-version 1.2 --pkg-name "FXP Package"
       -Xassembler ".cpt_ver.s" /opt/p4-programs/layer-3-forwarding/l3-fwd_p2p/l3-fwd_p2p.p4
       -o artifacts/l3-fwd_p2p
    ```

4. All output files, including the `.pkg` file, will be in the artifacts
   directory.

   ```bash
   [root@a54d354e447e p4-programs] ls -lrt artifacts/l3-fwd_p2p/
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
