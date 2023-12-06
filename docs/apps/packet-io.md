# Packet-IO
The Packet I/O feature facilitates the exchange of packets between control plane applications and P4 dataplanes. This functionality enables control plane applications to receive packets asynchronously from the dataplane, while also allowing the injection of packets into the dataplane.

The Packet-IO feature is currently supported on the Intel&reg; IPU E2100 target.

## Feature overview

Packet-IO consists of two essential components: Packet-In and Packet-Out.
- **Packet-In**: Refers to a data plane packet sent by the P4Runtime server to the control plane for further analysis. This is specified as `packetIn` message response in the [p4runtime specification](https://github.com/ipdk-io/p4runtime-dev/blob/mirroring/proto/p4/v1/p4runtime.proto).
- **Packet-Out**: Defined as a data packet originated by the control plane and injected into the data plane through the P4Runtime server. This is specified as `packetOut` in p4runtime specification.

During the set pipeline sequence, the Packet-IO configuration is extracted from the pipeline configuration. This configuration is then utilized to register Rx and Tx callbacks with the device driver. When a packet is received, the device driver invokes the RX callback, and when a packet is transmitted, the Tx callback is triggered.

### Rx Path
The P4 device driver invokes the registered RX callback upon receiving a packet, passing the packet details to the Stratum layer of `infrap4d`. The Stratum layer parses the received packet and translates it into a PacketIn message as defined in p4runtime.proto. The P4Runtime server sends the PacketIn message to the connected client.

### Tx Path
A P4runtime client/controller can send packets to the PacketIO port as a PacketOut message defined in p4runtime.proto. The P4CP Stratum layer translates the PacketOut message to TDI structures and sends them to the driver.

## Enabling Packet-IO

To enable the Packet-IO feature, the `pktio-args` configuration must be added to the following files:
- The configuration file used by the `infrap4d` process.
- The configuration file used by `tdi_pipeline_builder`.

The Packet-IO configuration is per device and should be added under the `p4_devices` section.

#### Packet-IO configuration
	"pktio-args": {
		"ports" 		// list of ports to receive and transmit packetIO packets
		"nb_rxqs" 		// number of rx queues per port
		"nb_txqs" 		// number of tx queues per port
	},

#### Example
```json
	"pktio-args": {
		"ports": \[2,3\],
		"nb_rxqs" : 4,
		"nb_txqs" : 4
	},
```

Follow the sequence of steps listed below to enable Packet-IO functionality.

### Configure and run infrap4d
The `infrap4d` process provides the gRPC server-side support for P4Runtime packetIn and packetOut messages.

To start `infrap4d` process with Packet-IO, the /usr/share/stratum/es2k/es2k_skip_p4.conf file must include Packet IO configuration.
Instructions to run infrap4d can be found at [running infrap4d](/docs/guides/es2k/running-infrap4d.md)

Ensure you update this configuration before starting `infrap4d`.

### Configure and set the pipeline
The Packet IO configuration mentioned above should also be present in the configuration file provided with the `p4c_conf_file` option for building the pipeline.
Instructions to build and set pipeline can be found at [set pipeline](/docs/guides/setup/es2k-setup-guide.md)

## Reference client
The reference client `p4rt-ctl` application can be used to excercise Packet-IO feature.
Instructions to start p4rt-ctl client in Packet-IO mode can be found under `Start Packet-IO` section in [p4rt-ctl guide](/docs/clients/p4rt-ctl.rst)

In Packet-IO mode, the following steps take place:
- The p4rt-ctl client initializes a pktioTap0 port designed for testing purposes. This port facilitates the sending and receiving of packets.
- The packets sent to pktioTap0 port are forwarded to P4Runtime server as `PacketOut` messages.
- The p4rt-ctl client establishes a connection with the P4Runtime server and awaits incoming RX packets from the server. Subsequently, the received packets are forwarded to the pktioTap0 port.

