// Copyright 2020-present Open Networking Foundation
// Copyright 2022-2023 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

#ifndef STRATUM_HAL_LIB_TDI_ES2K_SWITCH_H_
#define STRATUM_HAL_LIB_TDI_ES2K_SWITCH_H_

#include <map>
#include <memory>
#include <string>
#include <vector>

#include "absl/synchronization/mutex.h"
#include "stratum/hal/lib/common/switch_interface.h"
#include "stratum/hal/lib/tdi/es2k/es2k_chassis_manager.h"
#include "stratum/hal/lib/tdi/es2k/es2k_node.h"
#include "stratum/hal/lib/tdi/es2k/es2k_virtual_port_manager.h"
#include "stratum/hal/lib/tdi/tdi_global_vars.h"
#include "stratum/hal/lib/tdi/tdi_ipsec_manager.h"

// Suppress clang errors
#undef LOCKS_EXCLUDED
#define LOCKS_EXCLUDED(...)

namespace stratum {
namespace hal {
namespace tdi {

class Es2kSwitch : public SwitchInterface {
 public:
  ~Es2kSwitch() override;

  // SwitchInterface public methods.
  ::util::Status PushChassisConfig(const ChassisConfig& config) override
      LOCKS_EXCLUDED(chassis_lock);
  ::util::Status VerifyChassisConfig(const ChassisConfig& config) override
      LOCKS_EXCLUDED(chassis_lock);
  ::util::Status PushForwardingPipelineConfig(
      uint64 node_id, const ::p4::v1::ForwardingPipelineConfig& config) override
      LOCKS_EXCLUDED(chassis_lock);
  ::util::Status SaveForwardingPipelineConfig(
      uint64 node_id, const ::p4::v1::ForwardingPipelineConfig& config) override
      LOCKS_EXCLUDED(chassis_lock);
  ::util::Status CommitForwardingPipelineConfig(uint64 node_id) override
      LOCKS_EXCLUDED(chassis_lock);
  ::util::Status VerifyForwardingPipelineConfig(
      uint64 node_id, const ::p4::v1::ForwardingPipelineConfig& config) override
      LOCKS_EXCLUDED(chassis_lock);
  ::util::Status Shutdown() override LOCKS_EXCLUDED(chassis_lock);
  ::util::Status Freeze() override;
  ::util::Status Unfreeze() override;
  ::util::Status WriteForwardingEntries(const ::p4::v1::WriteRequest& req,
                                        std::vector<::util::Status>* results)
      override LOCKS_EXCLUDED(chassis_lock);
  ::util::Status ReadForwardingEntries(
      const ::p4::v1::ReadRequest& req,
      WriterInterface<::p4::v1::ReadResponse>* writer,
      std::vector<::util::Status>* details) override
      LOCKS_EXCLUDED(chassis_lock);
  ::util::Status RegisterStreamMessageResponseWriter(
      uint64 node_id,
      std::shared_ptr<WriterInterface<::p4::v1::StreamMessageResponse>> writer)
      override LOCKS_EXCLUDED(chassis_lock);
  ::util::Status UnregisterStreamMessageResponseWriter(uint64 node_id) override
      LOCKS_EXCLUDED(chassis_lock);
  ::util::Status HandleStreamMessageRequest(
      uint64 node_id, const ::p4::v1::StreamMessageRequest& request) override
      LOCKS_EXCLUDED(chassis_lock);
  ::util::Status RegisterEventNotifyWriter(
      std::shared_ptr<WriterInterface<GnmiEventPtr>> writer) override
      LOCKS_EXCLUDED(chassis_lock);
  ::util::Status UnregisterEventNotifyWriter() override
      LOCKS_EXCLUDED(chassis_lock) LOCKS_EXCLUDED(chassis_lock);
  ::util::Status RetrieveValue(uint64 node_id, const DataRequest& requests,
                               WriterInterface<DataResponse>* writer,
                               std::vector<::util::Status>* details) override
      LOCKS_EXCLUDED(chassis_lock);
  ::util::Status SetValue(uint64 node_id, const SetRequest& request,
                          std::vector<::util::Status>* details) override
      LOCKS_EXCLUDED(chassis_lock);
  ::util::StatusOr<std::vector<std::string>> VerifyState() override;

  // Factory function for creating the instance of the class.
  static std::unique_ptr<Es2kSwitch> CreateInstance(
      Es2kChassisManager* chassis_manager, TdiIpsecManager* ipsec_manager,
      Es2kVirtualPortManager* vport_manager,
      const std::map<int, Es2kNode*>& device_id_to_es2k_node);

  // Es2kSwitch is neither copyable nor movable.
  Es2kSwitch(const Es2kSwitch&) = delete;
  Es2kSwitch& operator=(const Es2kSwitch&) = delete;
  Es2kSwitch(Es2kSwitch&&) = delete;
  Es2kSwitch& operator=(Es2kSwitch&&) = delete;

  TdiIpsecManager* GetIPsecManager() LOCKS_EXCLUDED(chassis_lock) {
    return ipsec_manager_;
  }

  Es2kVirtualPortManager* GetVportManager() LOCKS_EXCLUDED(chassis_lock) {
    return vport_manager_;
  }

 private:
  // Private constructor. Use CreateInstance() to create an instance of this
  // class.
  Es2kSwitch(Es2kChassisManager* chassis_manager,
             TdiIpsecManager* ipsec_manager,
             Es2kVirtualPortManager* vport_manager,
             const std::map<int, Es2kNode*>& device_id_to_es2k_node);

  // Helper to get Es2kNode pointer from device_id number or return error
  // indicating invalid device_id.
  ::util::StatusOr<Es2kNode*> GetEs2kNodeFromDeviceId(int device_id) const;

  // Helper to get Es2kNode pointer from node id or return error indicating
  // invalid/unknown/uninitialized node.
  ::util::StatusOr<Es2kNode*> GetEs2kNodeFromNodeId(uint64 node_id) const;

  // Pointer to ChassisManager object. Note that there is only one instance
  // of this class.
  Es2kChassisManager* chassis_manager_;  // not owned by the class.

  // Pointer to TdiIpsecManager object. Note that there is only one instance
  // of this class.
  TdiIpsecManager* ipsec_manager_;  // not owned by the class.

  // Pointer to Virtual port manager object. Note that there is only one
  // instance of this class.
  Es2kVirtualPortManager* vport_manager_;  // not owned by the class.

  // Map from zero-based device_id number corresponding to a node/ASIC to a
  // pointer to Es2kNode which contain all the per-node managers for that
  // node/ASIC. This map is initialized in the constructor and will not change
  // during the lifetime of the class.
  // TODO(max): Does this need to be protected by chassis_lock?
  const std::map<int, Es2kNode*> device_id_to_es2k_node_;  // pointers not owned

  // Map from the node ids to to a pointer to Es2kNode which contain all the
  // per-node managers for that node/ASIC. Created everytime a config is pushed.
  // At any point of time this map will contain a keys the ids of the nodes
  // which had a successful config push.
  // TODO(max): Does this need to be protected by chassis_lock?
  std::map<uint64, Es2kNode*> node_id_to_tdi_node_;  //  pointers not owned
};

}  // namespace tdi
}  // namespace hal
}  // namespace stratum

#endif  // STRATUM_HAL_LIB_TDI_ES2K_SWITCH_H_
