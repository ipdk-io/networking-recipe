.. Copyright 2024 Intel Corporation
   SPDX-License-Identifier: Apache-2.0

=====================
P4 Role Configuration
=====================

The Stratum component of P4 Control Plane supports the P4 Role
Configuration feature. The `P4Runtime Specification <https://p4.org/p4-spec/p4runtime/main/P4Runtime-Spec.html#sec-arbitration-role-config>`_
allows multiple P4 clients simultaneous access to the various parts of the P4
pipeline, and defines the client arbitration rules. The partitioning of the
control planes is controlled via the concept of 'roles'. A role defines a
grouping of P4 entities and P4Runtime allows a primary controller for each role.

*P4 Clients* and *controllers* are used interchangably in this context.

Stratum's role config description is protobuf-based and can be found in
`stratum/public/proto/p4_role_config.proto <https://github.com/ipdk-io/stratum-dev/blob/split-arch/stratum/public/proto/p4_role_config.proto>`_.

Each controller defines a role name and the P4 entities that it has either
shared or exclusive write-access to. In addition, the controller configures
whether it can receive input packets, push new pipelines, etc.
Multiple controllers with the same role will be arbitrated via time-based
election ID by the P4 Runtime server. If no roles are defined, the
controller will assume a *default* role, giving it unrestricted full pipeline
access.

Example Configuration
~~~~~~~~~~~~~~~~~~~~~

This example demonstrates two control planes (i.e., two P4 clients)
partitioning the pipeline into non-overlapping sections giving each client
exclusive access to specified P4 tables. Clients chosen for this example are
Linux Networking and IPsec.

* Compile P4 program

Compile the application program according to the instructions in the Compiling
P4 Programs guide to generate P4 artifacts for programming the pipeline.

* Extract table IDs

From the P4 artifact ``p4info.txt``, extract P4 table IDs. A helper script is
provided to assist in extracting all table IDs from the provided input file.

Each P4 client will need its own role configuration, so run the script for
each of them.

.. code-block:: text

   python $P4CP_RECIPE/install/sbin/extract_table_ids_from_p4info.py \
         -i p4info.txt -o /usr/share/stratum/ovs_p4rt_role_config.pb.txt

   python $P4CP_RECIPE/install/sbin/extract_table_ids_from_p4info.py \
         -i p4info.txt -o /usr/share/stratum/ipsec_role_config.pb.txt

* Edit role config files to partition the roles

You will need to compare the table IDs from input file ``p4info.txt`` and the
extracted list in the output role configuration file. For each table ID, define
whether a table gets exclusive access or shared access between controllers.

Following shows table IDs with comments added showing table name for clarity.

.. code-block:: text

   $ cat /usr/share/stratum/ovs_p4rt_role_config.pb.txt

   exclusive_p4_ids: 41667918  // table ID for linux_networking_control.ipv4_lpm_root_lut
   exclusive_p4_ids: 43524029  // table ID fro linux_networking_control.ipv6_lpm_root_lut
   exclusive_p4_ids: 40980035  // table ID for linux_networking_control.vxlan_decap_mod_table
   exclusive_p4_ids: 37624107  // table ID for linux_networking_control.vxlan_decap_and_push_vlan_mod_table
   exclusive_p4_ids: 42605135  // table ID for linux_networking_control.rif_mod_table_start
   ...
   receives_packet_ins: false
   can_push_pipeline: true

.. code-block:: text

   $ cat /usr/share/stratum/ipsec_role_config.pb.txt

   exclusive_p4_ids: 48773578  // table ID for MainControlDecrypt.lem_exception
   exclusive_p4_ids: 38116127  // table ID for MainControlDecrypt.lem_clear
   exclusive_p4_ids: 45068641  // table ID for MainControlDecrypt.ipsec_rx_sa_classification_table
   exclusive_p4_ids: 41233864  // table ID for crypto_tunnel_control.ipsec_rx_post_decrypt_table
   ...
   receives_packet_ins: false
   can_push_pipeline: false


* Connect P4 client to infrap4d

Once the configuration is complete, the P4 clients will push a role name and
the role configuration file to infrap4d with a ``MasterArbitrationUpdate``
message after the gRPC connection is initiated.

Your P4 Client
~~~~~~~~~~~~~~

If using your own P4 client, you can set role name and pack ``Any`` protobuf
message with the role configuration. Details of how to perform this are
available in this document:
https://github.com/ipdk-io/stratum-dev/blob/split-arch/stratum/public/proto/p4_role_config.md.
