#!/usr/bin/env python3
#
# Copyright 2024 Intel Corporation.
# SPDX-License-Identifier: Apache-2.0
#
# Generates P4RoleConfig proto file from P4Info input file
# 
# This is a helper script that extracts all table IDs from the specified input
# file and tags as 'exclusive_p4_ids'. User must review and edit as necessary
# for the client role configuration.
# See https://github.com/stratum/stratum/blob/main/stratum/public/proto/p4_role_config.md
# for details
# 

import re
import argparse

def parse_input(input_file):
    with open(input_file, 'r') as file:
        input_text = file.read()

    # Split the input text into blocks
    blocks = re.split(r'\n(?=\w)', input_text)

    ids_and_names = []

    # Iterate over each block
    for block in blocks:
        # If the block contains a preamble
        if 'preamble' in block:
            # Extract the id and name values
            id_value = re.search(r'id: (\d+)', block)
            name_value = re.search(r'name: "(.*?)"', block)
            if id_value and name_value:
                # Append the id and name to the list as a tuple
                ids_and_names.append((int(id_value.group(1)), name_value.group(1)))

    return ids_and_names

def write_output(ids_and_names, output_file):
    with open(output_file, 'w') as file:
        for id, name in ids_and_names:
            file.write(f"exclusive_p4_ids: {id}\n")
        file.write(f"receives_packet_ins: false\n")
        file.write(f"can_push_pipeline: false\n")

parser = argparse.ArgumentParser(description='This script extracts P4 table ID \
                                values from the input P4Info.txt file and \
                                writes them to the output role_config.txt file.')
requiredArgs = parser.add_argument_group("required arguments")
requiredArgs.add_argument("-i", "--input", required=True,
                          help="input file path")
requiredArgs.add_argument("-o", "--output", required=True,
                          help="output file path")
args = parser.parse_args()


ids_and_names = parse_input(args.input)
# print(ids_and_names) # debug
write_output(ids_and_names, args.output)
