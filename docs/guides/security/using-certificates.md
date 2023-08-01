# Working with TLS Certificates

This document provides information about generating and installing TLS certificates
for running infrap4d in secure mode.

## Generating certificates

Note: Here target name can be `dpdk` or `es2k`.

Review the files `ca.conf` and `grpc-client.conf` available under
`/usr/share/stratum/<target_name>` to verify that the configuration settings are
as desired.

Run the `generate-certs.sh` available under `/usr/share/stratum/<target_name>`.

Note: Here `IP` is the IP address of gRPC server.
For example, `IP` can be `127.0.0.1`, `5.5.5.5` or `localhost`.

```bash
cd /usr/share/stratum/<target_name>/

COMMON_NAME=<IP> ./generate-certs.sh
```

The system relies on mTLS (mutual TLS) for authentication.

## Installing certificates

`infrap4d` will check for server certificates in the default location
`/usr/share/stratum/certs/`.

### Default location

Copy the generated `ca.crt`, `stratum.crt`, and `stratum.key` in the
default location `/usr/share/stratum/certs/` to the server running infrap4d.

Copy the generated `ca.crt`, `client.crt`, and `client.key` in the
default location `/usr/share/stratum/certs/` to the client machine.

### Non-default location

If you would like to use a different location for the server certificates,
copy the certifactes to the location and specify the following options on
the `infrap4d` command line.

Option                 | Description
---------------------- | -------------------
-ca_cert_file=PATH     | CA certificate file
-server_cert_file=PATH | Server certificate file
-server_key_file=PATH  | Server private key file

For example, start infrap4d with certificates in `/tmp/certs`:

```bash
$P4CP_INSTALL/sbin/infrap4d \
    -ca_cert_file=/tmp/certs/ca.crt \
    -server_cert_file=/tmp/certs/stratum.crt \
    -server_key_file=/tmp/certs/stratum.key
```

Note: Client certificates must be installed in `/usr/share/stratum/certs`.

For more details about available options with respect to running infrap4d and
clients in insecure mode and default behavior, see
[security_guide](https://github.com/ipdk-io/networking-recipe/blob/main/docs/guides/security-guide.md)
