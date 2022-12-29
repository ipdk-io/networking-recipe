gnmi-ctl executable
-------------------

gnmi-ctl is a gRPC-based C++ network management interface client to handle port configurations and program fixed functions in the P4 pipeline. This client connects to the Config monitoring service running as part of Stratum and integrated as part of infrap4d. This utility is built as part of networking recipe and is available under ``install/bin/``.

.. note::
Use ``sudo`` when issuing gnmi-ctl command when you are non-root user

gnmi client connects to gRPC port 9339 opened by the server and sends a protobuf based message. Refer to gnmi.proto and gnmi_ext.proto for more details on type of messages and services that are available for a gNMI client. These documents can be found in ``stratum/proto`` directory.

Each CLI format should be in line with the YANG parser available in server which is a tree based hierarchy. Tree has ``interfaces`` as root-node, followed by the device type. Each device can have multiple ports, where port-name acts as a KEY. SET command can be executed to configure the ports and GET command to retrieve the information about previously configured ports.

1) Set attributes for a vhost port::

    $ gnmi-ctl set PARAMS
    $ Example:
    gnmi-ctl set "device:virtual-device,name:net_vhost0,device-type:VIRTIO_NET"
    gnmi-ctl set "device:virtual-device,name:net_vhost0,port-type:LINK"
    gnmi-ctl set "device:virtual-device,name:net_vhost0,host-name:host1,
                  device-type:VIRTIO_NET,queues:1,
                  socket-path:/tmp/vhost-user-0,port-type:LINK"
    gnmi-ctl set "device:virtual-device,name:net_vhost0,host-name:host1,
                  device-type:VIRTIO_NET,queues:1,
                  pipeline-name:pipe,mempool-name:MEMPOOL0,mtu:2000,
                  socket-path:/tmp/vhost-user-0,packet-dir:host,port-type:LINK"
 ..

* Parameter description:
    * ``PARAMS``: These params are key:value pairs. Here virtual-device is a sub-node which can hold multiple ports like net_vhost0, net_vhost1,... and each port accepts multiple configuration parameters. These configuration parameters are again a key:value pair, which can be either passed in single CLI command or multiple CLI commands.

    * ``name``: Supported values are defined in chassis config file ``dpdk_port_config.pb.txt``.
    * ``host-name``: can be any string.
    * ``queues``: number of queues to be configured by target backend.
    * ``socket-path``: socket path for vhost port.
    * ``port-type``: Defines the port type and supported value for vhost port is ``LINK``.
    * ``device-type``: Defines the device type and supported value for vhost port is ``VIRTIO_NET``.
    * ``pipeline-name``: Defines the pipeline name. Optional parameter. Default value is ``pipe``.
    * ``mempool-name``: Defines mempool name. Optional parameter. Default value is ``MEMPOOL0``.
    * ``mtu``: Defines maximum transmission unit. Optional parameter. Default value is ``1500``.
    * ``packet-dir``: Optional parameter. Value may be ``host`` or ``network`` (case insensitive). The value ```host` indicates that traffic via this port will be within the host. The value ``network`` indicates this port will be able to send/receive traffic to/from network. It's a non-mandatory parameter and default value is ``host``.

2) Set attributes for vhost port with control port::

    $ gnmi-ctl set PARAMS
    $ Example:
    gnmi-ctl set "device:virtual-device,name:net_vhost0,host-name:host1,
                  device-type:VIRTIO_NET,queues:1,
                  pipeline-name:pipe,mempool-name:MEMPOOL0,control-port:TAP2,
                  socket-path:/tmp/vhost-user-0,packet-dir:host,port-type:LINK"
  ..
* Parameter description:
    * ``PARAMS``: These params are key:value pairs. Here virtual-device is a sub-node which can hold multiple ports like net_vhost0, net_vhost1,... and each port accepts multiple configuration parameters. These configuration parameters are again a key:value pair, either can be passed in single CLI command or multiple CLI commands.
    * ``name``: Supported values are defined in chassis config file ``dpdk_port_config.pb.txt``.
    * ``host-name``: can be any string.
    * ``queues``: number of queues to be configured by target backend.
    * ``socket-path``: socket path for vhost port.
    * ``port-type``: Defines the port type and supported value for vhost port is ``LINK``.
    * ``device-type``: Defines the device type and supported value for vhost port is ``VIRTIO_NET``.
    * ``pipeline-name``: Defines the pipeline name. Optional parameter. Default value is ``pipe``.
    * ``mempool-name``: Defines mempool name. Optional parameter. Default value is ``MEMPOOL0``.
    * ``mtu``: Defines maximum transmission unit. Optional parameter. Default value is ``1500``.
    * ``control-port``: Defines the name for control TAP port.
    * ``packet-dir``: Optional parameter. Value may be ``host`` or ``network`` (case insensitive). The value ```host`` indicates that traffic via this port will be within the host. The value ``network`` indicates this port will be able to send/receive traffic to/from network. It's a non-mandatory parameter and default value is ``host``.

3) Get attributes for a port::

    $ gnmi-ctl get PARAMS
    $ Example:
    gnmi-ctl get "device:virtual-device,name:net_vhost0,device-type"
    gnmi-ctl get "device:virtual-device,name:net_vhost0,port-type"

  ..
* Parameter description:
    ``PARAMS``: These params are key:value pairs. Here virtual-device is a sub-node which can hold multiple ports like net_vhost0, net_vhost1, etc. Pass the key name whose value needs to be fetched. Each get can take ONLY one key, and fetches value for the previously configured parameter.

4) VIRTIO-NET Device Hot plug for DPDK Target::
    This feature will allow the user to hotplug the vhost-user ports to the running VM.
    To hotplug the vhost-user port to qemu based VM, add monitor option when instantiating qemu based VM and specify the telnet port and IP for qemu monitor socket.

    Example of qemu command::

      qemu-system-x86_64 -enable-kvm -smp 4 -m 1024M \
      -boot c -cpu host -enable-kvm -nographic \
      -L /root/pc-bios -name VM1_TAP_DEV \
      -hda /root/VM/vm1.qcow2 \
      -object memory-backend-file,id=mem,size=1024M,mem-path=/dev/hugepages,share=on \
      -mem-prealloc \
      -numa node,memdev=mem \
      -monitor telnet::6555,server,nowait \
      -serial telnet::6551,server &

    Boot up the VM and login to console using telnet port (in the above example port 6551). This VM will have 1 default port. To hotplug the vhost-user port, issue the following gnmi-ctl command::

      $ gnmi-ctl set PARAMS
      $ Command to hotplug add the port. Example:
            gnmi-ctl set "device:virtual-device,name:net_vhost0,qemu-hotplug-mode:add,
                          qemu-socket-ip:127.0.0.1,qemu-socket-port:6555,
                          qemu-vm-mac-address:00:e8:ca:11:aa:01,qemu-vm-netdev-id:netdev0,
                          qemu-vm-chardev-id:char1,native-socket-path:/tmp/vhost-user-0,
                          qemu-vm-device-id:dev0"
      $ Command to hotplug delete the port. Example:
            gnmi-ctl set "device:virtual-device,name:net_vhost0,qemu-hotplug-mode:del"
..
* Parameter description:
    * ``PARAMS``: These params are key:value pairs. Here virtual-device is a sub-node which can hold multiple ports like net_vhost0, net_vhost1, etc. and each port accepts multiple configuration parameters. These configuration parameters are again a key:value pair, either can be passed in single CLI command or multiple CLI commands.
    * ``name``: Supported values are defined in chassis config file ``dpdk_port_config.pb.txt``.
    * ``qemu-hotplug-mode``: Defines the hotplug mode. Supported values are (add/ADD, del/DEL).
    * ``qemu-socket-ip``: Defines IP address for qemu monitor socket.
    * ``qemu-socket-port``: Defines port number for qemu monitor socket.
    * ``qemu-vm-mac-address``: Defines MAC address for port hotplugged to qemu VM.
    * ``qemu-vm-netdev-id``: Defines netdev ID for port hotplugged to qemu VM.
    * ``qemu-vm-chardev-id``: Defines chardev ID for port hotplugged to qemu VM.
    * ``native-socket-path``: Defines the native path for vhost-user socket on host.
    * ``qemu-vm-device-id``: Defines device ID for port hotplugged to qemu VM.

.. note::

   Port can be hotplug added once and hotplug deleted once. Re-adding or
   deleting the port is not supported in older qemu versions due to qemu bug
   reporting false duplicate IDs. This bug is fixed in qemu version 6.1.0 and
   re-adding and re-deleting the hotplug port is supported from qemu version
   6.1.0

5) Set atrributes for link ports::

    $ gnmi-ctl set PARAMS
    $ Example:
    gnmi-ctl set "device:physical-device,name:PORT1,pci-bdf:0000:00:05.0,
                  port-type:link"
    gnmi-ctl set "device:physical-device,name:PORT0,pipeline-name:pipe,
                  mempool-name:MEMPOOL0,mtu:1000,pci-bdf:0000:00:04.0,
                  packet-dir:network,port-type:link"
  ..
* Parameter description:
    * ``PARAMS``: These params are key:value pairs. Here physical-device is a sub-node which can hold multiple ports like PORT0, PORT1, etc. and each port accepts multiple configuration parameters. These configuration parameters are again a key:value pair, either can be passed in single CLI command or multiple CLI commands.
    * ``name``: Supported values are defined in chassis config file ``dpdk_port_config.pb.txt``.
    * ``pci-bdf``: Define PCI board device function value.
    * ``port-type``: Defines the port type and supported value for vhost port is ``LINK``.
    * ``pipeline-name``: Defines the pipeline name. Optional parameter. Default value is ``pipe``.
    * ``mempool-name``: Defines the mempool name. Optional parameter. Default value is ``MEMPOOL0``.
    * ``mtu``: Defines maximum transmission unit. Optional parameter. Default value is ``1500``.
    * ``packet-dir``: Optional parameter. Value may be ``host`` or ``network`` (case insensitive). The value ``host`` indicates that traffic via this port will be within the host. The value ``network`` indicates this port will be able to send/receive traffic to/from network. If not specifically configured by the user, default value is ``host``

6) Set attributes for link ports and a control port::

    $ gnmi-ctl set PARAMS
    $ Example:
    gnmi-ctl set "device:physical-device,name:PORT2,pipeline-name:pipe,
                  mempool-name:MEMPOOL0,control-port:TAP1,mtu:1000,
                  pci-bdf:0000:00:06.0,packet-dir:network,port-type:link"
  ..
* Parameter description:
    * ``PARAMS``: These params are key:value pairs. Here physical-device is a  sub-node which can hold multiple ports like PORT0, PORT1, etc. and each port accepts multiple configuration parameters. These configuration parameters are again a key:value pair, either can be passed in single CLI command or multiple CLI commands.
    * ``name``: Supported values are defined in chassis config file ``dpdk_port_config.pb.txt``.
    * ``pci-bdf``: Define PCI board device function value.
    * ``port-type``: Defines the port type and supported value for vhost port is ``LINK``.
    * ``pipeline-name``: Defines the pipeline name. Optional parameter. Default value is ``pipe``.
    * ``mempool-name``: Defines mempool name. Optional parameter. Default value is ``MEMPOOL0``.
    * ``mtu``: Defines maximum transmission unit. Optional parameter. Default value is ``1500``.
    * ``control-port``: Defines the name for control TAP port.
    * ``packet-dir``: Optional parameter. Value may be ``host`` or ``network`` (case insensitive). The value ``host`` indicates that traffic via this port will be within the host. The value ``network`` indicates this port will be able to send/receive traffic to/from network. If not specifically configured by the user, default value is ``host``.

7) Set attributes for TAP ports::

    $ gnmi-ctl set PARAMS
    $ Example:
    gnmi-ctl set "device:virtual-device,name:TAP1,mtu:1500,port-type:TAP"
    gnmi-ctl set "device:virtual-device,name:TAP0,pipeline-name:pipe,
                  packet-dir:host,mempool-name:MEMPOOL0,mtu:1500,port-type:TAP"
  ..
* Parameter description:
    * ``PARAMS``: These params are key:value pairs. Here virtual-device is a sub-node which can hold multiple ports like TAP0, TAP1, etc. and each port accepts multiple configuration parameters. These configuration parameters are again a key:value pair, either can be passed in single CLI command or multiple CLI commands.
    * ``name``: Supported values are defined in chassis config file ``dpdk_port_config.pb.txt``.
    * ``port-type``: Defines the port type and supported value for TAP port is ``TAP``.
    * ``mtu``: Defines maximum transmission unit. Optional parameter. Default value is ``1500``.
    * ``pipeline-name``: Defines the pipeline name. Optional parameter. Default value is ``pipe``.
    * ``mempool-name``: Defines mempool name. Optional parameter. Default value is ``MEMPOOL0``.
    * ``packet-dir``: Optional parameter. Value may be ``host`` or ``network`` (case insensitive). The value  ``host`` indicates that traffic via this port will be within the host. The value ``network`` indicates this port will be able to send/receive traffic to/from network. If not specifically configured by the user, default value is ``host``.

8) Set attributes for TAP port with control port::

    $ gnmi-ctl set PARAMS
    $ Example:
    gnmi-ctl set "device:virtual-device,name:TAP2,mtu:1000,
                  pipeline-name:pipe,mempool-name:MEMPOOL0,control-port:TAP31,
                  packet-dir:host,port-type:TAP"
  ..
* Parameter description:
    * ``PARAMS``: These params are key:value pairs. Here virtual-device is a sub-node which can hold multiple ports like TAP0, TAP1,etc. and each port accepts multiple configuration parameters. These configuration parameters are again a key:value pair, either can be passed in single CLI command or multiple CLI commands.
    * ``name``: Supported values are defined in chassis config file ``dpdk_port_config.pb.txt``.
    * ``port-type``: Defines the port type and supported value for TAP port is ``TAP``.
    * ``pipeline-name``: Defines the pipeline name. Optional parameter. Default value is ``pipe``.
    * ``mempool-name``: Defines mempool name. Optional parameter. Default value is ``MEMPOOL0``.
    * ``mtu``: Defines maximum transmission unit. Optional parameter. Default value is ``1500``.
    * ``control-port``: Defines the name for control TAP port.
    * ``packet-dir``: Optional parameter. Value may be ``host`` or ``network`` (case insensitive). The value ``host`` indicates that traffic via this port will be within the host. The value ``network`` indicates this port will be able to send/receive traffic to/from network. If not specifically configured by the user, default value is ``host``.

9) Get attributes for Pipelines Configuration::

    $ gnmi-ctl get PARAMS
    $ Example:
    gnmi-ctl get "device:virtual-device,name:net_vhost0,tdi-portin-id"
    gnmi-ctl get "device:virtual-device,name:net_vhost0,tdi-portout-id"
  ..
* Parameter description:
    * ``PARAMS``: These params are key:value pairs. Here virtual-device is a sub-node which can hold multiple ports like net_vhost0, net_vhost1, etc. Pass the key name whose value needs to be fetched. Each get can take ONLY one key, and fetches value for the previously configured parameter.
    * ``tdi-portin-id``: Port ID for pipeline in input Direction.
    * ``tdi-portin-id``: Port ID for pipeline in output Direction.

10) Get port statistics for VHOST, Physical link and non-control TAP ports::

    $ gnmi-ctl get PARAMS
    $ Example for Physical link ports:
    $ gnmi-ctl get "device:physical-device,name:PORT0,counters" | grep "name\|uint_val" | grep -v "interface\|key\|config\|counters"
    $ Example for vhost ports:
    $ gnmi-ctl get "device:virtual-device,name:net_vhost0,counters" | grep "name\|uint_val" | grep -v "interface\|key\|config\|counters"
    $ Example for non-control TAP ports:
    $ gnmi-ctl get "device:virtual-device,name:TAP0,counters" | grep "name\|uint_val" | grep -v "interface\|key\|config\|counters"
  ..
* Parameter description:
    * ``PARAMS``: These params are key:value pairs. Here physical-device or virtual-device is a sub-node which holds multiple ports,... to Pass he key name for whose value need to be fetched. Each get can take ONLY one key, and fetches value for that previously configured KEY.

  .. note::
    Port stats can be retrieved for the ports that are created through GNMI CLI.
    These ports can be of type LINK/VHOST/TAP type. PORT0, net_vhost0, and TAP0
    corresponds to the name used when creating the ports using GNMI CLI.
    gnmi-ctl by default outputs the data in yang model, so the output is formatted
    using the grep command to display the port statistics.

    Refer to section-11,``Get port statistics for control TAP ports`` to retrieve
    port statistics for control TAP ports.

11) Get port statistics for control TAP ports::

    $ Example:
    $ ovs-ofctl dump-ports <BRIDGE>

  .. note::
    ``gnmi-ctl get`` counters command is not applicable for the TAP ports that are
    added as control ports. But these control ports when added to OVS bridge
    through the "ovs-vsctl add-port <BRIDGE> <TAP-PORT>" command, stats can be
    read through the "ovs-ofctl dump-ports <BRIDGE>" command.

Limitations/Note
----------------

    1) All the optional parameters(like mempool name, pipeline name, etc)
    should be provided before the mandatory parameters. The CLI considers
    the parameters only till the last mandatory parameter. After the
    last mandatory parameter, all the optional parameters are ignored.

    2) DPDK target doesn't support packet categorization for the purpose of
    statistics. Hence all packets are reported under the same category as
    ``unicast packets/bytes``, and the rest of the other fields are displayed
    as zero.

    3) ``gnmi-ctl get`` counters command doesn't work for the TAP ports that
    are added as control ports. For these control ports, stats can be observed
    through the standard ovs-ofctl dump-ports command.

    4) ``gnmi-ctl get`` command shows target datapath index as 0 for all control
    TAP ports.

    5) Number of ports configured should be power 2. No port configuration is
    allowed once PIPELINE is enabled. MODIFY and DELETE operations on ports are
    not supported once the port is added to DPDK target backend.

    6) Runtime validation of ``value`` for each key in ``gnmi-cli`` is not supported.
