// Copyright 2019-present Barefoot Networks, Inc.
// Copyright 2022-2023 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

#ifndef STRATUM_HAL_LIB_TDI_TDI_SDE_MOCK_H_
#define STRATUM_HAL_LIB_TDI_TDI_SDE_MOCK_H_

#include <memory>
#include <string>
#include <vector>

#include "gmock/gmock.h"
#include "stratum/hal/lib/tdi/tdi_sde_interface.h"

namespace stratum {
namespace hal {
namespace tdi {

class SessionMock : public TdiSdeInterface::SessionInterface {
 public:
  MOCK_METHOD0(BeginBatch, ::util::Status());
  MOCK_METHOD0(EndBatch, ::util::Status());
};

class TableKeyMock : public TdiSdeInterface::TableKeyInterface {
 public:
  MOCK_METHOD2(SetExact, ::util::Status(int id, const std::string& value));
  MOCK_METHOD(::util::Status, SetExact, (std::string field_name, uint64 value));
  MOCK_CONST_METHOD2(GetExact, ::util::Status(int id, std::string* value));
  MOCK_METHOD3(SetTernary, ::util::Status(int id, const std::string& value,
                                          const std::string& mask));
  MOCK_CONST_METHOD3(GetTernary, ::util::Status(int id, std::string* value,
                                                std::string* mask));
  MOCK_METHOD3(SetLpm, ::util::Status(int id, const std::string& prefix,
                                      uint16 prefix_length));
  MOCK_CONST_METHOD3(GetLpm, ::util::Status(int id, std::string* prefix,
                                            uint16* prefix_length));
  MOCK_METHOD3(SetRange, ::util::Status(int id, const std::string& low,
                                        const std::string& high));
  MOCK_CONST_METHOD3(GetRange, ::util::Status(int id, std::string* low,
                                              std::string* high));
  MOCK_METHOD(::util::Status, SetPriority, (uint64 priority));
  MOCK_METHOD(::util::Status, GetPriority, (uint32 * priority), (const));
};

class TableDataMock : public TdiSdeInterface::TableDataInterface {
 public:
  MOCK_METHOD2(SetParam, ::util::Status(int id, const std::string& value));
  MOCK_METHOD(::util::Status, SetParam, (std::string field_name, uint64 value));
  MOCK_METHOD(::util::Status, SetParam,
              (std::string field_name, const std::string& value));

  MOCK_CONST_METHOD2(GetParam, ::util::Status(int id, std::string* value));
  MOCK_CONST_METHOD2(GetParam, ::util::Status(std::string name, uint64* value));

  MOCK_METHOD1(SetActionMemberId, ::util::Status(uint64 action_member_id));
  MOCK_CONST_METHOD1(GetActionMemberId,
                     ::util::Status(uint64* action_member_id));

  MOCK_METHOD1(SetSelectorGroupId, ::util::Status(uint64 selector_group_id));
  MOCK_CONST_METHOD1(GetSelectorGroupId,
                     ::util::Status(uint64* selector_group_id));

  MOCK_METHOD2(SetCounterData, ::util::Status(uint64 bytes, uint64 packets));
  MOCK_CONST_METHOD2(GetCounterData,
                     ::util::Status(uint64* bytes, uint64* packets));

  MOCK_METHOD5(SetMeterConfig,
               ::util::Status(bool in_pps, uint64 cir, uint64 cburst,
                              uint64 pir, uint64 pburst));
  MOCK_CONST_METHOD5(GetMeterConfig,
                     ::util::Status(bool in_pps, uint64* cir, uint64* cburst,
                                    uint64* pir, uint64* pburst));

  MOCK_METHOD1(SetPktModMeterConfig,
               ::util::Status(const TdiPktModMeterConfig& cfg));

  MOCK_CONST_METHOD1(GetPktModMeterConfig,
                     ::util::Status(TdiPktModMeterConfig& cfg));

  MOCK_CONST_METHOD1(GetActionId, ::util::Status(int* action_id));

  MOCK_METHOD1(Reset, ::util::Status(int action_id));
};

class TdiSdeMock : public TdiSdeInterface {
 public:
  MOCK_METHOD3(InitializeSde,
               ::util::Status(const std::string& sde_install_path,
                              const std::string& sde_config_file,
                              bool run_in_background));
  MOCK_METHOD2(AddDevice, ::util::Status(int device,
                                         const TdiDeviceConfig& device_config));
  MOCK_METHOD0(CreateSession,
               ::util::StatusOr<std::shared_ptr<SessionInterface>>());
  MOCK_METHOD1(IsSoftwareModel, ::util::StatusOr<bool>(int device));
  MOCK_CONST_METHOD1(GetChipType, std::string(int device));
  MOCK_CONST_METHOD0(GetSdeVersion, std::string());
  MOCK_METHOD2(TxPacket, ::util::Status(int device, const std::string& packet));
  MOCK_METHOD1(StartPacketIo, ::util::Status(int device));
  MOCK_METHOD1(StopPacketIo, ::util::Status(int device));
  MOCK_METHOD2(
      RegisterPacketReceiveWriter,
      ::util::Status(int device,
                     std::unique_ptr<ChannelWriter<std::string>> writer));
  MOCK_METHOD1(UnregisterPacketReceiveWriter, ::util::Status(int device));
  MOCK_METHOD5(CreateMulticastNode,
               ::util::StatusOr<uint32>(
                   int device,
                   std::shared_ptr<TdiSdeInterface::SessionInterface> session,
                   int mc_replication_id, const std::vector<uint32>& mc_lag_ids,
                   const std::vector<uint32>& ports));
  MOCK_METHOD3(
      DeleteMulticastNodes,
      ::util::Status(int device,
                     std::shared_ptr<TdiSdeInterface::SessionInterface> session,
                     const std::vector<uint32>& mc_node_ids));
  MOCK_METHOD6(
      GetMulticastNode,
      ::util::Status(int device,
                     std::shared_ptr<TdiSdeInterface::SessionInterface> session,
                     uint32 mc_node_id, int* replication_id,
                     std::vector<uint32>* lag_ids, std::vector<uint32>* ports));
  MOCK_METHOD4(
      InsertMulticastGroup,
      ::util::Status(int device,
                     std::shared_ptr<TdiSdeInterface::SessionInterface> session,
                     uint32 group_id, const std::vector<uint32>& mc_node_ids));
  MOCK_METHOD4(
      ModifyMulticastGroup,
      ::util::Status(int device,
                     std::shared_ptr<TdiSdeInterface::SessionInterface> session,
                     uint32 group_id, const std::vector<uint32>& mc_node_ids));
  MOCK_METHOD3(DeleteMulticastGroup,
               ::util::Status(int device,
                              std::shared_ptr<SessionInterface> session,
                              uint32 group_id));
  MOCK_METHOD5(
      GetMulticastGroups,
      ::util::Status(int device,
                     std::shared_ptr<TdiSdeInterface::SessionInterface> session,
                     uint32 group_id, std::vector<uint32>* group_ids,
                     std::vector<std::vector<uint32>>* mc_node_ids));
  MOCK_METHOD6(
      InsertCloneSession,
      ::util::Status(int device,
                     std::shared_ptr<TdiSdeInterface::SessionInterface> session,
                     uint32 session_id, int egress_port, int cos,
                     int max_pkt_len));
  MOCK_METHOD6(
      ModifyCloneSession,
      ::util::Status(int device,
                     std::shared_ptr<TdiSdeInterface::SessionInterface> session,
                     uint32 session_id, int egress_port, int cos,
                     int max_pkt_len));
  MOCK_METHOD3(GetNodesInMulticastGroup,
               ::util::StatusOr<std::vector<uint32>>(
                   int device,
                   std::shared_ptr<TdiSdeInterface::SessionInterface> session,
                   uint32 group_id));

  MOCK_METHOD3(
      DeleteCloneSession,
      ::util::Status(int device,
                     std::shared_ptr<TdiSdeInterface::SessionInterface> session,
                     uint32 session_id));
  MOCK_METHOD7(
      GetCloneSessions,
      ::util::Status(int device,
                     std::shared_ptr<TdiSdeInterface::SessionInterface> session,
                     uint32 session_id, std::vector<uint32>* session_ids,
                     std::vector<int>* egress_ports, std::vector<int>* coss,
                     std::vector<int>* max_pkt_lens));

  MOCK_METHOD6(
      WriteIndirectCounter,
      ::util::Status(int device,
                     std::shared_ptr<TdiSdeInterface::SessionInterface> session,
                     uint32 counter_id, int counter_index,
                     absl::optional<uint64> byte_count,
                     absl::optional<uint64> packet_count));
  MOCK_METHOD8(
      ReadIndirectCounter,
      ::util::Status(int device,
                     std::shared_ptr<TdiSdeInterface::SessionInterface> session,
                     uint32 counter_id, absl::optional<uint32> counter_index,
                     std::vector<uint32>* counter_indices,
                     std::vector<absl::optional<uint64>>* byte_counts,
                     std::vector<absl::optional<uint64>>* packet_counts,
                     absl::Duration timeout));

  MOCK_METHOD5(
      WriteRegister,
      ::util::Status(int device,
                     std::shared_ptr<TdiSdeInterface::SessionInterface> session,
                     uint32 table_id, absl::optional<uint32> register_index,
                     const std::string& register_data));
  MOCK_METHOD7(
      ReadRegisters,
      ::util::Status(int device,
                     std::shared_ptr<TdiSdeInterface::SessionInterface> session,
                     uint32 table_id, absl::optional<uint32> register_index,
                     std::vector<uint32>* register_indices,
                     std::vector<uint64>* register_values,
                     absl::Duration timeout));

  MOCK_METHOD9(
      WriteIndirectMeter,
      ::util::Status(int device,
                     std::shared_ptr<TdiSdeInterface::SessionInterface> session,
                     uint32 table_id, absl::optional<uint32> meter_index,
                     bool in_pps, uint64 cir, uint64 cburst, uint64 pir,
                     uint64 pburst));
  MOCK_METHOD10(
      ReadIndirectMeters,
      ::util::Status(int device,
                     std::shared_ptr<TdiSdeInterface::SessionInterface> session,
                     uint32 table_id, absl::optional<uint32> meter_index,
                     std::vector<uint32>* meter_indices,
                     std::vector<uint64>* cirs, std::vector<uint64>* cbursts,
                     std::vector<uint64>* pirs, std::vector<uint64>* pbursts,
                     std::vector<bool>* in_pps));

  MOCK_METHOD5(
      WritePktModMeter,
      ::util::Status(int device,
                     std::shared_ptr<TdiSdeInterface::SessionInterface> session,
                     uint32 table_id, absl::optional<uint32> meter_index,
                     TdiPktModMeterConfig& cfg));
  MOCK_METHOD6(
      ReadPktModMeters,
      ::util::Status(int device,
                     std::shared_ptr<TdiSdeInterface::SessionInterface> session,
                     uint32 table_id, absl::optional<uint32> meter_index,
                     std::vector<uint32>* meter_indices,
                     std::vector<TdiPktModMeterConfig>& cfg));

  MOCK_METHOD4(
      DeletePktModMeterConfig,
      ::util::Status(int device,
                     std::shared_ptr<TdiSdeInterface::SessionInterface> session,
                     uint32 table_id, absl::optional<uint32> meter_index));

  MOCK_METHOD5(
      InsertActionProfileMember,
      ::util::Status(int device,
                     std::shared_ptr<TdiSdeInterface::SessionInterface> session,
                     uint32 table_id, int member_id,
                     const TableDataInterface* table_data));
  MOCK_METHOD5(
      ModifyActionProfileMember,
      ::util::Status(int device,
                     std::shared_ptr<TdiSdeInterface::SessionInterface> session,
                     uint32 table_id, int member_id,
                     const TableDataInterface* table_data));
  MOCK_METHOD4(
      DeleteActionProfileMember,
      ::util::Status(int device,
                     std::shared_ptr<TdiSdeInterface::SessionInterface> session,
                     uint32 table_id, int member_id));
  MOCK_METHOD6(
      GetActionProfileMembers,
      ::util::Status(
          int device,
          std::shared_ptr<TdiSdeInterface::SessionInterface> session,
          uint32 table_id, int member_id, std::vector<int>* member_ids,
          std::vector<std::unique_ptr<TableDataInterface>>* table_values));

  MOCK_METHOD7(
      InsertActionProfileGroup,
      ::util::Status(int device,
                     std::shared_ptr<TdiSdeInterface::SessionInterface> session,
                     uint32 table_id, int group_id, int max_group_size,
                     const std::vector<uint32>& member_ids,
                     const std::vector<bool>& member_status));
  MOCK_METHOD7(
      ModifyActionProfileGroup,
      ::util::Status(int device,
                     std::shared_ptr<TdiSdeInterface::SessionInterface> session,
                     uint32 table_id, int group_id, int max_group_size,
                     const std::vector<uint32>& member_ids,
                     const std::vector<bool>& member_status));

  MOCK_METHOD4(
      DeleteActionProfileGroup,
      ::util::Status(int device,
                     std::shared_ptr<TdiSdeInterface::SessionInterface> session,
                     uint32 table_id, int group_id));
  MOCK_METHOD8(
      GetActionProfileGroups,
      ::util::Status(int device,
                     std::shared_ptr<TdiSdeInterface::SessionInterface> session,
                     uint32 table_id, int group_id, std::vector<int>* group_ids,
                     std::vector<int>* max_group_sizes,
                     std::vector<std::vector<uint32>>* member_ids,
                     std::vector<std::vector<bool>>* member_status));

  MOCK_METHOD1(
      CreateTableKey,
      ::util::StatusOr<std::unique_ptr<TableKeyInterface>>(uint32 table_id));
  MOCK_METHOD2(CreateTableData,
               ::util::StatusOr<std::unique_ptr<TableDataInterface>>(
                   uint32 table_id, uint32 action_id));

  MOCK_METHOD5(
      InsertTableEntry,
      ::util::Status(int device,
                     std::shared_ptr<TdiSdeInterface::SessionInterface> session,
                     uint32 table_id, const TableKeyInterface* table_key,
                     const TableDataInterface* table_data));
  MOCK_METHOD5(
      ModifyTableEntry,
      ::util::Status(int device,
                     std::shared_ptr<TdiSdeInterface::SessionInterface> session,
                     uint32 table_id, const TableKeyInterface* table_key,
                     const TableDataInterface* table_data));
  MOCK_METHOD4(
      DeleteTableEntry,
      ::util::Status(int device,
                     std::shared_ptr<TdiSdeInterface::SessionInterface> session,
                     uint32 table_id, const TableKeyInterface* table_key));
  MOCK_METHOD5(
      GetTableEntry,
      ::util::Status(int device,
                     std::shared_ptr<TdiSdeInterface::SessionInterface> session,
                     uint32 table_id, const TableKeyInterface* table_key,
                     TableDataInterface* table_data));
  MOCK_METHOD5(
      GetAllTableEntries,
      ::util::Status(
          int device,
          std::shared_ptr<TdiSdeInterface::SessionInterface> session,
          uint32 table_id,
          std::vector<std::unique_ptr<TableKeyInterface>>* table_keys,
          std::vector<std::unique_ptr<TableDataInterface>>* table_values));
  MOCK_METHOD4(
      SetDefaultTableEntry,
      ::util::Status(int device,
                     std::shared_ptr<TdiSdeInterface::SessionInterface> session,
                     uint32 table_id, const TableDataInterface* table_data));
  MOCK_METHOD3(
      ResetDefaultTableEntry,
      ::util::Status(int device,
                     std::shared_ptr<TdiSdeInterface::SessionInterface> session,
                     uint32 table_id));
  MOCK_METHOD4(
      GetDefaultTableEntry,
      ::util::Status(int device,
                     std::shared_ptr<TdiSdeInterface::SessionInterface> session,
                     uint32 table_id, TableDataInterface* table_data));
  MOCK_METHOD4(
      SynchronizeCounters,
      ::util::Status(int device,
                     std::shared_ptr<TdiSdeInterface::SessionInterface> session,
                     uint32 table_id, absl::Duration timeout));
  MOCK_METHOD(::util::StatusOr<uint32>, GetTdiRtId, (uint32 p4info_id),
              (const));
  MOCK_CONST_METHOD1(GetP4InfoId, ::util::StatusOr<uint32>(uint32 bfrt_id));
  MOCK_CONST_METHOD1(GetActionSelectorTdiRtId,
                     ::util::StatusOr<uint32>(uint32 action_profile_id));
  MOCK_CONST_METHOD1(GetActionProfileTdiRtId,
                     ::util::StatusOr<uint32>(uint32 action_selector_id));
  MOCK_METHOD(::util::StatusOr<uint32>, GetTableId, (std::string & table_name),
              (const));

  MOCK_METHOD(::util::Status, InitNotificationTableWithCallback,
              (int dev_id, std::shared_ptr<SessionInterface> session,
               const std::string& table_name,
               ipsec_notification_table_callback_t callback, void* cookie),
              (const));

  MOCK_METHOD(::util::Status, InitNotificationTableWithCallback,
              (int dev_id, std::shared_ptr<SessionInterface> session,
               const std::string& table_name,
               vport_notification_table_callback_t callback, void* cookie),
              (const));

  MOCK_METHOD(::util::Status, SetPacketIoConfig,
              (const PacketIoConfig& pktio_config));
};

}  // namespace tdi
}  // namespace hal
}  // namespace stratum

#endif  // STRATUM_HAL_LIB_TDI_TDI_SDE_MOCK_H_
