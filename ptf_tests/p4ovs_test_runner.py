# Copyright (c) 2022 Intel Corporation.
#
# SPDX-License-Identifier: Apache-2.0
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at:
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

import subprocess
import argparse
import os
import sys
import datetime
import re
import fileinput
import time

from itertools import dropwhile

def replaceAll(file,searchExp,replaceExp):
    for line in fileinput.input(file, inplace=1):
        if searchExp in line:
            line = line.replace(searchExp,replaceExp)
        sys.stdout.write(line)

class MyParser(argparse.ArgumentParser):
    def print_help(self):
        print ("""
usage: p4ovs_test_runner.py [-h] -f FILE -s P4SDE_INSTALL_PATH -o P4OVS_INSTALL_PATH [-vm VM_LOCATION_LIST] [-bdf PCI_BDF] [-d P4DEP_INSTALL_PATH] [-l LOG_FILE] [--verbose]

mandatory arguments:
    -f FILE, --file FILE  Reads the test suite file default location ptf_tests/ . if kept in a different location, then mention absolute file name. This
                        file consists tests scripts to run (without .py extension) and the corresponding "test-params"
    -s P4SDE_INSTALL_PATH, --p4sde_install_path P4SDE_INSTALL_PATH
                        Absolute P4SDE Install path
    -o P4OVS_INSTALL_PATH, --p4ovs_install_path P4OVS_INSTALL_PATH
                        Absolute P4OVS Install path

optional arguments:
    -h, --help            show this help message and exit
    -vm VM_LOCATION_LIST, --vm_location_list VM_LOCATION_LIST
                        Absolute vm image location path(s) separated by comma
    -bdf PCI_BDF, --pci_bdf PCI_BDF
                        PCI BDF list separated by comma
    -port REMOTE_PORT, --remote_port REMOTE_PORT
                        REMOTE_PORT list separated by comma
    -client CLIENT_CRED, --client_cred CLIENT_CRED
                        CLIENT cretials in the format of hostname, user,passwrod
    -d P4DEP_INSTALL_PATH, --p4dep_install_path P4DEP_INSTALL_PATH
                        Absolute P4OVS Dependency Install path
    -l LOG_FILE, --log_file LOG_FILE
                        name of the log file, by default located in ptf_tests/
    --verbose prints ptf logs in the console
    """)

help_message_f = """Reads the test suite file
default location ptf_tests/ .
if kept in a different location, then mention absolute file name.
This file consists tests scripts to run (without .py extension)
and the corresponding "test-params"
"""

parser = MyParser()
parser.add_argument('-f', '--file', type=str, required=True, help=help_message_f)
parser.add_argument('-s', '--p4sde_install_path', type=str, required=True,
                    help="Absolute P4SDE Install path")
parser.add_argument('-o', '--p4ovs_install_path', type=str, required=True,
                    help="Absolute P4OVS Install path")
parser.add_argument('-vm', '--vm_location_list', type=str, required=False,
                    help="Absolute vm image location path(s) separated by comma")
parser.add_argument('-bdf', '--pci_bdfs', type=str, required=False,
                    help="PCI BDF list separated by comma")
parser.add_argument('-port', '--remote_port', type=str, required=False,
                    help="Remote Port list separated by comma")
parser.add_argument('-client', '--client_cred', type=str, required=False,
                    help="Client Credential like hostname, user,password")
parser.add_argument('-d', '--p4dep_install_path', type=str, required=False,
                    help="Absolute P4OVS Dependency Install path")
parser.add_argument('-l', '--log_file', type=str, required=False,
                    help="name of the log file, by default located in ptf_tests/")
parser.add_argument('--verbose', action="store_true", help="prints ptf logs in the console")
args = parser.parse_args()

if not args.p4dep_install_path:
    args.p4dep_install_path = ""

# Check if 'file' exists
if not os.path.exists(args.file):
    print(f"File {args.file} doesn't exist")
    sys.exit()

subprocess.run("cp %s %s.bkp"%(args.file,args.file), shell=True)
try:
    # Dynamically update the tests_to_run file with the vm images
    if args.vm_location_list:
        test_params={}
        for k,v in enumerate(args.vm_location_list.split(',')):
            test_params['VM'+str(k+1)]=v
        for searchExp, replaceExp in test_params.items():
            print(f"replacing {searchExp} with {replaceExp}")
            replaceAll(args.file,searchExp,replaceExp)

    # Dynamically update the tests_to_run file with PCI BDF info
    if args.pci_bdfs:
        test_params={}
        for k,v in enumerate(args.pci_bdfs.split(',')):
            test_params['BDF'+str(k+1)]=v
        for searchExp, replaceExp in test_params.items():
            print(f"replacing {searchExp} with {replaceExp}")
            replaceAll(args.file,searchExp,replaceExp)

    # Dynamically update the tests_to_run file with remote port info
    if args.remote_port:
        test_params={}
        for k,v in enumerate(args.remote_port.split(',')):
            test_params['PORT'+str(k+1)]=v
        for searchExp, replaceExp in test_params.items():
            print(f"replacing {searchExp} with {replaceExp}")
            replaceAll(args.file,searchExp,replaceExp)

    # Dynamically update the tests_to_run file with client cred
    if args.client_cred:
        searchExp = "CLIENT"
        replaceExp = args.client_cred
        replaceAll(args.file,searchExp,replaceExp)
    
    # Check if ptf is installed as a binary
    out = subprocess.run("ptf --help", shell=True, capture_output=True)
    if not out.stdout:
        print("ptf is not installed as an executable")
        sys.exit()

    test_to_run = {}
    sequence = []
    with open(args.file) as fh:
        for i in fh.readlines():
            # skip if blank line
            if not i.strip():
                continue
            # skip if line starts with #
            if i.startswith("#"):
                continue
            items = i.strip().split(":")
            test_to_run[items[0].strip()] = ':'.join(items[1:]).strip()
            sequence.append(items[0])
    test_to_run['sequence'] = sequence

    results = {}
    for test in test_to_run['sequence']:
        time.sleep(2)
        process = subprocess.Popen('/bin/bash', stdin=subprocess.PIPE, stdout=subprocess.PIPE)
        cmd = f"""source pre_test.sh {args.p4sde_install_path} {args.p4ovs_install_path} {args.p4dep_install_path}
        sleep 2
        ptf --test-dir tests/ {test} --pypath $PWD --test-params="{test_to_run[test]}" --platform=dummy
        """
        print(f"\nRunning {test}\n")
        out, err = process.communicate(cmd.encode('utf-8'))
        try:
            out = out.decode('utf-8')
            print(out)
        except UnicodeDecodeError:
            results[test]="Test has FAILED"
            continue
        # discarding pre_test.sh logs
        results[test] = '\n'.join([x for x in list(dropwhile(lambda x: "Using packet manipulation module" not in x,
                                                            out.split('\n'))) if x])
    summary = []
    if args.log_file:
        fh = open(args.log_file, "w")
        fh.truncate()
        fh.seek(0)
        e = datetime.datetime.now()
        fh.write(f"P4OVS tests log as on : {e.strftime('%Y-%m-%d %H:%M:%S')}")
        for test in test_to_run['sequence']:
            if args.verbose:
                fh.write("\n\n"+"="*20+f"\n{test}\n"+"="*20+"\n")
                fh.write(results[test])

    pattern = r"Test has (PASSED|FAILED)"
    for test in test_to_run['sequence']:
        pattern_found = False
        for line in results[test].split('\n'):
            m = re.match(pattern, line)
            if m:
                pattern_found = True
                summary.append((test, m.groups()[0]))
        if not pattern_found:
            summary.append((test, "FAILED"))

    summary_out = "\n\n"+"="*20+"\nSummary\n"+"="*20+"\n"
    for i in summary:
        summary_out += f"{i[0]} : {i[1]}\n"

    passed_count = len([True for i in summary if i[1] == "PASSED"])
    failed_count = len([True for i in summary if i[1] == "FAILED"])

    cons_out = f"Ran {len(summary)} test(s), {passed_count} Passed, {failed_count} Failed."
    if args.log_file:
        fh.write(summary_out)
        fh.write(f"\n\n{cons_out}")
        fh.close()

    print(summary_out)
    print(f"\n\n{cons_out}")
except Exception as err:
    print(f"Exception occurred: {err}")
finally:
    print(f"Restoring {args.file}")
    subprocess.run("mv %s.bkp %s"%(args.file,args.file), shell=True)
