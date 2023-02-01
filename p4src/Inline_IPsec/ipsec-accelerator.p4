/*
Copyright 2022 Intel Corporation

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

// IPsec accelerator result for the current packet
enum ipsec_status {
	IPSEC_SUCCESS,
	IPSEC_ERROR
}

// IPsec accelerator extern definition
extern ipsec_accelerator {
	// IPsec accelerator constructor.
	ipsec_accelerator();

	// Set the security association (SA) index for the current packet.
	//
	// When not invoked, the SA index to be used is undefined, leading to taget dependent
	// behavior.
	void set_sa_index<T>(in T sa_index);

	// Set the offset to the IPv4/IPv6 header for the current packet.
	//
	// When not invoked, the default value set for the IPv4/IPv6 header offset is 0.
	void set_ip_header_offset<T>(in T offset);

	// Enable the IPsec operation to be performed on the current packet.
	//
	// The type of the operation (i.e. encrypt/decrypt) and its associated parameters (such as
	// the crypto cipher and/or authentication parameters, the tunnel/transport mode headers,
	// etc) are completely specified by the SA that was set for the current packet.
	//
	// When enabled, this operation takes place once the deparser operation is completed.
	void enable();

	// Disable any IPsec operation that might have been previously enabled for the current
	// packet.
	void disable();

	// Returns true when the current packet has been processed by the IPsec accelerator and
	// reinjected back into the pipeline, and false otherwise.
	bool from_ipsec(out ipsec_status status);
}
