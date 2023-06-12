# Security Guide for P4 Control Plane

## Overview

This document provides information related to gRPC ports, secure/insecure
modes, and certificate management.

## gRPC ports

The IPDK Networking Recipe uses a secure-by-default model. If you wish to
open insecure ports, you must do so explicitly. It is strongly recommended
that you use secure ports in production systems.

### Secure-by-default (TLS-mode)

infrap4d is launched with gRPC ports secured via TLS certificates.
The port numbers are:

* 9339 - an IANA-registered port for gNMI
* 9559 - an IANA-registered port for P4RT

### Generating TLS certificates and installing manually

The gRPC ports are secured using TLS certificates. A script and reference
configuration files are available to assist in generating certificates and
keys using OpenSSL. You may use other tools if you wish.

The reference file uses a simple PKI where a self-signed key and certificate.
The root level Certificate Authority (CA) is used to generate server-side
key and cert files, and client-side key and cert files. This results in a 1-depth
level certificate chain, which will suffice for validation and confirmation
but may not provide sufficient security for production systems. It is
highly recommended to use well-known CAs, and generate certificates at multiple
depth levels in order to conform to higher security standards.

The reference files are available here: <https://github.com/stratum/stratum/tree/main/tools/tls>

All certificates are in PEM format.

To generate the TLS certificates:

* Review the files `ca.conf` and `grpc-client.conf` to verify if configuration settings are as-desired
* Run the generate-certs.sh script with following command:

```bash
COMMON_NAME=<IP> ./generate-certs.sh
or
COMMON_NAME=<FQDN> ./generate-certs.sh
or
COMMON_NAME=localhost ./generate-certs.sh
```

* Copy the generated ca.crt, stratum.crt, and stratum.key to the server
  running infraP4D
* Copy the generated ca.crt, client.crt, and client.key to the gRPC client
  machine

#### Certificate installation

infrap4d will check for certificate in the following default location:

> /usr/share/stratum/certs/

If alternate location is desired, the location will need to be specified
during runtime with the following flags:

```text
-ca_cert_file=[CA certificate file]
-server_cert_file=[Server certificate file]
-server_key_file=[Server private key file]
```

For example:

```bash
# Files present in /tmp/certs/ directory
$IPDK_RECIPE/install/sbin/infrap4d \
    -ca_cert_file=/tmp/certs/ca.crt \
    -server_cert_file=/tmp/certs/stratum.crt \
    -server_key_file=/tmp/certs/stratum.key
```

### Client certificate verification

infrap4d requires connecting gRPC clients to send a valid certificate
that can be verified. A flag is available to tune the level of security required.
The available values are:

```text
NO_REQUEST_CLIENT_CERT
REQUEST_CLIENT_CERT_NO_VERIFY
REQUEST_CLIENT_CERT_AND_VERIFY
REQUIRE_CLIENT_CERT_NO_VERIFY
REQUIRE_CLIENT_CERT_AND_VERIFY (default)
```

More info on these values can be found on [this gRPC library documentation page](https://grpc.github.io/grpc/cpp/grpc__security__constants_8h.html#a29ffe63a8bb3b4945ecab42d82758f09).

### Running in insecure mode

Ports can be opened in insecure mode if needed. This is controlled
by a flag that needs to be enabled at runtime. Change the
`grpc_open_insecure_ports` value to `true` to allow insecure communication
Also, make sure `certs` directory is removed from the default location mentioned
above.

To launch infrap4d in insecure mode:

```bash
$IPDK_RECIPE/install/sbin/infrap4d  -grpc_open_insecure_mode=true
```

## gRPC clients

Under default conditions, the gRPC clients will require the TLS certificates
to establish communication with infrap4d. The clients will need
to use the same ca.crt file and the client.key and client.crt files signed
by the ca.crt (can copy the generated files from the server if client is not
on the same system as server).

### P4RT client
The p4rt-ctl (P4RT client) will default to communicate via secure mode using
port 9559. If certificates are not available, the P4RT client will attempt a
connection using insecure client credentials as a fallback mechanism. Note that
the communication may fail if infrap4d runs in secure mode only. Both server and
client should specify insecure mode for this to work.

### gNMI client

The gnmi-ctl(gNMI client for DPDK) or sgnmi_cli (gNMI client for Intel IPU E2100) requests
are directed to port 9339. If infrap4d is run in insecure mode, a flag is available on client side
to use insecure mode.

For DPDK,
```bash
$IPDK_RECIPE/install/bin/gnmi-ctl set <COMMAND> -grpc_use_insecure_mode=true
```

FOR Intel IPU E2100,
```bash
$IPDK_RECIPE/install/bin/sgnmi_cli set <COMMAND> -grpc_use_insecure_mode=true
```

