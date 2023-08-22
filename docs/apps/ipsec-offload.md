# IPsec Offload

The Inline IPsec offload feature uses the
[IPDK Infrastructure Application Interface](https://ipdk.io/documentation/Interfaces/InfraApp/)
to enable cryptographically-secure data traffic. The IPsec control plane
(IKE protocol) is offloaded, thus avoiding specialized drivers or control
planes burdening compute instances. Further details are available on the
[IPDK Inline Acceleration - IPsec](https://ipdk.io/documentation/Recipes/InlineIPsec/)
page.

This feature works in conjunction with the
[IPsec Recipe](https://github.com/ipdk-io/ipsec-recipe), which provides
strongSwan plugin code to configure and program control messages into the
target.

The IPsec offload feature is only supported on the Intel&reg; IPU E2100 target.

## Feature overview

The `infrap4d` process provides the gRPC server-side support for P4RT and
gNMI messages. strongSwan acts as the gRPC client in this context.
You can also use other clients that implement the IKE stack. The
client uses P4Runtime to program the Security Policy Database (SPD), and it
uses gNMI to configure Security Association Database (SAD) entries, which
includes encryption keys, re-keying etc.

The [openconfig-ipsec-offload](https://github.com/ipdk-io/openconfig-public/blob/master/release/models/ipsec/openconfig-ipsec-offload.yang)
YANG model is used to configure IPsec offload.

## Enabling IPsec

Follow the sequence of steps listed below to enable IPsec functionality.

### Compile P4 program

Compile the ipsec-offload program according to the instructions in
[Compiling P4 programs](/guides/es2k/compiling-p4-programs.md)
to generate crypto P4 artifacts for programming the pipelines.

### Load IPsec P4 package

Follow the instructions in [Deploying P4 programs](/guides/es2k/deploying-p4-programs.md)
to load the hardware FXP pipeline with the IPsec package.

### Configure and run infrap4d

To be able to program Security Association Database (SAD) entries using gNMI,
enable fixed function support in infrap4d. Follow the instructions in
[Running infrap4d](/guides/es2k/running-infrap4d.md)
to prepare system with generated TDI.json and context.json file references.

The /usr/share/stratum/es2k/es2k_skip_p4.conf file must include the fixed
function configuration reference.

```json
"fixed_functions" : [
  {
   "name": "crypto",
   "tdi": "/tmp/fixed/tdi.json",
   "ctx": "/tmp/fixed/crypto-mgr-ctx.json"
  }
],
```

Update this configuration before starting `infrap4d`.

### Configure strongSwan

Follow the instructions in the [strongSwan documentation](https://docs.strongswan.org/docs/5.9/index.html)
to configure host for the use-case selected. This includes details such as
certificate location, IPs, lifetime thresholds, etc.

### Start IPsec

With the strongSwan application configured, starting IPsec
(see [ipsec-recipe](https://github.com/ipdk-io/ipsec-recipe) for details) will
initiate the pipeline, program the SPD rules as per the P4 program, and
configure/re-configure SAD entries based on negotiated encryption parameters
between local and peer system.

## OpenConfig and gNMI messages

This section provides detailed information on OpenConfig model and gNMI
messages with the expected format. The strongSwan plugin has the following
details encoded.

### Config SAD message

The P4 Control Plane provides SET and DELETE service to the
IPsec-offload [Config SAD message](https://github.com/ipdk-io/openconfig-public/blob/master/release/models/ipsec/openconfig-ipsec-offload.yang#L39-L185)
at `/ipsec-offload/sad/sad-entry[offload-id=x][direction=y]/config`.

Note that individual leaf nodes of the YANG tree cannot be set and the gRPC
server expects the full config message as one single message. This design
was chosen in order to avoid caching of the encryption keys in the
software stack.

The expected format of the Config SAD message is:

```text
offload_id: 1,
direction: true,
req_id: 2,
spi: 599,
ext_seq_num: false,
anti_replay_window_size: 64,
protocol_parameters: IPSEC_PROTOCOL_PARAMS_ESP,
mode: IPSEC_MODE_TRANSPORT,
esp_payload {
  encryption {
    encryption_algorithm: 12,
    key: "33:65:6b:12:d7:9f:56:63:0b:59:a3:9f:8b:03:d2:c1:b8:51:98:e8",
    key_len: 20,
  }
},
sa_hard_lifetime {
  bytes: 200000
},
sa_soft_lifetime {
  bytes: 100000
}
```

### Fetch SPI message

The GET gNMI call is used to retrieve the
[SPI value](https://github.com/ipdk-io/openconfig-public/blob/master/release/models/ipsec/openconfig-ipsec-offload.yang#L292)
at `/ipsec-offload/ipsec-spi/rx-spi`.

### Key Expiry Notification message

The [gRPC Notification message](https://github.com/ipdk-io/openconfig-public/blob/master/release/models/ipsec/openconfig-ipsec-offload.yang#L308)
at `/ipsec-offload` is used as a signal to trigger the
re-keying mechanism in IKE protocol.

A gNMI subscription stream is opened from the gNMI client listening to these
notification messages originating in the target. Upon receiving this
notification, clients will initiate the re-keying mechanism to refresh
the encyrption keys.
