# Security Guide for P4 Control Plane

- [1. Overview](#1-overview)
- [2. TLS certificates](#2-tls-certificates)
## Overview

This document provides information about secure and insecure
modes for networking recipe and certificate management.

### TLS certificates

The gRPC ports are secured using TLS certificates. A script and reference
configuration files are available to assist in generating certificates and
keys using OpenSSL. You may use other tools if you wish.

The reference file uses a simple PKI where a self-signed key and certificate.
The root level Certificate Authority (CA) is used to generate server-side
key and cert files, and client-side key and cert files. This results in a
1-depth level certificate chain, which will suffice for validation and
confirmation but may not provide sufficient security for production systems.
It is highly recommended to use well-known CAs, and generate certificates at
multiple depth levels in order to conform to higher security standards.

The reference files are available here:
<https://github.com/stratum/stratum/tree/main/tools/tls>

### Running in secure mode

See [Install TLS Certificates guide](https://github.com/ipdk-io/networking-recipe/blob/main/docs/guides/install-tls-certificates.md) for step by step guide to generate
and install TLS certificates

#### Requirements for gRPC server

The IPDK Networking Recipe uses a secure-by-default model. If you wish to
open insecure ports, you must do so explicitly. It is strongly recommended
that you use secure ports in production systems.

infrap4d is launched with following gRPC ports secured via TLS certificates.
The port numbers are:

* 9339 - an IANA-registered port for gNMI
* 9559 - an IANA-registered port for P4RT

#### Requirements for gRPC clients

Under default conditions, the gRPC clients will require the TLS certificates
to establish communication with infrap4d. The clients will need
to use the same ca.crt file and the client.key and client.crt files signed
by the ca.crt (can copy the generated files from the server if client is not
on the same system as server).

- P4RT client

    The P4Runtime Control client will default to communicating in secure mode
using port 9559. If certificates are not available, the P4RT client will attempt
a connection using insecure client credentials as a fallback mechanism.
Note that the communication will fail if infrap4d runs in secure mode.
Both server and client must specify insecure mode for this to work.

- gNMI client

    gnmi-ctl (the gNMI client for DPDK) and sgnmi_cli (the secure gNMI client)
issue requests to port 9339.

### Running in insecure mode

Ports can be opened in insecure mode if needed. This is controlled
by a flag that needs to be enabled at runtime. Change the
`grpc_open_insecure_mode` value to `true` to allow insecure communication
Also, make sure `certs` directory is removed from the default location mentioned
above.

To launch infrap4d in insecure mode:

```bash
$IPDK_RECIPE/install/sbin/infrap4d  -grpc_open_insecure_mode=true
```

To launch clients in insecure mode:

- For DPDK,
   ```bash
   $IPDK_RECIPE/install/bin/gnmi-ctl set <COMMAND> \
   -grpc_use_insecure_mode=true
   ```

- FOR Intel IPU E2100,
   ```bash
   $IPDK_RECIPE/install/bin/sgnmi_cli set <COMMAND> \
   -grpc_use_insecure_mode=true
   ```
