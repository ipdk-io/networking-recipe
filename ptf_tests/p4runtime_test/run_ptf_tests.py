#!/usr/bin/env python
# Copyright 2013-present Barefoot Networks, Inc.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#    http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

import sys
import os
import subprocess
import argparse
import six
#root_dir = '/root/STRATUM-BFRT/bf_sde/9.3.0/bf-sde-9.3.0/packages/ptf-modules-9.3.0/ptf-utils'

parser = argparse.ArgumentParser()
parser.add_argument("--test-dir", required=True,
                    help="directory containing the tests")
parser.add_argument("--ptf", required=False,
                    default="ptf",
                    help="PTF executable (default: ptf)")
parser.add_argument("--arch", required=False, default="Tofino",
                    help="Architecture (Tofino or SimpleSwitch)")
parser.add_argument("--target", required=False, default="asic-model",
                    help="Target (asic-model or hw)")
parser.add_argument("--port-info", required=False,
                    default="None",
                    help="json file containing the port mapping")
parser.add_argument("--grpc-server", required=False, default="localhost",
                    help="GRPC server IP address")
parser.add_argument("--test-params", required=False, default="",
                    help="test params str")
parser.add_argument("--p4c", required=False, default="",
                    help="compiler version")
args, unknown_args = parser.parse_known_args()
 
if __name__ == "__main__":

    new_args = unknown_args
    my_dir = os.path.dirname(os.path.realpath(__file__))
    # Add the install location of this file to the import path.
    new_args += ["--pypath", my_dir]
    # Add the install location of python packages. This is where P4
    # program-specific PD Thrift files are installed.
    new_args += ["--pypath", os.path.join(my_dir, '..')]
    new_args += ["--test-dir", args.test_dir]

    if "--openflow" in args:
        new_args +=  ["-S 127.0.0.1", "-V1.3"]

    #Example: 1@eth1 or 0-1@eth2 (use eth2 as port 1 of device 0)
    if args.port_info is not None and args.port_info != "None":
        import json
        file_handler = open(args.port_info)
        port_map = json.load(file_handler)
        ports = port_map['port_map']
        for key in ports.keys():
            new_args += ["-i", str(ports[key][0]) + "-" + str(ports[key][1]) + "@" + str(key)]
        #new_args += ["--interface", json.dumps(port_map['port_map'])]
    if args.port_info is None or args.port_info == "None":
        new_args += ["-i", "0-0@lo"]

    kvp = "arch='%s';" % (args.arch.lower())
    kvp += "target='%s';" % (args.target.lower())
    
    if args.test_params != "":
        kvp += args.test_params + ";"

    kvp += "grpc_server='%s';" % (args.grpc_server)
    
    new_args += ["--test-params=%s" % (kvp)]

    env = os.environ.copy()
    six.print_(os.pathsep, type(os.pathsep))
    six.print_(os.environ.get("PYTHONPATH", ""),type(os.environ.get("PYTHONPATH", "")))
    env["PYTHONPATH"] = os.pathsep + os.environ.get("PYTHONPATH", "")
    child = subprocess.Popen([args.ptf] + new_args, env=env)
    child.wait()
    sys.exit(child.returncode)

