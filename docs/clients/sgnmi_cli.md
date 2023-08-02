<!--
Copyright 2018-present Open Networking Foundation
Copyright 2022-2023 Intel Corporation

SPDX-License-Identifier: Apache-2.0
-->

# sgnmi_cli Client Guide

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
usage: sgnmi_cli [--help] [Options] COMMAND PATH

Secure gNMI CLI

positional arguments:
  COMMAND                  gNMI command
  PATH                     gNMI path

optional arguments:
  --grpc_addr GRPC_ADDR    gNMI server address
  --ca-cert                CA certificate
  --client-cert            gRPC Client certificate
  --client-key             gRPC Client key
  --grpc_use_insecure_mode Insecure mode (default: false)

[get request only]
  --get-type TYPE          Use specified data type for get request
                           (ALL,CONFIG,STATE,OPERATIONAL)

[set request only]
  --bool_val BOOL_VAL      Set boolean value
  --int_val INT_VAL        Set int value (64-bit)
  --uint_val UINT_VAL      Set uint value (64-bit)
  --string_val STRING_VAL  Set string value
  --float_val FLOAT_VAL    Set float value
  --proto_bytes BYTES_VAL  Set proto_bytes value
  --bytes_val_file FILE    File to be sent as bytes value
  --replace                Replace instead of updating

[sample subscribe only]
  --interval INTERVAL      Sample subscribe poll interval in ms

commands:
  get                      Get Request
  set                      Set Request
  cap                      Capability Request
  del                      Delete Request
  sub-onchange             Subscribe On Change Request
  sub-sample               Subscribe Sampled Request
```

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
        --ca-cert=/tmp/ca.crt \
        --client-cert=/tmp/client.crt \
        --client-key=/tmp/client.key
```
