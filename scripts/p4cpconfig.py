#!/usr/bin/env python3
#
# Copyright 2023 Intel Corporation.
# SPDX-License-Identifier: Apache-2.0
#
# Generates P4 Control Plane build configuration files.
#

import argparse
import json
import logging
import os
from pprint import pprint
import sys

errcount = 0
warncount = 0

# Values that indicate a parameter is unspecified.
UNSPECIFIED = [None, 'None', '-', '']

# Implicit default configuration file.
LOCAL_CONFIG_FILE = '.p4cpconfig'
USER_CONFIG_FILE = '~/' + LOCAL_CONFIG_FILE

#-----------------------------------------------------------------------
# ConfigParam class - Represents a single configuration parameter.
#-----------------------------------------------------------------------
class ConfigParam:
    def __init__(self, name, cmake_name, env_name=None, value=None,
                 default=None, cmake_type=None, cmake_doc=None):
        self.name = name
        self.cmake_name = cmake_name
        self.env_name = env_name
        self._setvalue(value, default)
        self.cmake_type = cmake_type
        self.cmake_doc = cmake_doc
        return

    def defined(self):
        return self.value not in UNSPECIFIED

    def docstring(self):
        return "config: {}".format(self.cmake_doc)

    def dump(self):
        pprint(vars(self))
        return

    def write_cmake_variable(self, file):
        self._write_quoted_variable(file)
        return

    def write_env_variable(self, file):
        print("export {}={}".format(self.env_name, self.value), file=file)

    def _setvalue(self, value, default):
        if value not in UNSPECIFIED:
            self.value = value
        elif default not in UNSPECIFIED:
            self.value = default
        else:
            self.value = None
        return

    def _write_quoted_variable(self, outfile):
        print("set({var:s} \"{val}\" CACHE {type:s} \"{doc:s}\")".format(
            var=self.cmake_name, val=self.value, type=self.cmake_type,
            doc=self.docstring()),
              file=outfile)
        return

    def _write_unquoted_variable(self, outfile):
        print("set({var:s} {val} CACHE {type:s} \"{doc:s}\")".format(
            var=self.cmake_name, val=self.value, type=self.cmake_type,
            doc=self.docstring()),
              file=outfile)
        return
# class ConfigParam

#-----------------------------------------------------------------------
# ConfigParam subclasses.
#-----------------------------------------------------------------------
class BoolParam(ConfigParam):
    def __init__(self, name, cmake_name, env_name=None, value=None,
                 default=None, doc=None):
        ConfigParam.__init__(self, name, cmake_name, env_name, value, default,
                             cmake_type="BOOL", cmake_doc=doc)
        return

    def write_cmake_variable(self, file):
        self._write_unquoted_variable(file)
        return
# class BoolParam

class PathParam(ConfigParam):
    def __init__(self, name, cmake_name, env_name=None, value=None,
                 default=None, doc=None):
        ConfigParam.__init__(self, name, cmake_name, env_name, value, default,
                             cmake_type="PATH", cmake_doc=doc)
        return
# class PathParam

class StringParam(ConfigParam):
    def __init__(self, name, cmake_name, env_name=None, value=None,
                 default=None, doc=None):
        ConfigParam.__init__(self, name, cmake_name, env_name, value, default,
                             cmake_type="STRING", cmake_doc=doc)
        return
# class StringParam

#-----------------------------------------------------------------------
# Config class - Represents the collection of parameters.
#-----------------------------------------------------------------------
class Config:
    def __init__(self):
        self.params = {}
        self.cmake_vars = {}
        self.env_vars = {}
        return

    def define_bool_param(self, name, cmake_name, env_name=None, value=None,
                          default=None, doc=None):
        self._add_param(
            BoolParam(name, cmake_name, env_name, value, default, doc))
        return

    def define_path_param(self, name, cmake_name, env_name=None, value=None,
                          default=None, doc=None):
        self._add_param(
            PathParam(name, cmake_name, env_name, value, default, doc))
        return

    def define_string_param(self, name, cmake_name, env_name=None, value=None,
                            default=None, doc=None):
        self._add_param(
            StringParam(name, cmake_name, env_name, value, default, doc))
        return

    def define_target_param(self, name, cmake_name, env_name=None, value=None,
                            default=None, doc=None):
        self._add_param(
            StringParam(name, cmake_name, env_name, value, default, doc))
        return

    def dump(self):
        for param in self.params.values():
            param.dump()
        return

    def get_cmake_params(self):
        """Returns the cmake variable params as a sorted list."""
        keys = sorted(
            [k for k in self.cmake_vars.keys() if self.cmake_vars[k].defined()])
        return [self.cmake_vars[k] for k in keys]

    def get_env_params(self):
        """Returns the environment variable params as a sorted list."""
        keys = sorted(
            [k for k in self.env_vars.keys() if self.env_vars[k].defined()])
        return [self.env_vars[k] for k in keys]

    def get_json_params(self):
        """Returns the json params as a dictionary."""
        return {k:v.value for (k,v) in self.params.items() if v.defined()}

    def write_cmake_variables(self, outfile):
        """Writes the cmake variable definitions to a file."""
        for param in self.get_cmake_params():
            param.write_cmake_variable(outfile)
        return

    def write_env_variables(self, outfile):
        """Writes the environment variable definitions to a file."""
        for param in self.get_env_params():
            param.write_env_variable(outfile)
        return

    def write_json_params(self, outfile):
        """Writes the json parameter definitions to a file."""
        json_cfg = self.get_json_params()
        json.dump(json_cfg, outfile, indent=4)
        outfile.write('\n')
        return

    def _add_param(self, param):
        self.params[param.name] = param
        if param.cmake_name is not None:
            self.cmake_vars[param.cmake_name] = param
        if param.env_name is not None:
            self.env_vars[param.env_name] = param
        return

    def __contains__(self, key):
        return key in self.params

    def __getitem__(self, key):
        return self.params[key] if key in self.params else None

    def __len__(self):
        return len(self.params)
# end Config

#-----------------------------------------------------------------------
# Defaults - Default parameter values.
#-----------------------------------------------------------------------
class Defaults:
    def __init__(self):
        self.params = {}
        return

    def load(self, cfgfile):
        """Loads default parameters from a json file."""
        self.params = json.load(cfgfile)
        return

    def __contains__(self, key):
        return key in self.params

    def __getitem__(self, key):
        return self.params[key] if key in self.params else None

    def __len__(self):
        return len(self.params)
# end Defaults

#-----------------------------------------------------------------------
# define_config() - Creates the Config object.
#-----------------------------------------------------------------------
def define_config(args, defs):
    config = Config()

    config.define_string_param(name="build_type",
                               cmake_name="CMAKE_BUILD_TYPE",
                               value=args.build_type,
                               default=defs["build_type"],
                               doc="build configuration")

    config.define_string_param(name="cxx_standard",
                               cmake_name="CMAKE_CXX_STANDARD",
                               value=args.cxx_standard,
                               default=defs["cxx_standard"],
                               doc="c++ standard to use")

    config.define_path_param(name="dep_install",
                             cmake_name="DEPEND_INSTALL_DIR",
                             env_name="DEPEND_INSTALL",
                             value=args.dep_install,
                             default=defs["dep_install"],
                             doc="dependencies install directory")

    config.define_path_param(name="ovs_install",
                             cmake_name="OVS_INSTALL_DIR",
                             env_name="OVS_INSTALL",
                             value=args.ovs_install,
                             default=defs["ovs_install"],
                             doc="ovs install directory")

    config.define_path_param(name="prefix",
                             cmake_name="CMAKE_INSTALL_PREFIX",
                             value=args.prefix,
                             default=defs["prefix"],
                             doc="install path prefix")

    config.define_path_param(name="sde_install",
                             cmake_name="SDE_INSTALL_DIR",
                             env_name="SDE_INSTALL",
                             value=args.sde_install,
                             default=defs["sde_install"],
                             doc="SDE install directory")

    config.define_bool_param(name="set_rpath",
                             cmake_name="SET_RPATH",
                             value=args.set_rpath,
                             default=defs["set_rpath"],
                             doc="whether to set RPATH in binary artifacts")

    config.define_target_param(name="tdi_target",
                               cmake_name="TDI_TARGET",
                               value=args.target,
                               default=defs["tdi_target"],
                               doc="TDI target to build")

    config.define_bool_param(name="with_krnlmon",
                             cmake_name="WITH_KRNLMON",
                             value=args.with_krnlmon,
                             default=defs["with_krnlmon"],
                             doc="whether to enable krnlmon support")

    config.define_bool_param(name="with_ovs",
                             cmake_name="WITH_OVSP4RT",
                             value=args.with_ovs,
                             default=defs["with_ovs"],
                             doc="whether to enable ovs support")

    return config

#-----------------------------------------------------------------------
# load_defaults() - Creates the Defaults object.
#-----------------------------------------------------------------------
def load_defaults(args):
    defaults = Defaults()

    if args.default_cfg not in UNSPECIFIED:
        fn = args.default_cfg
    elif args.default_cfg is not None:
        # Do not load default configuration file if the parameter
        # value is *explicitly unspecified* ('None', '-', '').
        return defaults
    elif os.path.exists(LOCAL_CONFIG_FILE):
        fn = LOCAL_CONFIG_FILE
    elif os.path.exists(USER_CONFIG_FILE):
        fn = USER_CONFIG_FILE
    else:
        return defaults

    logging.info("Loading default configuration from '{}'".format(fn))

    with open(fn, 'r') as cfgfile:
        defaults.load(cfgfile)

    return defaults

#-----------------------------------------------------------------------
# process_args()
#
# Processes command-line parameters after they have been parsed and
# before the Config object is created.
#
# Performs canonicalization and input validation as needed.
#-----------------------------------------------------------------------
def process_args(args):
    args.variant = None
    process_build_type_param(args)
    process_format_param(args)
    process_output_param(args)
    process_target_param(args)
    process_path_params(args)
    return

def process_build_type_param(args):
    global errcount
    if args.build_type in UNSPECIFIED:
        args.build_type = None
        return
    build_type = args.build_type.lower()
    if build_type == 'debug':
        args.build_type = 'Debug'
    elif build_type == 'release':
        args.build_type = 'Release'
    elif build_type == 'reldeb' or build_type == 'relwithdebinfo':
        args.build_type = 'RelWithDebInfo'
    elif build_type == 'minsize' or build_type == 'minsizerel':
        args.build_type = 'MinSizeRel'
    else:
        logging.error("Invalid build type: '%s'", args.build_type)
        errcount += 1
    return

def process_format_param(args):
    global errcount
    if args.format in UNSPECIFIED:
        logging.error("No output format specified")
        errcount += 1
        return
    format = args.format.lower()
    if format in ['cmake', 'env', 'json']:
        args.format = format
    else:
        logging.error("Invalid output format: '%s'", args.format)
        errcount += 1
    return

def process_output_param(args):
    if args.outfile in UNSPECIFIED:
        args.outfile = '/dev/stdout'
    return

def process_target_param(args):
    global errcount
    if args.target in UNSPECIFIED:
        args.target = None
        return
    target = args.target.upper()
    if target in ['DPDK', 'ES2K', 'TOFINO']:
        args.target = target
    else:
        logging.error("Invalid target type: '%s'", args.target)
        errcount += 1
    return

def process_path_params(args):
    args.dep_install = resolve_path("dep_install", args.dep_install)
    args.ovs_install = resolve_path("ovs_install", args.ovs_install)
    args.sde_install = resolve_path("sde_install", args.sde_install)
    args.prefix = resolve_path("prefix", args.prefix, check_path=False)
    return

def resolve_path(name, value, check_path=True):
    global warncount
    if value in UNSPECIFIED:
        return None
    realpath = os.path.abspath(os.path.expanduser(value))
    if check_path:
        if not os.path.exists(realpath):
            logging.warning("{}: '{}' not found".format(name, realpath))
            warncount += 1
        elif not os.path.isdir(realpath):
            logging.warning("{}: '{}' not a directory".format(name, realpath))
            warncount += 1
    return realpath

#-----------------------------------------------------------------------
# create_parser() - Creates the command-line parser.
#-----------------------------------------------------------------------
def create_parser():
    parser = argparse.ArgumentParser(
        prog='p4cpconfig',
        description='Generates build configuration file for a target.')

    parser.add_argument('--format', '-f', type=str,
                        default='cmake',
                        help='output file format (cmake|json|env)')

    parser.add_argument('--load', '-L', dest='default_cfg',
                        help='default configuration file')

    parser.add_argument('--output', '-o', dest='outfile',
                        help='output file path')

    # 'paths' group
    paths = parser.add_argument_group(title='paths')

    paths.add_argument('--dep-install', '-D', type=str,
                       help='stratum dependencies install directory')

    paths.add_argument('--ovs-install', '-O', type=str,
                       help='open vswitch install directory')

    paths.add_argument('--prefix', '-P', type=str,
                       help='p4 control plane install directory')

    paths.add_argument('--sde-install', '-S', type=str,
                       help='target SDE install directory')

    # 'options' group
    options = parser.add_argument_group(title='options')

    options.add_argument('--build-type', type=str,
                         help='type of build (debug|reldeb|release)')

    options.add_argument('--cxx-standard', type=int,
                         help='c++ standard to be used by the compiler (11|14|17)')

    options.add_argument('--set-rpath', type=str2bool,
                         help="set RPATH in binary artifacts")

    options.add_argument('--target', type=str,
                         help='target to build (dpdk|es2k|tofino)')

    options.add_argument('--with-krnlmon', type=str2bool,
                         help='include kernel monitor')

    options.add_argument('--with-ovs', type=str2bool,
                         help='include open vswitch')

    return parser

def str2bool(v):
    """Converts a string to a Boolean value."""
    if isinstance(v, bool):
        return v
    if v.lower() in ('yes', 'on', 'true', 't', 'y', '1'):
        return True
    elif v.lower() in ('no', 'off', 'false', 'f', 'n', '0'):
        return False
    else:
        raise argparse.ArgumentTypeError('Boolean value expected.')

#-----------------------------------------------------------------------
# main - Main program.
#-----------------------------------------------------------------------
if __name__ == '__main__':
    logging.basicConfig(level=logging.INFO)

    parser = create_parser()
    args = parser.parse_args()
    process_args(args)

    if errcount != 0:
        exit(1)

    defaults = load_defaults(args)
    config = define_config(args, defaults)

    with open(args.outfile, 'w') as outfile:
        if args.format == 'cmake':
            config.write_cmake_variables(outfile)
        elif args.format == 'env':
            config.write_env_variables(outfile)
        elif args.format == 'json':
            config.write_json_params(outfile)
    # end with
# end __main__

