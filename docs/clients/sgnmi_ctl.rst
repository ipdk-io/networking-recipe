sgnmi_cli executable
--------------------

sgnmi_cli is a gNMI client tool that defaults to secure mode of communication
with a gNMI server. The gRPC communication is secured via TLS certificates. 

A flag is available if user chooses to use insecure mode.
For more information on security and refer to guides/security-guide.md document.

Usage
-----

usage: sgnmi_cli [--help] [Options] {get,set,cap,del,sub-onchange,sub-sample} path

gNMI CLI

positional arguments:
  {get,set,cap,del,sub-onchange,sub-sample}         gNMI command
  path                                              gNMI path

optional arguments:
  --help                   show this help message and exit
  --grpc_addr GRPC_ADDR    gNMI server address
  --bool_val BOOL_VAL      [SetRequest only] Set boolean value
  --int_val INT_VAL        [SetRequest only] Set int value (64-bit)
  --uint_val UINT_VAL      [SetRequest only] Set uint value (64-bit)
  --string_val STRING_VAL  [SetRequest only] Set string value
  --float_val FLOAT_VAL    [SetRequest only] Set float value
  --proto_bytes BYTES_VAL  [SetRequest only] Set proto_bytes value
  --interval INTERVAL      [Sample subscribe only] Sample subscribe poll interval in ms
  --replace                [SetRequest only] Use replace instead of update
  --get-type               [GetRequest only] Use specific data type for get request (ALL,CONFIG,STATE,OPERATIONAL)
  --ca-cert                CA certificate
  --client-cert            gRPC Client certificate
  --client-key             gRPC Client key
  --grpc_use_insecure_mode Insecure mode (default:false)

Examples
--------

# To get port index (which is used when referring to a port in P4Runtime)::
    sgnmi_cli -- get /interfaces/interface[name=1/1/1]/state/ifindex

# To set port health indicator::
    sgnmi_cli -- set /interfaces/interface[name=1/1/1]/config/health-indicator --string-val GOOD

# To subscribe one sample of port operation status per second::
    sgnmi_cli -- sub-sample /interfaces/interface[name=1/1/1]/state/oper-status --interval 1000

# To use insecure mode::
    sgnmi_cli -- -- get /interfaces/interface[name=1/1/1]/state/ifindex --grpc_use_insecure_mode=true

# To specify alternate certificate file paths::
    sgnmi_cli -- get /interfaces/interface[name=1/1/1]/state/ifindex \
        --ca-cert=/tmp/ca.crt \
        --client-cert=/tmp/client.crt \
        --client-key=/tmp/client.key

