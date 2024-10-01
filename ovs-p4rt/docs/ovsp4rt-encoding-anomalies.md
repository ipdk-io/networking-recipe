# Summary of ovsp4rt Encoding Anomalies

When I was writing unit tests for ovsp4rt, I ran across a number of
anomalies in the encoding functions. In most cases, the function
encodes fewer bytes than the P4Runtime action value or match key
provides for, or than are required to support the full range of the
source value.

In the last few months, we've found and fixed errors in the
encoding of port numbers (8 bits instead of 16) and VNIs (16 bits
instead of 24, or 20 bits where the VNI is used as a Tunnel ID).
Given this history, I suspect that some if not all of the anomalies
I found are latent bugs that have not been found in testing or
discovered by our customers.

I have created
[GitHub issues](https://github.com/ipdk-io/networking-recipe/issues)
in the networking-recipe repository for each of these anomalies. Each
summary includes a link to the corresponding issue.

## Actions

### System Tests

I recommend that end-to-end test cases be created for each
encoding anomaly, to determine whether it is in fact a defect.

If the anomaly _is_ a defect, it should be fixed, the reference to
the issue should be removed from the code, and the
[unit test](#unit-tests) should be updated.

If the anomaly is _not_ a defect, the reference to the issue should
be replaced with a comment explaining why the value is being truncated,
and an explanation should be added to the GitHub issue before it is
closed.

### Unit Tests

The unit tests check for values that are consistent with the code
_in its current state_. If changes are made to address any of the
anomalies, the corresponding test cases should also be updated.
In particular, wider test values should be used in any case where
the encoded value is enlarged.

I also recommend that each byte of the test value be unique,
to help detect byte-order errors.

## Issues

### Truncated Fields

#### PrepareFdbRxVlanTableEntry() - DPDK #1

Encodes the port action param as a single byte.

- The input field is uint32_t.
- The action param is bit<32>.
- VLAN identifiers are bit<12>.

Also has a [value semantics anomaly](#preparefdbrxvlantableentry---dpdk-2).

See GitHub issue for details.

- <https://github.com/ipdk-io/networking-recipe/issues/683>


- <https://github.com/ipdk-io/networking-recipe/issues/683>

#### PrepareFdbRxVlanTableEntry() - ES2K

Encodes the port field as a single byte.

- The source field is uint32_t.
- The param value is bit<32>.

See GitHub issue for details.

- <https://github.com/ipdk-io/networking-recipe/issues/682>

#### PrepareFdbTableEntryforV4VxlanTunnel() - DPDK

DPDK variant encodes the `tunnel_id` action parameter as a single byte.

- The source field is uint32_t.
- The DPDK tunnel_id param is 24 bits wide.

See GitHub issue for details.

- <https://github.com/ipdk-io/networking-recipe/issues/677>

#### PrepareFdbTxVlanTableEntry() - DPDK #1

Encodes the transformed vlan_id (nominally bit<12>) as a single byte (bit<8>).

Also has a [value semantics anomaly](#preparefdbtxvlantableentry---dpdk-2).

See GitHub issue for details.

- <https://github.com/ipdk-io/networking-recipe/issues/689>

#### PrepareFdbTxVlanTableEntry() - ES2K #1

Encodes the port_vlan input as a single byte.

- The input value is an int.
- The parameter value is bit<24>.
- VLAN identifiers are bit<12>.

See GitHub issue for details.

- https://github.com/ipdk-io/networking-recipe/issues/620>

#### PrepareFdbTxVlanTableEntry() - ES2K #2

Encodes the src_port input as a single byte.

- The input value is uint32_t.
- The parameter value is bit<32>.

See GitHub issue for details.

- <https://github.com/ipdk-io/networking-recipe/issues/619>

#### PrepareGeneveDecapModAndVlanPushTableEntry()

Encodes the vlan_id field as a single byte.

- The source field is int.
- The param value is bit<12>.

See GitHub issue for details.

- <https://github.com/ipdk-io/networking-recipe/issues/679>

#### PrepareTunnelTermTableEntry() - DPDK

Encodes the tunnel_id as a single byte.

- The tunnel_id action param is bit<24>.

See GitHub issue for details.

- <https://github.com/ipdk-io/networking-recipe/issues/685>

#### PrepareTxAccVsiTableEntry()

Encodes the VSI match key value as a single byte.

- The input parameter is uint32_t.
- The match-key value is bit<11>.

See GitHub issue for details.

- <https://github.com/ipdk-io/networking-recipe/issues/680>

#### PrepareVlanPopTableEntry()

Encodes the vlan_id action param as a single byte.

- The canonical width of a VLAN identifier is bit<12>.
- The output field is bit<24>.

See GitHub issue for details.

- <https://github.com/ipdk-io/networking-recipe/issues/684>

#### PrepareVlanPushTableEntry()

Encodes the vlan_id parameter as as single byte in both the
`mod_blob_ptr` match key and the `vlan_id` action parameter.

- VLAN identifiers are bit<12>.

See GitHub issue for details.

- <https://github.com/ipdk-io/networking-recipe/issues/632>

#### PrepareVxlanDecapModAndVlanPushTableEntry()

Encodes the vlan_id action parameter as a single byte.

- The source field is int.
- The vlan_id action parameter is bit<12>.

See GitHub issue for details.

- <https://github.com/ipdk-io/networking-recipe/issues/678>

### Undefined fields

#### PrepareTunnelTermTableEntry()

Adds a match key for the Bridge ID (`user_meta.pmeta.bridge_id`).
The Linux Networking P4 program does not define this match field
for `ipv4_tunnel_term_table`.

See GitHub issue for details.

- <https://github.com/ipdk-io/networking-recipe/issues/617>

#### PrepareV6TunnelTermTableEntry()

Adds a match key for the Bridge ID (`user_meta.pmeta.bridge_id`).
The Linux Networking P4 program does not define this match field
for `ipv6_tunnel_term_table`.

See GitHub issue for details.

- <https://github.com/ipdk-io/networking-recipe/issues/617>

### Value Semantics

#### PrepareFdbRxVlanTableEntry() - DPDK #2

The DPDK variant has unusual value semantics.

- It uses the vln_info.vlan_id field as input, rather than a port-related field.
- It also sets the value of the port action param to vln_info.vlan_id - 1.

Also has a [field width anomaly](#preparefdbrxvlantableentry---dpdk-1).

See GitHub issue for details.

- <https://github.com/ipdk-io/networking-recipe/issues/683>

#### PrepareFdbTxVlanTableEntry() - DPDK #2

Subtracts one 1 from vlan_id and uses it as port_id.

Also has a [field width anomaly](#preparefdbtxvlantableentry---dpdk-1).

See GitHub issue for details.

- <https://github.com/ipdk-io/networking-recipe/issues/683>
