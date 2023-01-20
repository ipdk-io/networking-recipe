# IPDK Security

## Overview

This document captures all the details related to gRPC ports and secure/insecure modes and certificate management.

## gRPC Ports

IPDK uses a secure-by-default model. The gRPC server will secure ports by default. A user may choose to open insecure ports if needed, but this will be a conscience decision taken by the user at run time.

### Secure-by-default (TLS-mode)

P4CP is launched only with gRPC ports open secured via TLS certificates. The port numbers are:

* 9339 - an IANA-registered port for gNMI and gNOI
* 9559 - an IANA-registered port for P4RT

### Generating TLS Certificates & installing with script

A script is available to generate and install the certificates in order to establish gRPC secure-mode communication. This setup script uses a pre-configured options and uses OpenSSL to generate the certificate and key files. 

To run the script, which generates certificate and key files and installs to default location:

```bash
$IPDK_RECIPE/scripts/security/setup_certs_tls_mode.sh
```

### Generating TLS Certificates & installing manually

The gRPC ports are secured using TLS certificates. A Stratum script and reference configuration files are available to assist in generating the certificates. The reference file uses OpenSSL to generate the keys & certificates. However, users may choose other tools.

The reference file uses a simple PKI where a self-signed RootCA is generated and the same RootCA is used to generate server key & cert and client key & cert. This results in a 1-depth level certificate chain, which will suffice for testing and confirmation, but production systems are encouraged to generate multiple depth levels in order to conform to higher security standards.

The Stratum reference files are available here: https://github.com/stratum/stratum/tree/main/tools/tls

All certificates are in PEM format.

To generate the TLS certificates:

* Review the files to verify if configuration settings are as-desired
* Run the generate-certs.sh script with following command:

```bash
COMMON_NAME=<IP> ./generate-certs.sh
or
COMMON_NAME=<FQDN> ./generate-certs.sh
or
COMMON_NAME=localhost ./generate-certs.sh
```

* Copy the generated ca.crt, stratum.crt and stratum.key to the server running InfraP4D
* Copy the generated ca.crt, client.crt and client.key to the gRPC client machine


#### Certificate installation

InfraP4D will check for certificate in the following default location:

> /usr/share/stratum/certs/

If alternate location is desired, the location will need to be specified during runtime with the following flags

> -ca_cert_file=[CA certificate file]
> -server_cert_file=[Server certificate file]
> -server_key_file=[Server private key file]

For e.g.:

```bash
# Files present in /tmp/certs/ directory
 
$IPDK_RECIPE/install/sbin/infrap4d  -ca_cert_file=/tmp/certs/ca.crt  -server_cert_file=/tmp/certs/stratum.crt  -server_key_file=/tmp/certs/stratum.key

```

### Opening Ports in Insecure Mode

Ports can be opened in insecure mode by user if needed. This is controlled by a flag that needs to be enabled during runtime. Change the grpc_open_insecure_ports value to true to open insecure ports. Also, make sure `certs` directory is removed from default location or user desired location as mentioned above.

To launch InfraP4D with insecure ports 9339 and 9559 open:

```bash

$IPDK_RECIPE/install/sbin/infrap4d  -grpc_open_insecure_mode=true

```


## gRPC Clients

Under default conditions, the gRPC clients will require the TLS certificates to establish communication with p4ovs/infrap4d server. The clients will need to use the same ca.crt file and the client.key and client.crt files signed by the ca.crt (can copy the generated files from the server if client is not on the same system as server).

P4RT Client
The p4rt-ctl (P4RT client) will default to communicate over secure mode (port 9559). If certificates are not available or if there are certificate read errors, it will try the insecure port as a fallback mechanism. This may fail if insecure ports are not open on the server.

The gnmi-ctl (gNMI client) requests should be directed to port 9339.
If the user desires to run the gnmi-ctl in insecure mode, a flag is available. Note that the insecure mode may fail if insecure ports are not open on the server.

```bash

$IPDK_RECIPE/install//bin/gnmi-ctl set <COMMAND>   -grpc_use_insecure_mode=true

```
