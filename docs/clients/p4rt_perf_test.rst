.. Copyright 2023 Intel Corporation
   SPDX-License-Identifier: Apache-2.0

==============
p4rt_perf_test
==============

``p4rt_perf_test`` is a test application designed to assess the performance
of a P4Runtime server across various P4 profiles.
Its primary focus is to measure the time required to program a specified
number of entries.

Prerequisites
=============

Before running p4rt_perf_test, ensure that ``infrap4d`` has been started
and a supported pipeline has been configured.

Syntax
======

.. code-block:: text

   p4rt_perf_test -o OPER [-t THREADS] [-n ENTRIES] [-p PROFILE]

Parameters
==========

``-n ENTRIES``
  Number of entries to be programmed.
  Default is 1000000 (one million) entries, with a maximum value of 2^64-1.

``-o OPER``
  Required.
  Number specifying the operation to be performed.

  +-------+-----------+
  | Value | Operation |
  +=======+===========+
  | 1     | ADD       |
  +-------+-----------+
  | 2     | DEL       |
  +-------+-----------+

``-p PROFILE``
  Number specifying the p4 program used for the test.
  Default is simple_l2_demo(1).
  This is the only profile (program) currently supported.

``-t THREADS``
  Number of threads to connect to the server.
  Default is 1 thread, with a maximum value of 8.

Example
=======

.. code-block:: bash

   p4rt_perf_test -t 1 -o 1 -n 4000000 -p 1

Known Issues
============

At present, the P4Runtime server exclusively supports 'write' operations
through the master client, rendering it incompatible with multi-threaded
clients.
