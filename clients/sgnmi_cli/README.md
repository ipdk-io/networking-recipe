<!--
Copyright 2018-present Open Networking Foundation
Copyright 2022-2023 Intel Corp

SPDX-License-Identifier: Apache-2.0
-->

Secure gNMI tool
----------------

This is a basic gNMI client tool that can be used for testing and sending messages
to a gNMI server (such as infrap4d). A YANG path needs to be specified with the
key and value.

This version of the tool `sgnmi_cli` is secure-by-default. It uses TLS certificates
to authenticate communication with the server. Default location for the certificates
is /usr/share/stratum/certs but can be overridden with a flag (details below).

It is also possible to run this tool in an insecure mode. Communication can be
established as long as the gNMI server also runs/supports insecure mode. To use
insecure mode, use flag `--grpc_use_insecure_mode=true`.

### Usage

```
usage: sgnmi_cli [--help] [Options] {get,set,cap,del,sub-onchange,sub-sample} path

Basic gNMI CLI

positional arguments:
  {get,set,cap,del,sub-onchange,sub-sample}         gNMI command
  path                                              gNMI path

optional arguments:
  --help            show this help message and exit
  --grpc_addr GRPC_ADDR    gNMI server address
  --bool_val BOOL_VAL      [SetRequest only] Set boolean value
  --int_val INT_VAL        [SetRequest only] Set int value (64-bit)
  --uint_val UINT_VAL      [SetRequest only] Set uint value (64-bit)
  --string_val STRING_VAL  [SetRequest only] Set string value
  --float_val FLOAT_VAL    [SetRequest only] Set float value
  --proto_bytes BYTES_VAL  [SetRequest only] Set proto_bytes value
  --bytes_val_file FILE    [SetRequest only] A file to be sent as bytes value
  --interval INTERVAL      [Sample subscribe only] Sample subscribe poll interval in ms
  --replace                [SetRequest only] Use replace instead of update
  --get-type               [GetRequest only] Use specific data type for get request (ALL,CONFIG,STATE,OPERATIONAL)
  --ca-cert                CA certificate
  --client-cert            gRPC Client certificate
  --client-key             gRPC Client key
  --grpc_use_insecure_mode Insecure mode (default: false)
```

### Examples

```
# To get port index
sgnmi_cli -- get /interfaces/interface[name=1/1/1]/state/ifindex

# To set port health indicator
sgnmi_cli -- set /interfaces/interface[name=1/1/1]/config/health-indicator --string-val GOOD

# To subscribe one sample of port operation status per second
sgnmi_cli -- sub-sample /interfaces/interface[name=1/1/1]/state/oper-status --interval 1000

# To push chassis config
sgnmi_cli -- --replace --bytes_val_file [chassis config file] set /
```
