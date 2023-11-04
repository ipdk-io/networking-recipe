#!/usr/bin/env python3
#
# Copyright 2023 Intel Corporation.
# SPDX-License-Identifier: Apache-2.0
#
# Generates a script from a template file.
#

import argparse
import logging
import os

from jinja2 import Environment
from jinja2 import FileSystemLoader

# Templates are in the same directory as this script.
TEMPLATE_PATH = os.path.dirname(__file__)

VALID_TARGETS = ['dpdk', 'es2k', 'tofino']

logger = logging.getLogger('genscript')

errcount = 0

def error(msg, *args, **kwargs):
    """Logs an error and increments the error count."""
    global errcount
    logger.error(msg, *args, **kwargs)
    errcount += 1
    return

#-----------------------------------------------------------------------
# generate_script()
#-----------------------------------------------------------------------
def generate_script(args):
    gendir = os.path.dirname(__file__)
    env = Environment(loader=FileSystemLoader(gendir))
    template = env.get_template(args.template)
    rendered = template.render(target=args.target)
    with open(args.output, 'w') as fd:
        fd.write(rendered)
    return

#-----------------------------------------------------------------------
# parse_cmd_line()
#-----------------------------------------------------------------------
def parse_cmd_line():
    parser = create_parser()
    args = parser.parse_args()
    process_args(args)
    return args

def create_parser():
    parser = argparse.ArgumentParser(
        prog='genfile.py',
        description='Generates a script from a template file.')

    parser.add_argument('--template', '-t', type=str,
                        help='template file')
    parser.add_argument('--output', '-o', help='output file')
    parser.add_argument('--target', type=str,
                        help='target to build {}'.format('|'.join(VALID_TARGETS)))

    return parser

def process_args(args):
    process_output_arg(args)
    process_target_arg(args)
    process_template_arg(args)
    return

def process_output_arg(args):
    if args.output is None:
        error("'--output' parameter not specified")
        return
    args.output = os.path.abspath(os.path.expanduser(args.output))
    return

def process_target_arg(args):
    if args.target is None:
        error("'--target' parameter not specified")
        return
    target = args.target.lower()
    if target in VALID_TARGETS:
        args.target = target
    else:
        error("Invalid target type: '%s'", args.target)
    return

def process_template_arg(args):
    if args.template is None:
        error("'--template' parameter not specified")
        return
#   args.template = os.path.abspath(os.path.expanduser(args.template))
    return

#-----------------------------------------------------------------------
# main - Main program
#-----------------------------------------------------------------------
if __name__ == '__main__':
    logging.basicConfig(level=logging.INFO)

    args = parse_cmd_line()
    if errcount:
        exit(1)

    generate_script(args)
# end __main__
