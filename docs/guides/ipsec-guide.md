# IPsec Offload Guide

## 1. Overview

The [ipdk-io/ipsec-recipe](https://github.com/ipdk-io/ipsec-recipe) is an
independent IPDK recipe, which provides strongSwan plugin code to configure and
program control messages into the target.
Consult the ipsec-recipe for full setup information.

This document provides information pertaining to the gRPC server and the
IPsec-specific feature details, which the P4 Control Plane provides. 

The IPsec offload feature is only supported on the Intel&reg; IPU E2100 target.

## 2. Feature Overview

The `infrap4d` process provides the gRPC server-side support for P4RT and
gNMI messages. strongSwan acts as the gRPC client in this context.
Alternatively, you can use other clients that implement the IKE stack. The
client communicates using P4RT to program Security Policy Database (SPD) and
uses gNMI to configure Security Association Database (SAD) entries, which
includes encryption keys, re-keying, soft and hard lifetime values etc.

The YANG model uses for IPsec-offload is [available here](https://github.com/ipdk-io/openconfig-public/blob/master/release/models/ipsec/openconfig-ipsec-offload.yang).

Follow the sequence of steps listed below for enabling IPsec functionality.

### 2.1 Load Crypto Package

Follow instructions in [Deploying P4 programs](https://github.com/ipdk-io/networking-recipe/blob/main/docs/guides/es2k/deploying-p4-programs.md)
to load the hardware FXP pipeline with the crypto package.

## 2.2 Configure and Run Infrap4d

Follow instructions in [Running infrap4d on Intel&reg; IPU E2100](https://github.com/ipdk-io/networking-recipe/blob/main/docs/guides/es2k/running-infrap4d.md)
guide to prepare system with following details.

The /usr/share/stratum/es2k/es2k_skip_p4.conf file must include the fixed
function configuration reference.

```
"fixed_functions" : [
  {
   "name": "crypto",
   "tdi": "/tmp/fixed/tdi.json",
   "ctx": "/tmp/fixed/crypto-mgr-ctx.json"
  }
],
```

With this configuration updated, you can start infrap4d.

## 2.3 Compile P4 program

Follow instructions in [Compiling P4 programs](https://github.com/ipdk-io/networking-recipe/blob/main/docs/guides/es2k/compiling-p4-programs.md.)
to generate crypto P4 artifacts for programming the pipelines.

## 2.4 Configure strongSwan

Follow instructions in strongSwan documentation [here](https://docs.strongswan.org/docs/5.9/index.html).

## 2.5 Start IPsec

With the strongSwan application configured, starting IPsec
(see [ipsec-recipe](https://github.com/ipdk-io/ipsec-recipe) for details) will 
initiate the pipeline, program the SPD rules as per the P4 program, and
configure/re-configure SAD entries based on negotiated encryption parameters
between local and remote systems.

# 3. OpenConfig and gNMI Messages

This section provides detailed information on OpenConfig model and gNMI messages
with the expected format. The strongSwan plugin has the following details
encoded. This is only provided for clarity and information purposes.

### 3.1 The Config SAD message

The P4 Control Plane provides SET and DELETE API calls service to the 
IPsec-offload [Config SAD message](https://github.com/ipdk-io/openconfig-public/blob/master/release/models/ipsec/openconfig-ipsec-offload.yang#L39-L185)
at `/ipsec-offload/sad/sad-entry[offload-id=x][direction=y]/config`.

    Note that individual leaf nodes of the YANG tree cannot be set and the gRPC
    server expects the full config message as one single message. This design
    was chosen in order to avoid caching of the encryption keys in the
    software stack.

The expected format of the Config SAD message is:

```
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

The message is expected to be in [proto_bytes](https://github.com/openconfig/gnmi/blob/master/proto/gnmi/gnmi.proto#L133)
TypedValue as defined in gNMI specification.

### 3.2 The Fetch SPI message

The P4 Control Plane supports a GET gNMI call to retrieve the
[SPI value](https://github.com/ipdk-io/openconfig-public/blob/master/release/models/ipsec/openconfig-ipsec-offload.yang#L292)
at `/ipsec-offload/ipsec-spi/rx-spi`.

### 3.3 The Key Expiry Notification message

The P4 Control Plane supports a [gRPC Notification message](https://github.com/ipdk-io/openconfig-public/blob/master/release/models/ipsec/openconfig-ipsec-offload.yang#L308) when the soft lifetime byte count is passed at `/ipsec-offload`. 

A gNMI subscription stream is opened from the gNMI client listening to these
notification messages originating in the target. Upon receiving this
notification, clients will initiate the re-keying mechanism to refresh
the encyrption keys.
