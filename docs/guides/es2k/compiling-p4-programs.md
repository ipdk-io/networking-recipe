# Compiling P4 Programs for ES2K

## 1. Overview

This document explains how to install the Intel&reg; IPU E2100 P4 Compiler
(`p4c-pna-xxp`) and Configurable Pipeline Tool (`cpt`), and use
them to compile a P4 program

## 2. Installing the P4 Compiler and Tools

### 2.1 Install RPMs

Install the `cpt`, `p4c-pna-xxp`, and `p4-sde` packages on a Fedora 37 x86_64 server,
from RPMs in the release tarball.

  ```bash
  # Extract RPMs from the tarball
  tar xvzf mev-hw-b0-<release>-fedora37.tgz
  cd host/packages/x86_64/

  # Install RPMs on server
  dnf localinstall cpt*.rpm --allowerasing
  dnf localinstall p4c*.rpm --allowerasing
  dnf localinstall p4-sde*.rpm --allowerasing
  ```

The packages will be installed under `/usr` directory. Note that these
RPMs are not relocatable.

### 2.2 Confirm tool versions

After installing the RPMs, verify the version numbers of the executables:

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

### 2.3 Address "no such file" error

If the `p4c-pna-xxp --version` command displays the following error message:

```text
error while loading shared libraries: libboost_iostreams.so.1.69.0:\
cannot open shared object file: No such file or directory
```

You will need to download and install the correct version of the Boost
libraries.

```bash
wget https://boostorg.jfrog.io/artifactory/main/release/1.69.0/source/boost_1_69_0.tar.gz
tar -xf boost_1_69_0.tar.gz
cd boost_1_69_0/
./bootstrap.sh
./b2
./b2 install
```

Verify the compiler version number:

```bash
[root@host ~] # p4c-pna-xxp --version
p4c-pna-xxp
Version 3.0.70.7
```

The compiler should now be ready for use.

## 3. P4 Reference Files

The `/usr/share/mev_reference_p4_files/` directory contains a number of sample
P4 programs.

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

Each P4 program has its own directory and an accompanying README file that
provides instructions on how to configure the IPU pipeline.

## 4. Compiling a P4 Program

Use `p4c-pna-xxp` to compile a P4 program. We will be using one of the reference
programs mentioned above.

```bash
# Set environment variables
export LD_LIBRARY_PATH=/usr/lib:/usr/lib64:/usr/local/lib:/usr/local/lib64:$LD_LIBRARY_PATH
export OUTPUT_DIR=/usr/share/mev_reference_p4_files/simple_l3_l4_pna

# Compile p4 program
p4c-pna-xxp -I/usr/lib -I/usr/share/p4c/p4include -I/usr/share/p4c/idpf-lib \
            $OUTPUT_DIR/simple_l3_l4_pna.p4 -o $OUTPUT_DIR/simple_l3_l4_pna.s \
            --p4runtime-files $OUTPUT_DIR/simple_l3_l4_pna.p4info.txt \
            --context $OUTPUT_DIR/simple_l3_l4_pna.context.json \
            --bfrt $OUTPUT_DIR/simple_l3_l4_pna.bf-rt.json
```

The compiler will generate the following files:

- simple_l3_l4_pna.p4info.txt
- simple_l3_l4_pna.bf-rt.json
- simple_l3_l4_pna.context.json
- simple_l3_l4_pna.s

These files are called _P4 artifacts_.

## 5. Generating a Pkg File

Use `cpt` to prepare the P4 artifacts for deployment:

```bash
cpt --npic --device idpf --format csr --pbd  -o simple_l3_l4_pna.pkg \
    cpt_ver.s simple_l3_l4_pna.s
```

Please see [Deploying P4 Programs](https://github.com/ipdk-io/networking-recipe/blob/main/docs/guides/es2k/deploying-p4-programs.md)
for details about deployment.
