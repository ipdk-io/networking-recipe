<!--
Copyright 2018-present Open Networking Foundation
Copyright 2022-2023 Intel Corporation

SPDX-License-Identifier: Apache-2.0
-->

# sgnmi_cli

This is a basic gNMI client that can be used for testing and sending
messages to a gNMI server such as `infrap4d`. A YANG path needs to be
specified with the key and value.

`sgnmi_cli` is a version of the Stratum gNMI tool (`gnmi_cli`) that operates
in secure mode by default. It uses TLS certificates to establish the
connection. You must create a certificate and install it to the server
in order to operate.

The tool may be run in insecure mode by specifying `--grpc_use_insecure_mode=true`
on the command line. `infrap4d` must be started in insecure mode for this to
work.

For more information, see the [Security Guide](/guides/security/security-guide.md).

## Usage

```text
sgnmi_cli: usage: sgnmi_cli [--help] [Options] COMMAND PATH

Secure gNMI CLI

positional arguments:
  COMMAND                  gNMI command
                           (get,set,cap,del,sub-onchange,sub-sample)
  PATH                     gNMI path

optional arguments:
  --helpshort              show help message and exit
  --help                   show help on all flags and exit
  --grpc_addr GRPC_ADDR    gNMI server address
  --ca_cert_file FILE      CA certificate file
  --client_cert_file FILE  Client certificate file
  --client_key_file FILE   Client key file
  --grpc_use_insecure_mode Insecure mode (default: false)

[get request only]
  --get-type TYPE          Use specific data type for get request
                           (ALL,CONFIG,STATE,OPERATIONAL)

[set request only]
  --bool_val BOOL_VAL      Set boolean value
  --int_val INT_VAL        Set int value (64-bit)
  --uint_val UINT_VAL      Set uint value (64-bit)
  --string_val STRING_VAL  Set string value
  --float_val FLOAT_VAL    Set float value
  --proto_bytes PROTO_VAL  Set protobuf bytes value
  --bytes_val_file FILE    Send file as bytes value
  --replace                Replace instead of updating

[sample subscribe only]
  --interval INTERVAL      Sample subscribe poll interval in ms
```

## Flags

Command-line flags are processed using the Google
[gflags](https://gflags.github.io/gflags/) library.

Flag names may be prefixed with one or two hyphens.
`-help` is the same as `--help`.

Underscores (`_`) and hyphens (`-`) may be used interchangeably within a
flag name. `-ca_cert_file` is the same as `-ca-cert-file`.

There are a great many built-in flags in addition to the ones listed above.
`-help` or `-helpfull` will list all the flags. `-helpshort` will list only
the flags that are specific to the current program.

Boolean flags may be specified several different ways:

- `-detach`, `-detach=yes`, `-detach=true` (enable)
- `-nodetach`, `-detach=no`, `-detach=false` (disable)

## Examples

### Get port index

```bash
    sgnmi_cli get /interfaces/interface[name=1/1/1]/state/ifindex
```

### Set Config SADB entry in ipsec-offload

```bash
    sgnmi_cli --proto_bytes="`cat /tmp/sadconfig.txt`" \
        set "/ipsec-offload/sad/sad-entry[name=1]/config"
```

### Subscribe to ipsec-offload notifications

```bash
    sgnmi_cli sub-onchange /ipsec-offload
```

### Use insecure mode

```bash
    sgnmi_cli get /interfaces/interface[name=1/1/1]/state/ifindex \
        --grpc_use_insecure_mode=true
```

### Specify alternate certificate file paths

```bash
    sgnmi_cli get /interfaces/interface[name=1/1/1]/state/ifindex \
        --ca-cert-file=/tmp/ca.crt \
        --client-cert=file=/tmp/client.crt \
        --client-key-file=/tmp/client.key
```
