# Building and Deploying P4 Programs (ES2K)

- [1. Overview](#1-overview)
- [2. Install compiler and tools](#2-install-compiler-and-tools)
- [3. Generate P4 artifacts](#3-generate-p4-artifacts)
   - [3.1 Generate artifacts](#31-generate-artifacts)
   - [3.2 Generate .pkg file](#32-generate-pkg-file)
   - [3.3 P4 reference files](#33-P4-reference-files)

## 1. Overview

This document explains how to install Intel® IPU E2100 P4 Compiler
`p4c-pna-xxp`and Intel® Configurable Pipeline Tool `cpt`, and use
them to build P4 artifacts.

## 2. Install compiler and tools

Install the `cpt`, `p4c` and `p4-sde` tools, using prebuilt RPMs
available as part of the release package, on a Fedora 37 x86_64 server.

  ```bash
  # Extract RPMs from the tarball
  tar xvzf mev-hw-b0-<release>-fedora37.tgz
  cd host/packages/x86_64/

  # Install RPMs from tarball
  dnf localinstall cpt*.rpm --allowerasing
  dnf localinstall p4c*.rpm --allowerasing
  dnf localinstall p4-sde*.rpm --allowerasing
  ```

Note: Default location for installed packages is under `/usr`, and these rpms
are not relocatable

- After installing the RPMs, confirm the version of the cpt and p4c-pna-xxp.

  ```bash
  # Set environment variables
  export LD_LIBRARY_PATH=/usr/lib:/usr/lib64:/usr/local/lib:/usr/local/lib64:$LD_LIBRARY_PATH

  [root@host ~]# cpt --version
  Intel(R) Configurable Pipeline Tool Version 3.8.0.16
  Copyright (C) 2022 Intel Corporation.  All rights reserved.

  [root@host ~] # p4c-pna-xxp --version
  p4c-pna-xxp
  Version 3.0.70.7
  ```

  Note: If you see boost lib version mismatch while running p4c-pna-xxp, install
that boost from source tar ball to fix the problem.

  ```bash
  p4c-pna-xxp --version
  p4c-pna-xxp: error while loading shared libraries: \
  libboost_iostreams   -.so.1.69.0: cannot open shared object file \:
  No such file or directory
  ```

  Before compiling boost_1_69_0, ensure Development Tools are installed.

   ```bash
   wget https://boostorg.jfrog.io/artifactory/main/release/1.69.0/source/boost_1_69_0.tar.gz
   tar -xf boost_1_69_0.tar.gz
   cd boost_1_69_0/
   ./bootstrap.sh
   ./b2
   ./b2  install
   ```

## 3. Generate P4 artifacts

Use the `p4c-pna-xxp` and `cpt` tools installed to create artifacts.

### 3.1 Generate artifacts

```bash
# Set the LD_LIBRARY_PATH if not set
export LD_LIBRARY_PATH=/usr/lib:/usr/lib64:/usr/local/lib:/usr/local/lib64:$LD_LIBRARY_PATH
export OUTPUT_DIR=/usr/share/mev_reference_p4_files/simple_l3_l4_pna

# Generate artifacts using p4c-pna-xxp
p4c-pna-xxp -I/usr/lib -I/usr/share/p4c/p4include -I/usr/share/p4c/idpf-lib \
            $OUTPUT_DIR/simple_l3_l4_pna.p4 -o $OUTPUT_DIR/simple_l3_l4_pna.s \
            --p4runtime-files $OUTPUT_DIR/simple_l3_l4_pna.p4info.txt \
            --context $OUTPUT_DIR/simple_l3_l4_pna.context.json \
            --bfrt $OUTPUT_DIR/simple_l3_l4_pna.bf-rt.json
```

Note: The above commands will generate three files (simple_l3_l4_pna.p4info.txt,
simple_l3_l4_pna.bf-rt.json, and simple_l3_l4_pna.context.json)

### 3.2 Generate .pkg file

```bash
cpt --npic --device idpf --format csr --pbd  -o simple_l3_l4_pna.pkg \
cpt_ver.s simple_l3_l4_pna.s
```

### 3.3 P4 reference files

More p4 files and corresponding README's are available under
`/usr/share/mev_reference_p4_files/`.

```bash
    [root@host ~]# ls -l /usr/share/mev_reference_p4_files/
    total 28
    -rw-r--r--. 1 root root   79 Jun  8 08:06 CONTENTS
    drwxr-xr-x. 2 root root 4096 Jun  8 03:21 linux_networking
    -rw-r--r--. 1 root root 1104 Jun  8 08:06 mev_sample.conf
    drwxr-xr-x. 2 root root 4096 Jun  8 03:21 pna_connection_track
    drwxr-xr-x. 2 root root 4096 Jun  8 03:21 simple_l2_demo
    drwxr-xr-x. 2 root root 4096 Jun  8 03:21 simple_l2_mod_demo
    drwxr-xr-x. 2 root root 4096 Jun  8 03:21 simple_l3_l4_pna
```

Note: Each P4 program has a README file with instructions to configure Intel
IPU pipeline.
