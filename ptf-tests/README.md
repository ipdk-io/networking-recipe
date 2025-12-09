# IPDK Networking Recipe tests using PTF framework

---

# Introduction

PTF is a Python based dataplane test framework. It is based on unittest, which
is included in the standard Python distribution. 
More details on PTF: https://github.com/p4lang/ptf 

This document is meant to provide a step by step guide on how to run the ipdk tests using the ptf framework

---

# Infrastructure details for Automation to run

The ptf_tests repo contains test cases for a varied type of scenarios. Keeping everything in mind the kind of setup that will run all will look like:

<img width="524" alt="Auto Infra" src="https://user-images.githubusercontent.com/93581264/200238574-baad8411-9d91-4272-ad93-8e647a77dfc3.png">

1. Linux Networking related tests (scripts having 'lnt'/'LNT' in their name"):
Two Fedora/Ubuntu servers connected back to back using an NIC 

2. Link Port test cases (scripts having "link" in their name"):
One Fedora/Ubuntu server with 2-port NIC. These 2 ports are connected in loopback

We need to bind only the first port (PCI ID XX:YY.0) with vfio-pci driver:

Enable intel_iommu for binding the device

Load uio or vfio-pci driver       

```console
~ modprobe uio                       

~ modprobe vfio-pci
```

Bind the devices to dpdk(vfio-pci)

```console
~ cd $SDE_INSTALL/bin

~ ethtool -i <interface name> ; take bus id and use for below command.
 
~ ./dpdk-devbind.py --bind=vfio-pci <pci_bdf>          eg: ./dpdk-devbind.py --bind=vfio-pci 0000:18:00.0
```

Check if device is bound correctly: `./dpdk-devbind.py -s`


3. VM / vhost related test cases (tests_to_run.txt file will have VM tags VM1, VM2, ...):
 
We need to create 4 copies of Ubuntu VM images in our DUT. These images are used to run vhost/hot plug test cases. 


4. Enable hugepages:

E.g.

```console
echo 1024 > /sys/devices/system/node/node0/hugepages/hugepages-2048kB/nr_hugepages

echo 1024 > /sys/devices/system/node/node1/hugepages/hugepages-2048kB/nr_hugepages
```

---

# Placeholder Tags in tests_to_run.txt

In general the test case script name and its corresponding test parameters are mentioned in tests_to_run.txt file. If running one test case at a time and not in a suite, you can refer to this file. While referring, please replace the below tags. These are placeholder tags for running the tests in a suite: 
 
VM{i} with absolute path of VM images: E.g. `ptf --test-dir tests/ DPDK_Hot_Plug_multi_port_test --pypath $PWD --test-params="config_json='DPDK_Hot_Plug_multi_port_test.json';vm_location_list='/home/admin2/VM/ubuntu-20.04-server-cloudimg-amd64_1.img,/home/admin2/VM/ubuntu-20.04-server-cloudimg-amd64_2.img'" --platform=dummy`
 
BDF{i} with the PCI ID of port(s) bound to dpdk: E.g. `ptf --test-dir tests/ l3_wcm_with_link_port --pypath $PWD --test-params="config_json='l3_wcm_with_link_port.json';pci_bdf='0000:af:00.0'" --platform=dummy`
 
For LNT test cases we have some more parameters. #TBD 

---

# Directory Structure

The following directory structure is a pre-requisite to run the tests. All steps should be run from the main directory, ptf_tests.

```text
.
├── common
│   ├── config
│   │   └── l3_exact_match_dst_ip_tap.json
│   ├── __init__.py
│   ├── lib
│   │   ├── __init__.py
│   │   └── ovs_p4ctl.py
│   ├── p4c_artifacts
│   │   └── l3_exact_match_dst_ip
│   │       ├── p4Info.txt
│   │       └── simple_l3.pb.bin
│   └── utils
│       ├── __init__.py
│       └── ovsp4ctl_utils.py
├── __init__.py
├── pre_test.sh
├── requirements.txt
└── tests
    ├── __init.py__
    └── l3_exact_match_dst_ip_with_tap_port.py

```

---

# Pre-requisite

ipdk networking recipe, P4SDE and P4C should be installed before running the tests

---

# Installing Dependencies

```console
~ cd <ptf_tests>

~ python3 -m pip install -r requirements.txt
```

# Pre Test

```console
~ source pre_test.sh <SDE_INSTALL_PATH> <IPDK_RECIPE_PATH> <DEPEND_INSTALL_PATH>
```

# Running the test


## Running a single test

```console
~ ptf --test-dir tests/ <test_script_name_without_extension> --pypath $PWD --test-params="config_json='<config json file name>'" --platform=dummy
```

E.g. `ptf --test-dir tests/ l3_exact_match_with_tap_port --pypath $PWD --test-params="config_json='l3_exact_match_with_tap.json'" --platform=dummy`


## Running tests in a suite

To run multiple tests in a suite, we need to use the script ipdk_test_runner.py.

E.g. `python ipdk_test_runner.py -f tests_to_run.txt -s  /root/dkaranjai/drop7/p4sde/install/ -o /root/dkaranjai/drop7/ipdk-recipe/ -d /root/dkaranjai/drop7/DEP_LIB/ -vm /home/saldju/VM/ubuntu-20.04-server-cloudimg-amd64_1.img,/home/saldju/VM/ubuntu-20.04-server-cloudimg-amd64_5.img,/home/saldju/VM/ubuntu-20.04-server-cloudimg-amd64_2.img,/home/saldju/VM/ubuntu-20.04-server-cloudimg-amd64_4.img -lnt_bdf 0000:af:00.0,0000:af:00.1 -bdf 0000:c1:00.0 -client '10.233.134.175,root,cloudsw' -port ens802f0,ens802f1 --log_file ptf_tests_run_frr_1.log --verbose`

```console
~ python ipdk_test_runner.py -h

usage: ipdk_test_runner.py [-h] -f FILE -s P4SDE_INSTALL_PATH -o IPDK_RECIPE_PATH -d DEP_LIB [-vm VM_LOCATION_LIST] [-bdf PCI_BDF] [-lnt_bdf LNT_BDF1,LNT_BDF2] -client '<remote_ip>,<username>,<password>'] [-port REMOTE_PORT1,REMOTE_PORT2] [-l LOG_FILE] [--verbose]

mandatory arguments:
    -f FILE, --file FILE  Reads the test suite file default location ptf_tests/ . if kept in a different location, then mention absolute file name. This
                        file consists tests scripts to run (without .py extension) and the corresponding "test-params"
    -s P4SDE_INSTALL_PATH, --p4sde_install_path P4SDE_INSTALL_PATH
                        Absolute P4SDE Install path
    -o IPDK_RECIPE_PATH, --ipdk_recipe_path IPDK_RECIPE_PATH
                        Absolute IPDK Recipe path
    -d DEP_LIB, --dep_lib_path DEP_LIB_PATH
                        Absolute Dependency Lib path

optional arguments:
    -h, --help            show this help message and exit
    -vm VM_LOCATION_LIST, --vm_location_list VM_LOCATION_LIST
                        Absolute vm image location path(s) separated by comma
    -bdf PCI_BDF, --pci_bdf PCI_BDF
                        PCI BDF list separated by comma
    -lnt_bdf LNT_BDF, --lnt_pci_bdfs LNT_BDF
                        PCI BDF connected back to back for LNT scenario, separated by comma
    -port REMOTE_PORT, --remote_port REMOTE_PORT
                        REMOTE_PORT list separated by comma
    -client CLIENT_CRED, --client_cred CLIENT_CRED
                        CLIENT cretials in the format of hostname, user,passwrod
    -l LOG_FILE, --log_file LOG_FILE
                        name of the log file, by default located in ptf_tests/
    --verbose prints ptf logs in the console

```

# Post Test

[TBD]

# Reading Results

## Log File
All ptf autogenerated logs can be found at ptf_tests/ptf.log

## Console Output

Individual steps start with "PASS" or "FAIL" which shows their execution status.

Example

```text
FAIL: ovs-p4ctl set pipe Failed with error: P4Runtime RPC error (FAILED_PRECONDITION): Only a single forwarding pipeline can be pushed for any node so far.

Scenario1 : l3 exact match with destination IP

Adding rule for port 0 and 1 with destination IP

PASS: ovs-p4ctl add entry: headers.ipv4.dst_addr=1.1.1.1,action=ingress.send(0)

PASS: ovs-p4ctl add entry: headers.ipv4.dst_addr=1.1.1.2,action=ingress.drop(1)

Test has PASSED

```

## Consolidated output

The consolidated output is applicable only when running multiple tests. Refer to "Running tests in a suite".
To store the test logs we need to provide -l or --log_file option with the name of the log file.
Additionally, if --verbose is enabled, the log file will contain the full execution logs, 
else it will contain a summary.


E.g.

```text

====================

Summary

====================

l2_exact_match_with_tap_port : PASSED

l3_action_profile_with_tap_ports : PASSED

l3_action_selector_with_tap_ports : PASSED

l3_action_selector_with_tap_ports : PASSED

l3_exact_match_with_tap_port : PASSED




Ran 5 test(s), 5 Passed, 0 Failed.

```
