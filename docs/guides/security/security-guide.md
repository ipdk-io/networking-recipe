# Security Guide

This document provides information about secure and insecure
modes for networking recipe and certificate management.

## TLS Certificates

The gRPC ports are secured using TLS certificates. A script and reference
configuration files are available to assist in generating certificates and
keys using OpenSSL. You may use other tools if you wish.

The [reference files](https://github.com/ipdk-io/stratum-dev/tree/split-arch/tools/tls)
use a simple PKI with a self-signed key and certificate.
The root level Certificate Authority (CA) is used to generate server-side
key and cert files, and client-side key and cert files. This results in a
1-depth level certificate chain, which will suffice for validation and
confirmation but may not provide sufficient security for production systems.
It is highly recommended to use well-known CAs, and generate certificates at
multiple depth levels in order to conform to higher security standards.

See [Using TLS Certificates](/guides/security/using-tls-certificates.md)
for step by step guide to generate and install TLS certificates

## Running in secure mode

### Server requirements

The IPDK Networking Recipe uses a secure-by-default model. If you wish to
open insecure ports, you must do so explicitly. It is strongly recommended
that you use secure ports in production systems.

infrap4d is launched with the gRPC ports secured by TLS certificates.
The port numbers are:

- 9339 - an IANA-registered port for gNMI
- 9559 - an IANA-registered port for P4RT

### Client requirements

Under default conditions, the gRPC clients will require the TLS certificates
to establish communication with infrap4d. The clients will need to use the
generated client.key and client.crt files signed by the same CA (can copy
the generated files from the server if client is not on the same system as
server).

- P4RT client

    The P4Runtime Control client will default to communicating in secure mode
using port 9559. If certificates are not available, the P4RT client will attempt
a connection using insecure client credentials as a fallback mechanism.
Note that the communication will fail if infrap4d runs in secure mode. Server
must specify insecure mode for this to work.

- gNMI client

    gnmi-ctl (the gNMI client for DPDK) and sgnmi_cli (the secure gNMI client)
issue requests to port 9339.

## Running in insecure mode

To launch infrap4d in insecure mode:

```bash
infrap4d -grpc_open_insecure_mode=true
```

To launch clients in insecure mode:

For DPDK target:

```bash
gnmi-ctl set <COMMAND> -grpc_use_insecure_mode=true
```

For Intel IPU E2100 target:

```bash
sgnmi_cli set <COMMAND> -grpc_use_insecure_mode=true
```
