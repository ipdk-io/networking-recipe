// Copyright 2024 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

#ifndef OVSP4RT_CONTEXT_H_
#define OVSP4RT_CONTEXT_H_

#include "p4/v1/p4runtime.pb.h"

namespace ovs_p4rt {

class Envoy;
class Journal;

// Instantiated by a public API function and passed to internal functions
// it invokes during a call.
class Context {
 public:
  Context(Envoy& _envoy, ::p4::config::v1::P4Info& _p4info, Journal& _journal)
      : envoy(_envoy), p4info(_p4info), journal(_journal) {}

  // Interfaces to the P4Runtime session. Not owned by this object.
  Envoy& envoy;

  // Pipeline configuration. Not owned by this object.
  const ::p4::config::v1::P4Info& p4info;

  // Records the inputs and outputs of an API function.
  // Not owned by this object.
  Journal& journal;
};

}  // namespace ovs_p4rt

#endif  // OVSP4RT_CONTEXT_H_
