// Copyright 2020-present Open Networking Foundation
// Copyright 2022-2024 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

#ifndef STRATUM_HAL_LIB_TDI_ES2K_ES2K_NODE_H_
#define STRATUM_HAL_LIB_TDI_ES2K_ES2K_NODE_H_

#include <memory>
#include <vector>

#include "absl/synchronization/mutex.h"
#include "p4/v1/p4runtime.grpc.pb.h"
#include "p4/v1/p4runtime.pb.h"
#include "stratum/glue/integral_types.h"
#include "stratum/glue/status/status.h"
#include "stratum/hal/lib/common/common.pb.h"
#include "stratum/hal/lib/common/writer_interface.h"
#include "stratum/hal/lib/tdi/tdi.pb.h"
#include "stratum/hal/lib/tdi/tdi_action_profile_manager.h"
#include "stratum/hal/lib/tdi/tdi_counter_manager.h"
#include "stratum/hal/lib/tdi/tdi_node.h"
#include "stratum/hal/lib/tdi/tdi_packetio_manager.h"
#include "stratum/hal/lib/tdi/tdi_pre_manager.h"
#include "stratum/hal/lib/tdi/tdi_table_manager.h"

namespace stratum {
namespace hal {
namespace tdi {

// The TdiNode class encapsulates all per P4-native node/chip/ASIC
// functionalities, primarily the flow managers. Calls made to this class are
// processed and passed through to the TDI API.
class Es2kNode : public TdiNode {
 public:
  virtual ~Es2kNode();

  // Forwarding entries
  virtual ::util::Status WriteForwardingEntries(
      const ::p4::v1::WriteRequest& req, std::vector<::util::Status>* results)
      LOCKS_EXCLUDED(lock_);
  virtual ::util::Status ReadForwardingEntries(
      const ::p4::v1::ReadRequest& req,
      WriterInterface<::p4::v1::ReadResponse>* writer,
      std::vector<::util::Status>* details) LOCKS_EXCLUDED(lock_);

  // Factory function for creating the instance of the class.
  static std::unique_ptr<Es2kNode> CreateInstance(
      TdiTableManager* tdi_table_manager,
      TdiActionProfileManager* tdi_action_profile_manager,
      TdiPacketioManager* tdi_packetio_manager, TdiPreManager* tdi_pre_manager,
      TdiCounterManager* tdi_counter_manager,
      TdiSdeInterface* tdi_sde_interface, int device_id,
      bool initialized = false, uint64 node_id = 0);

  // Es2kNode is neither copyable nor movable.
  Es2kNode(const Es2kNode&) = delete;
  Es2kNode& operator=(const Es2kNode&) = delete;
  Es2kNode(Es2kNode&&) = delete;
  Es2kNode& operator=(Es2kNode&&) = delete;

 protected:
  // Default constructor. To be called by the Mock class instance only.
  Es2kNode();

 private:
  // Private constructor. Use CreateInstance() to create an instance of this
  // class.
  Es2kNode(TdiTableManager* tdi_table_manager,
           TdiActionProfileManager* tdi_action_profile_manager,
           TdiPacketioManager* tdi_packetio_manager,
           TdiPreManager* tdi_pre_manager,
           TdiCounterManager* tdi_counter_manager,
           TdiSdeInterface* tdi_sde_interface, int device_id, bool initialized,
           uint64 node_id);

  // Write extern entries like ActionProfile, DirectCounter, PortMetadata
  ::util::Status WriteExternEntry(
      std::shared_ptr<TdiSdeInterface::SessionInterface> session,
      const ::p4::v1::Update::Type type, const ::p4::v1::ExternEntry& entry);

  // Read extern entries like ActionProfile, DirectCounter, PortMetadata
  ::util::Status ReadExternEntry(
      std::shared_ptr<TdiSdeInterface::SessionInterface> session,
      const ::p4::v1::ExternEntry& entry,
      WriterInterface<::p4::v1::ReadResponse>* writer);

  // Reader-writer lock used to protect access to node-specific state.
  mutable absl::Mutex lock_;

  // Mutex used for exclusive access to rx_writer_.
  mutable absl::Mutex rx_writer_lock_;

  // Flag indicating whether the pipeline has been pushed.
  bool pipeline_initialized_ GUARDED_BY(lock_);

  // Flag indicating whether the chip is initialized.
  bool initialized_ GUARDED_BY(lock_);

  // Stores pipeline information for this node.
  TdiDeviceConfig tdi_config_ GUARDED_BY(lock_);

  // Pointer to a TdiSdeInterface implementation that wraps all the SDE calls.
  // Not owned by this class.
  TdiSdeInterface* tdi_sde_interface_ = nullptr;

  // Managers. Not owned by this class.
  TdiTableManager* tdi_table_manager_;
  TdiActionProfileManager* tdi_action_profile_manager_;
  TdiPacketioManager* tdi_packetio_manager_;
  TdiPreManager* tdi_pre_manager_;
  TdiCounterManager* tdi_counter_manager_;

  // Persistent TDI session used by WriteForwardingEntries.
  std::shared_ptr<TdiSdeInterface::SessionInterface> forwarding_session_;

  // Logical node ID corresponding to the node/ASIC managed by this class
  // instance. Assigned on PushChassisConfig() and might change during the
  // lifetime of the class.
  uint64 node_id_ GUARDED_BY(lock_);

  // Fixed zero-based BFRT device_id number corresponding to the node/ASIC
  // managed by this class instance. Assigned in the class constructor.
  const int device_id_;
};

}  // namespace tdi
}  // namespace hal
}  // namespace stratum

#endif  // STRATUM_HAL_LIB_TDI_ES2K_ES2K_NODE_H_
