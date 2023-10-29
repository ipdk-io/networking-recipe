.. Copyright 2023 Intel Corporation
   SPDX-License-Identifier: Apache 2.0

=============
p4cpconfig.py
=============

Generates a file that defines one or more build parameters, which may
be specified to ``cmake`` when configuring the build:

.. code-block:: text

  cmake -B build -C es2k-setup.cmake [parameters]

Parameters specified on the cmake command line will add to, or override,
the parameters in the configuration file.

If ``--format=env`` is specified, p4cpconfig will generate a file
that sets environment variables.

If ``--format=json`` is specified, the output will be a JSON file that
that can be used to specify default settings on a subsequent run.

.. note::

  The p4cpconfig utility is experimental and still under development.
  Parameters and functionality may change.

  This version does not support cross-compilation for ACC.

Syntax
======

.. code-block:: bash

  ./scripts/p4cpconfig.py [parameters]

Conventions
===========

* Long parameter names may be abbreviated to their shortest unique form.
  For example, ``--sde-install`` may be shortened to ``--sde``.

* Boolean parameter values may be:

  * True: ``on``, ``yes``, ``true``, ``1``
  * False: ``off``, ``no``, ``false``, ``0``

* p4cpconfig does not supply intrinsic default values for any of the
  build parameters. Parameters that do not have values are not written
  to the output file.

* A build parameter may be *explicitly unspecified* by assigning it
  a value of "``None``", "``-``", or the empty string ("").

* File paths will be converted to their absolute form.

Parameters
==========

General
-------

``--help``, ``-h``
  Displays usage information and exits.

``--format=FORMAT``, ``-f FORMAT``
  Output file format.
  Valid formats are ``cmake``, ``env``, and ``json``.

  Defaults to ``cmake`` if no value is specified.

``--load=FILEPATH``, ``-L FILEPATH``
  JSON file specifying default configuration settings.

  If no value is specified, p4cpconfig will check first for
  a file named ``.p4cpconfig`` in the current directory, then for
  a file of the same name in the user's home directory.

  May be suppressed by using an *explicitly unspecified* value
  (e.g., ``-L-`` or ``--load=None``).

``--output=FILEPATH``, ``-o FILEPATH``
  Output file path.

  Defaults to standard output if no value is specified.

Paths
-----

``--dep-install=DIRPATH``, ``-D DIRPATH``
  Directory in which the Stratum dependencies for the runtime system
  are installed.

  Sets the ``DEPEND_INSTALL_DIR`` cmake variable and the ``DEPEND_INSTALL``
  environment variable.

``--ovs-install=DIRPATH``, ``-O DIRPATH``
  Directory in which Open vSwitch is installed.

  Sets the ``OVS_INSTALL_DIR`` cmake variable and the ``OVS_INSTALL``
  environment variable.

``--prefix=DIRPATH``, ``-P DIRPATH``
  Directory in which P4 Control Plane will be installed.
  It will be created if it does not exist.

  Sets the ``CMAKE_INSTALL_PREFIX`` cmake variable and the
  ``P4CP_INSTALL`` environment variable.

``--sde-install=DIRPATH``, ``-S DIRPATH``
  Directory in which the SDE for the P4 target is installed.

  Sets the ``SDE_INSTALL_DIR`` cmake variable and the ``SDE_INSTALL``
  environment variable.

Options
-------

``--build-type=TYPE``
  Build configuration to use (debug, release, etc.).

  Standard cmake build types are ``Release``, ``Debug``, ``RelWithDebInfo``,
  and ``MinSizeRel``.
  p4cpconfig also accepts the ``reldeb`` and ``minsize`` abbreviations used
  in the helper scripts.

  The parameter value is case-insensitive.
  p4cpconfig will convert the value to one of the standard build types.

  Sets the ``CMAKE_BUILD_TYPE`` cmake variable.

``--cxx-standard=STD``
  C++ standard to be used by the compiler (11, 14, 17).

  Sets the ``CMAKE_CXX_STANDARD`` cmake variable.

``--set-rpath=BOOL``
  Whether to set RPATH in libraries and executables.

  Sets the ``SET_RPATH`` cmake variable.

``--target=TARGET``
  The target for which P4 Control Plane will be built.

  Valid target names are ``dpdk``, ``es2k``, and ``tofino``.
  The target name is case-insensitive.
  p4cpconfig will convert it to upper case.

  Sets the ``TDI_TARGET`` cmake variable.

``--with-krnlmon=BOOL``
  Whether to include the Kernel Monitor.

  Sets the ``WITH_KRNLMON`` cmake variable.

``--with-ovs=BOOL``
  Whether to include support for Open vSwitch.

  Sets the ``WITH_OVSP4RT`` cmake variable.

Examples
========

Default configuration
---------------------

By default, ``make-all.sh`` creates a separate install tree for OVS,
the P4 Control Plane ``install`` tree is under the main directory,
and the target type defaults to ``dpdk``.
You can duplicate this behavior by creating a default configuration file:

.. code-block:: bash

  ./scripts/p4cpconfig.py -L- -f json --target=dpdk \
      -O ovs/install -P install -o .p4cpconfig

The file it generates will be something like this:

.. code-block:: json

  {
      "ovs_install": "/home/rocky/work/latest/ovs/install",
      "prefix": "/home/rocky/work/latest/install",
      "tdi_target": "DPDK"
  }

Notes:

* ``-L-`` keeps p4cpconfig from loading the current defaults.
* ``-f json`` tells it to create a json file.
