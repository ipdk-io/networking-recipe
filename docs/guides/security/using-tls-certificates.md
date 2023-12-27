# Using TLS Certificates

This document provides information about generating and installing TLS
certificates for running infrap4d in secure mode.

## Generating certificates

Note: Here target name can be `dpdk` or `es2k`.

Review the files `ca.conf` and `grpc-client.conf` available under
`/usr/share/stratum/` to verify that the configuration settings are
as desired.

Run the `generate-certs.sh` available under `/usr/share/stratum/`.

Note: Here `IP` is the IP address of gRPC server.
For example, `IP` can be `127.0.0.1`, `5.5.5.5` or `localhost`.

```bash
cd /usr/share/stratum/

COMMON_NAME=<IP> ./generate-certs.sh
```

The system relies on mTLS (mutual TLS) for authentication.

### OpenSSL version

The `/usr/share/stratum/generate-certs.sh` script uses the installed OpenSSL version to generate the certificates. 

OpenSSL 1.1.1x has reached EOL and usage should be discontinued. See the [OpenSSL security guide](openssl-guide.md) for details.

Also, note that if running gRPC clients on remote system, both systems should be running OpenSSL 3.x. Running an OpenSSL 1.1.1x client with a OpenSSL 3.x server has been known to fail TLS handshakes with `WRONG_VERSION_NUMBER` error when trying to establish communication.

## Installing certificates

`infrap4d` will check for server certificates in the default location
`/usr/share/stratum/certs/`.

### Default location

Copy the generated `ca.crt`, `stratum.crt`, and `stratum.key` in the
default location `/usr/share/stratum/certs/` to the server running infrap4d.

Copy the generated `ca.crt`, `client.crt`, and `client.key` in the
default location `/usr/share/stratum/certs/` to the client machine.

For more details about available options with respect to running infrap4d and
clients in insecure mode and default behavior, see the
[security_guide](security-guide.md).
