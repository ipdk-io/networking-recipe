// Copyright 2018 Google LLC
// Copyright 2018-present Open Networking Foundation
// Copyright 2023-2024 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

// This is a mock implementation of P4InfoManager.

#ifndef STRATUM_HAL_LIB_P4_P4_INFO_MANAGER_MOCK_H_
#define STRATUM_HAL_LIB_P4_P4_INFO_MANAGER_MOCK_H_

#include <string>

#include "gmock/gmock.h"
#include "stratum/hal/lib/p4/p4_info_manager.h"

namespace stratum {
namespace hal {

class P4InfoManagerMock : public P4InfoManager {
 public:
  MOCK_METHOD0(InitializeAndVerify, ::util::Status());

  // FindTable
  MOCK_CONST_METHOD1(
      FindTableByID,
      ::util::StatusOr<const ::p4::config::v1::Table>(uint32 table_id));
  MOCK_CONST_METHOD1(FindTableByName,
                     ::util::StatusOr<const ::p4::config::v1::Table>(
                         const std::string& table_name));

  // FindAction
  MOCK_CONST_METHOD1(
      FindActionByID,
      ::util::StatusOr<const ::p4::config::v1::Action>(uint32 action_id));
  MOCK_CONST_METHOD1(FindActionByName,
                     ::util::StatusOr<const ::p4::config::v1::Action>(
                         const std::string& action_name));

  // FindActionProfile
  MOCK_CONST_METHOD1(FindActionProfileByID,
                     ::util::StatusOr<const ::p4::config::v1::ActionProfile>(
                         uint32 profile_id));
  MOCK_CONST_METHOD1(FindActionProfileByName,
                     ::util::StatusOr<const ::p4::config::v1::ActionProfile>(
                         const std::string& profile_name));

  // FindCounter
  MOCK_CONST_METHOD1(
      FindCounterByID,
      ::util::StatusOr<const ::p4::config::v1::Counter>(uint32 counter_id));
  MOCK_CONST_METHOD1(FindCounterByName,
                     ::util::StatusOr<const ::p4::config::v1::Counter>(
                         const std::string& counter_name));

  // FindMeter
  MOCK_CONST_METHOD1(
      FindMeterByID,
      ::util::StatusOr<const ::p4::config::v1::Meter>(uint32 meter_id));
  MOCK_CONST_METHOD1(FindMeterByName,
                     ::util::StatusOr<const ::p4::config::v1::Meter>(
                         const std::string& meter_name));

  // FindDirectMeter
  MOCK_CONST_METHOD1(
      FindDirectMeterByID,
      ::util::StatusOr<const ::p4::config::v1::DirectMeter>(uint32 meter_id));
  MOCK_CONST_METHOD1(FindDirectMeterByName,
                     ::util::StatusOr<const ::p4::config::v1::DirectMeter>(
                         const std::string& meter_name));

  // FindPktModMeter
  MOCK_CONST_METHOD1(
      FindPktModMeterByID,
      ::util::StatusOr<const ::idpf::PacketModMeter>(uint32 meter_id));
  MOCK_CONST_METHOD1(FindPktModMeterByName,
                     ::util::StatusOr<const ::idpf::PacketModMeter>(
                         const std::string& meter_name));

  // FindDirectPktModMeter
  MOCK_CONST_METHOD1(
      FindDirectPktModMeterByID,
      ::util::StatusOr<const ::idpf::DirectPacketModMeter>(uint32 meter_id));
  MOCK_CONST_METHOD1(FindDirectPktModMeterByName,
                     ::util::StatusOr<const ::idpf::DirectPacketModMeter>(
                         const std::string& meter_name));

  // FindValueSet
  MOCK_CONST_METHOD1(
      FindValueSetByID,
      ::util::StatusOr<const ::p4::config::v1::ValueSet>(uint32 value_set_id));
  MOCK_CONST_METHOD1(FindValueSetByName,
                     ::util::StatusOr<const ::p4::config::v1::ValueSet>(
                         const std::string& value_set_name));

  // FindRegister
  MOCK_CONST_METHOD1(
      FindRegisterByID,
      ::util::StatusOr<const ::p4::config::v1::Register>(uint32 register_id));
  MOCK_CONST_METHOD1(FindRegisterByName,
                     ::util::StatusOr<const ::p4::config::v1::Register>(
                         const std::string& register_name));

  MOCK_CONST_METHOD1(
      GetSwitchStackAnnotations,
      ::util::StatusOr<P4Annotation>(const std::string& p4_object_name));
  MOCK_CONST_METHOD0(DumpNamesToIDs, void());
  MOCK_CONST_METHOD0(p4_info, const ::p4::config::v1::P4Info&());
  MOCK_METHOD0(VerifyRequiredObjects, ::util::Status());
};

}  // namespace hal
}  // namespace stratum

#endif  // STRATUM_HAL_LIB_P4_P4_INFO_MANAGER_MOCK_H_
