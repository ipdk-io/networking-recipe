## Generate and install TLS certificates:
Note: Here target name can be `dpdk` or `es2k`
- Review the files `ca.conf` and `grpc-client.conf` available under
`/usr/share/stratum/<target_name>` to verify that the configuration settings are
as desired
- Run the `generate-certs.sh` available under `/usr/share/stratum/<target_name>`

```bash
	cd /usr/share/stratum/<target_name>/

	COMMON_NAME=<IP> ./generate-certs.sh
	or
	COMMON_NAME=localhost ./generate-certs.sh
```
Note: System relies on mTLS (mutual TLS) for authentication. `infrap4d` will
check for certificate in the default location `/usr/share/stratum/certs/`

- Copy the generated `ca.crt`, `stratum.crt`, and `stratum.key` to the server
running infrap4d:
```bash
mkdir -p /usr/share/stratum/certs
cp /usr/share/stratum/es2k/certs/stratum.crt /usr/share/stratum/certs/
cp /usr/share/stratum/es2k/certs/ca.crt /usr/share/stratum/certs/
cp /usr/share/stratum/es2k/certs/stratum.key /usr/share/stratum/certs/
```

- Copy the generated `ca.crt`, `client.crt`, and `client.key` to the
client machine
```bash
mkdir -p /usr/share/stratum/certs
cp /usr/share/stratum/es2k/certs/ca.crt /usr/share/stratum/certs/
cp /usr/share/stratum/es2k/certs/client.crt /usr/share/stratum/certs/
cp /usr/share/stratum/es2k/certs/client.key /usr/share/stratum/certs/
```

If alternate location is desired for server certificates, the location must be
specified when running infrap4d.Note: Alternation location for client
certificates is not supported.

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
Note: For more details about available options with respect to running infrap4d
and clients in insecure mode and default behavior,see [security_guide](https://github.com/ipdk-io/networking-recipe/blob/main/docs/guides/security-guide.md)
