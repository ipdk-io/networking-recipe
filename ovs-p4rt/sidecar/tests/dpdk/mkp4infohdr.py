#!/usr/bin/env python3
#
# Copyright 2024 Intel Corporation.
# SPDX-License-Identifier: Apache-2.0
#
# Converts a P4Info.txt file to a C++ header file.
#

import argparse
import logging
import os

DEFAULT_INFILE = "p4Info.txt"
DEFAULT_OUTFILE = "p4info_text.h"

PREAMBLE = \
"""// Copyright 2024 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

#ifndef ES2K_P4INFO_TEXT_H_
#define ES2K_P4INFO_TEXT_H_

// P4Info for ES2K Linux Networking.
constexpr char P4INFO_TEXT[] = R"p4(
"""

POSTAMBLE = \
""")p4";

#endif  // ES2K_P4INFO_TEXT_H_
"""

def create_header(infile, outfile):
    with open(infile, 'r') as file:
        print("Reading", infile)
        p4info = file.read()

    with open(outfile, 'w') as file:
        print("Writing", outfile)
        file.write(PREAMBLE)
        file.write(p4info)
        file.write(POSTAMBLE)

def create_parser():
    parser = argparse.ArgumentParser(
        prog="p4info_to_hdr",
        description="Generates C++ header file from a P4Info.txt file.")

    parser.add_argument("--input", "-i", dest="infile", default=DEFAULT_INFILE,
                        help=f"input file path (default: {DEFAULT_INFILE})")

    parser.add_argument("--output", "-o", dest="outfile",
                        default="p4info_text.h",
                        help=f"input file path (default: {DEFAULT_OUTFILE})")

    return parser

if __name__ == "__main__":
    logging.basicConfig(level=logging.INFO)

    parser = create_parser()
    args = parser.parse_args()
    create_header(args.infile, args.outfile)

# end __main__
