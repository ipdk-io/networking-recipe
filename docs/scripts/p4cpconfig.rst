.. Copyright 2023 Intel Corporation
   SPDX-License-Identifier: Apache 2.0

=============
p4cpconfig.py
=============

Generates a file that defines one or more build parameters, which may
be specified to ``cmake`` when configuring the build.

Additional parameters may specified on the cmake command line to override
or add to the parameters in the configuration file.

.. note::

  The p4cpconfig utility is experimental and still under development.
  Parameters and functionality may change.

  This version does not support cross-compilation for ACC.

Output formats
==============

The output file format may be specified using the ``--format`` (``-f``)
parameter. The default value is ``cmake``.

+-----------+--------------------------------------------------------+
| ``cmake`` | Output will be a cmake configuration file.             |
+-----------+--------------------------------------------------------+
| ``env``   | Output will be a file that sets environment variables. |
+-----------+--------------------------------------------------------+
| ``json``  | Output will be a JSON file that that can be used to    |
|           | specify default settings on a subsequent run.          |
+-----------+--------------------------------------------------------+

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

Default configuration file
--------------------------

By default, ``make-all.sh`` creates a separate install tree for OVS,
the P4 Control Plane ``install`` tree is under the main directory,
and the target type defaults to ``dpdk``.

You can duplicate this behavior by creating a default configuration file:

.. code-block:: bash

  ./scripts/p4cpconfig.py -L- -f json --target=dpdk \
      -O ovs/install -P install -o .p4cpconfig

Where:

- ``-L-`` keeps p4cpconfig from loading the current defaults
- ``-f json`` tells it to create a json file

The file it generates will be something like this:

.. code-block:: json

  {
      "ovs_install": "/home/rocky/work/latest/ovs/install",
      "prefix": "/home/rocky/work/latest/install",
      "tdi_target": "DPDK"
  }

JSON configuration file
-----------------------

In this example, we're going to define a build configuration and save it
to a json file.

.. code-block:: bash

  ./scripts/p4cpconfig.py --target=es2k --build-type=release \
      -D /opt/deps/ -S ~/mev-p4-sde/install/ -O ovs/install/ \
      --set-rpath=on --format=json \
      -o es2k-config.json

Which produces the following JSON file:

.. code-block:: json

  {
    "build_type": "Release",
    "dep_install": "/opt/deps",
    "ovs_install": "/home/rocky/work/latest/ovs/install",
    "prefix": "/home/rocky/work/latest/install",
    "sde_install": "/home/rocky/mev-p4-sde/install",
    "set_rpath": true,
    "tdi_target": "ES2K"
  }

The definition of ``prefix`` came from the ``.p4cpconfig`` file.

CMake configuration file
------------------------

In this example, we're going to create a cmake configuration file from a
saved JSON configuration:

.. code-block:: bash

  ./scripts/p4cpconfig.py -L es2k-config.json -o release-config.cmake

Where:

- The output format defaults to ``cmake``.

This produces the following cmake file:

.. code-block:: cmake

  set(CMAKE_BUILD_TYPE "Release" CACHE STRING "config: build configuration")
  set(CMAKE_INSTALL_PREFIX "/home/rocky/work/latest/install" CACHE PATH "config: install path prefix")
  set(DEPEND_INSTALL_DIR "/opt/deps" CACHE PATH "config: dependencies install directory")
  set(OVS_INSTALL_DIR "/home/rocky/work/latest/ovs/install" CACHE PATH "config: ovs install directory")
  set(SDE_INSTALL_DIR "/home/rocky/mev-p4-sde/install" CACHE PATH "config: SDE install directory")
  set(SET_RPATH True CACHE BOOL "config: whether to set RPATH in binary artifacts")
  set(TDI_TARGET "ES2K" CACHE STRING "config: TDI target to build")

Derivative configuration file
-----------------------------

In the above examples, we could have created the the CMake file directly,
instead of generating a JSON file first.

The advantage of having a JSON configuration is that you can use it to
create variants:

.. code-block:: bash

  ./scripts/p4cpconfig -L es2k-config.json --build-type=Debug -o debug-config.cmake

The output is:

.. code-block:: cmake

  set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "config: build configuration")
  set(CMAKE_INSTALL_PREFIX "/home/rocky/work/latest/install" CACHE PATH "config: install path prefix")
  set(DEPEND_INSTALL_DIR "/opt/deps" CACHE PATH "config: dependencies install directory")
  set(OVS_INSTALL_DIR "/home/rocky/work/latest/ovs/install" CACHE PATH "config: ovs install directory")
  set(SDE_INSTALL_DIR "/home/rocky/mev-p4-sde/install" CACHE PATH "config: SDE install directory")
  set(SET_RPATH True CACHE BOOL "config: whether to set RPATH in binary artifacts")
  set(TDI_TARGET "ES2K" CACHE STRING "config: TDI target to build")

The new file the configuration parameters as the previous example, but the
build type has been changed from ``Release`` to ``Debug``.

Using a cmake configuration
---------------------------

The cmake configuration file is used when you configure the build:

.. code-block:: bash

  cmake -B build -C debug-config.cmake

You are now free to specify any build-time parameters you choose. For
instance, you could use the configuration to run the krnlmon unit tests:

.. code-block:: bash

  cmake --build build -j4 --target krnlmon-test

With the following results:

.. code-block:: text

  Scanning dependencies of target switchlink_route_test
  Scanning dependencies of target switchlink_neighbor_test
  Scanning dependencies of target switchlink_link_test
  Scanning dependencies of target switchlink_address_test
    .
    .
  Test project /home/rocky/work/latest/build
      Start 1: switchlink_link_test
  1/4 Test #1: switchlink_link_test .............   Passed    0.00 sec
      Start 2: switchlink_address_test
  2/4 Test #2: switchlink_address_test ..........   Passed    0.00 sec
      Start 3: switchlink_neighbor_test
  3/4 Test #3: switchlink_neighbor_test .........   Passed    0.00 sec
      Start 4: switchlink_route_test
  4/4 Test #4: switchlink_route_test ............   Passed    0.00 sec