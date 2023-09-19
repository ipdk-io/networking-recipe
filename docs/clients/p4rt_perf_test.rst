..
      Copyright 2021-2023 Intel Corporation
      SPDX-License-Identifier: Apache-2.0

==========================
p4rt_perf_test Usage Guide
==========================

This document describes how to use p4rt_perf_test application.

Overview
-------------
p4rt_perf_test is a test application designed to assess the performance
of a P4Runtime server across various P4 profiles. The application's primary
focus is on measuring the time required to program a specified number of
entries.

prerequisites
-------------

Prior to running p4rt_perf_test, ensure that the infrap4d application has been initiated,
and one of the supported pipelines has been configured.

p4rt_perf_test
--------------

``p4rt_perf_test`` is an executable generated when P4 Control Plane is built. It
connects to the P4Runtime server in ``infrap4d`` via gRPC for
enabling P4Runtime capabilities.

Syntax
~~~~~~

.. code-block:: text

   Usage: p4rt [OPTIONS] COMMAND [ARG...]
   $IPDK_RECIPE/install/bin/p4rt_perf_test -t <value> -o <value> -n <value> -p <value>

Arguments
~~~~~~~~~

* ``-t number of threads``
      Optional argument that specifies the number of threads that can connect to the server.
      Default is one thread with the maximum limit being 8.

* ``-o operation``
      Mandatory arguments that specifies the operation to be performed.
      ADD(1) and DEL(2) are the currently supported operations.

* ``-n number of threads``
      Optional argument that specifies the number of entries to be programmed.
      Default is 1000000 entires with the maximum limit being the maximum size of uint64.

* ``-p p4 program used for the test``
      Optional argument that specifies the p4 program used for the test.
      Default is simple_l2_demo.
      Supported profiles: simple_l2_demo.
      Number to profile mapping: 1 => simple_l2_demo.


Example
~~~~~~~~

.. code-block:: bash

   p4rt_perf_test -t 1 -o 1 -n 4000000 -p 1

Known Issues
------------
At present, the P4Runtime server exclusively supports 'write' operations through the master client,
rendering it incompatible with the use of multi-threaded clients.

