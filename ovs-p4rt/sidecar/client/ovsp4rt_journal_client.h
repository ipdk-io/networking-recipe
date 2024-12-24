// Copyright 2024 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

#ifndef OVSP4RT_JOURNAL_CLIENT_H_
#define OVSP4RT_JOURNAL_CLIENT_H_

#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "client/ovsp4rt_client.h"
#include "journal/ovsp4rt_journal.h"
#include "p4/v1/p4runtime.pb.h"

namespace ovsp4rt {

class JournalClient : public Client {
 public:
  JournalClient() {}
  virtual ~JournalClient() = default;

  // Sends a Read Table Entry request to the P4Runtime server.
  absl::StatusOr<p4::v1::ReadResponse> sendReadRequest(
      const p4::v1::ReadRequest& request) override;

  // Sends a Write Table Entry request to the P4Runtime server.
  absl::Status sendWriteRequest(const p4::v1::WriteRequest& request) override;

  // Returns a reference to the Journal object.
  Journal& journal() { return journal_; }

 private:
  // Journal object.
  Journal journal_;
};

}  // namespace ovsp4rt

#endif  // OVSP4RT_JOURNAL_CLIENT_H_
