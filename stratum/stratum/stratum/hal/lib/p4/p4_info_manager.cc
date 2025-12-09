// Copyright 2018 Google LLC
// Copyright 2018-present Open Networking Foundation
// Copyright 2023-2024 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

// P4InfoManager implementation.

#include "stratum/hal/lib/p4/p4_info_manager.h"

#include <utility>

#include "absl/strings/ascii.h"
#include "absl/strings/strip.h"
#include "absl/strings/substitute.h"
#include "gflags/gflags.h"
#include "idpf/p4info.pb.h"
#include "p4/config/v1/p4info.pb.h"
#include "stratum/glue/gtl/map_util.h"
#include "stratum/lib/macros.h"
#include "stratum/lib/utils.h"

// This flag allows unit tests to simplify their P4Info setup.  For example,
// a test that only wants to verify something about a Counter can enable this
// flag to avoid adding Actions, Tables, and Header Fields to its tested P4Info.
DEFINE_bool(skip_p4_min_objects_check, false,
            "When true, the check for minimum required P4 objects is not "
            "enforced.");

namespace stratum {
namespace hal {

P4InfoManager::P4InfoManager(const ::p4::config::v1::P4Info& p4_info)
    : p4_info_(p4_info),
      table_map_("Table"),
      action_map_("Action"),
      action_profile_map_("Action-Profile"),
      counter_map_("Counter"),
      direct_counter_map_("Direct-Counter"),
      meter_map_("Meter"),
      direct_meter_map_("Direct-Meter"),
      pkt_mod_meter_map_("PacketModMeter"),
      direct_pkt_mod_meter_map_("DirectPacketModMeter"),
      direct_ts_meter_map_("DirectTSMeter"),
      value_set_map_("ValueSet"),
      register_map_("Register"),
      digest_map_("Digest"),
      all_resource_ids_() {}

P4InfoManager::P4InfoManager()
    : table_map_("Table"),
      action_map_("Action"),
      action_profile_map_("Action-Profile"),
      counter_map_("Counter"),
      direct_counter_map_("Direct-Counter"),
      meter_map_("Meter"),
      direct_meter_map_("Direct-Meter"),
      pkt_mod_meter_map_("PacketModMeter"),
      direct_pkt_mod_meter_map_("DirectPacketModMeter"),
      direct_ts_meter_map_("DirectTSMeter"),
      value_set_map_("ValueSet"),
      register_map_("Register"),
      digest_map_("Digest"),
      all_resource_ids_() {}

P4InfoManager::~P4InfoManager() {}

// Since P4InfoManager can be used in a verify role, it attempts to continue
// processing after most errors in order to describe every problem it
// encounters in p4_info_.
::util::Status P4InfoManager::InitializeAndVerify() {
  if (!all_resource_ids_.empty() || !all_resource_names_.empty()) {
    return MAKE_ERROR(ERR_INTERNAL) << "P4Info is already initialized";
  }

  ::util::Status status = ::util::OkStatus();
  APPEND_STATUS_IF_ERROR(status, VerifyRequiredObjects());
  PreambleCallback preamble_cb =
      std::bind(&P4InfoManager::ProcessPreamble, this, std::placeholders::_1,
                std::placeholders::_2);
  APPEND_STATUS_IF_ERROR(status,
                         table_map_.BuildMaps(p4_info_.tables(), preamble_cb));
  APPEND_STATUS_IF_ERROR(
      status, action_map_.BuildMaps(p4_info_.actions(), preamble_cb));
  APPEND_STATUS_IF_ERROR(status, action_profile_map_.BuildMaps(
                                     p4_info_.action_profiles(), preamble_cb));
  APPEND_STATUS_IF_ERROR(
      status, counter_map_.BuildMaps(p4_info_.counters(), preamble_cb));
  APPEND_STATUS_IF_ERROR(status, direct_counter_map_.BuildMaps(
                                     p4_info_.direct_counters(), preamble_cb));
  APPEND_STATUS_IF_ERROR(status,
                         meter_map_.BuildMaps(p4_info_.meters(), preamble_cb));
  APPEND_STATUS_IF_ERROR(status, direct_meter_map_.BuildMaps(
                                     p4_info_.direct_meters(), preamble_cb));
  APPEND_STATUS_IF_ERROR(
      status, value_set_map_.BuildMaps(p4_info_.value_sets(), preamble_cb));
  APPEND_STATUS_IF_ERROR(
      status, register_map_.BuildMaps(p4_info_.registers(), preamble_cb));
  APPEND_STATUS_IF_ERROR(
      status, digest_map_.BuildMaps(p4_info_.digests(), preamble_cb));

  // This code depends on a proposed change to the P4Runtime specification,
  // and is provisional.
  if (!p4_info_.externs().empty()) {
    for (const auto& p4extern : p4_info_.externs()) {
      switch (p4extern.extern_type_id()) {
        case ::p4::config::v1::P4Ids_Prefix_PACKET_MOD_METER:
          InitPacketModMeters(p4extern);
          break;
        case ::p4::config::v1::P4Ids_Prefix_DIRECT_PACKET_MOD_METER:
          InitDirectPacketModMeters(p4extern);
          break;
        case ::idpf::P4Ids_Prefix_DIRECT_TSMETER:
          InitDirectTSMeters(p4extern);
          break;
        default:
          LOG(INFO) << "Unrecognized p4_info extern type: "
                    << p4extern.extern_type_id() << " (ignored)";
          break;
      }
    }
  }

  APPEND_STATUS_IF_ERROR(status, VerifyTableXrefs());

  return status;
}

void P4InfoManager::InitDirectPacketModMeters(
    const p4::config::v1::Extern& p4extern) {
  const auto& extern_instances = p4extern.instances();
  PreambleCallback preamble_cb =
      std::bind(&P4InfoManager::ProcessPreamble, this, std::placeholders::_1,
                std::placeholders::_2);
  for (const auto& extern_instance : extern_instances) {
    ::idpf::DirectPacketModMeter direct_pkt_mod_meter;
    *direct_pkt_mod_meter.mutable_preamble() = extern_instance.preamble();
    p4::config::v1::MeterSpec meter_spec;
    meter_spec.set_unit(p4::config::v1::MeterSpec::BYTES);
    *direct_pkt_mod_meter.mutable_spec() = meter_spec;
    direct_meter_objects_.Add(std::move(direct_pkt_mod_meter));
  }
  direct_pkt_mod_meter_map_.BuildMaps(direct_meter_objects_, preamble_cb);
}

void P4InfoManager::InitDirectTSMeters(const p4::config::v1::Extern& p4extern) {
  const auto& extern_instances = p4extern.instances();
  PreambleCallback preamble_cb =
      std::bind(&P4InfoManager::ProcessPreamble, this, std::placeholders::_1,
                std::placeholders::_2);
  for (const auto& extern_instance : extern_instances) {
    ::idpf::DirectTSMeter direct_ts_meter;
    *direct_ts_meter.mutable_preamble() = extern_instance.preamble();
    p4::config::v1::MeterSpec meter_spec;
    meter_spec.set_unit(p4::config::v1::MeterSpec::PACKETS);
    *direct_ts_meter.mutable_spec() = meter_spec;
    direct_ts_meter_objects_.Add(std::move(direct_ts_meter));
  }
  direct_ts_meter_map_.BuildMaps(direct_ts_meter_objects_, preamble_cb);
}

void P4InfoManager::InitPacketModMeters(
    const p4::config::v1::Extern& p4extern) {
  const auto& extern_instances = p4extern.instances();
  PreambleCallback preamble_cb =
      std::bind(&P4InfoManager::ProcessPreamble, this, std::placeholders::_1,
                std::placeholders::_2);
  for (const auto& extern_instance : extern_instances) {
    ::idpf::PacketModMeter pkt_mod_meter;
    *pkt_mod_meter.mutable_preamble() = extern_instance.preamble();
    p4::config::v1::MeterSpec meter_spec;
    meter_spec.set_unit(p4::config::v1::MeterSpec::PACKETS);
    pkt_mod_meter.set_size(1024);
    pkt_mod_meter.set_index_width(20);
    *pkt_mod_meter.mutable_spec() = meter_spec;
    all_meter_objects_.Add(std::move(pkt_mod_meter));
  }
  pkt_mod_meter_map_.BuildMaps(all_meter_objects_, preamble_cb);
}

// FindTable
::util::StatusOr<const ::p4::config::v1::Table> P4InfoManager::FindTableByID(
    uint32 table_id) const {
  return table_map_.FindByID(table_id);
}

::util::StatusOr<const ::p4::config::v1::Table> P4InfoManager::FindTableByName(
    const std::string& table_name) const {
  return table_map_.FindByName(table_name);
}

// FindAction
::util::StatusOr<const ::p4::config::v1::Action> P4InfoManager::FindActionByID(
    uint32 action_id) const {
  return action_map_.FindByID(action_id);
}

::util::StatusOr<const ::p4::config::v1::Action>
P4InfoManager::FindActionByName(const std::string& action_name) const {
  return action_map_.FindByName(action_name);
}

// FindActionProfile
::util::StatusOr<const ::p4::config::v1::ActionProfile>
P4InfoManager::FindActionProfileByID(uint32 profile_id) const {
  return action_profile_map_.FindByID(profile_id);
}

::util::StatusOr<const ::p4::config::v1::ActionProfile>
P4InfoManager::FindActionProfileByName(const std::string& profile_name) const {
  return action_profile_map_.FindByName(profile_name);
}

// FindCounter
::util::StatusOr<const ::p4::config::v1::Counter>
P4InfoManager::FindCounterByID(uint32 counter_id) const {
  return counter_map_.FindByID(counter_id);
}

::util::StatusOr<const ::p4::config::v1::Counter>
P4InfoManager::FindCounterByName(const std::string& counter_name) const {
  return counter_map_.FindByName(counter_name);
}

// FindDirectCounter
::util::StatusOr<const ::p4::config::v1::DirectCounter>
P4InfoManager::FindDirectCounterByID(uint32 counter_id) const {
  return direct_counter_map_.FindByID(counter_id);
}

::util::StatusOr<const ::p4::config::v1::DirectCounter>
P4InfoManager::FindDirectCounterByName(const std::string& counter_name) const {
  return direct_counter_map_.FindByName(counter_name);
}

// FindMeter
::util::StatusOr<const ::p4::config::v1::Meter> P4InfoManager::FindMeterByID(
    uint32 meter_id) const {
  return meter_map_.FindByID(meter_id);
}

::util::StatusOr<const ::p4::config::v1::Meter> P4InfoManager::FindMeterByName(
    const std::string& meter_name) const {
  return meter_map_.FindByName(meter_name);
}

// FindDirectMeter
::util::StatusOr<const ::p4::config::v1::DirectMeter>
P4InfoManager::FindDirectMeterByID(uint32 meter_id) const {
  return direct_meter_map_.FindByID(meter_id);
}

::util::StatusOr<const ::p4::config::v1::DirectMeter>
P4InfoManager::FindDirectMeterByName(const std::string& meter_name) const {
  return direct_meter_map_.FindByName(meter_name);
}

// FindPktModMeter
::util::StatusOr<const ::idpf::PacketModMeter>
P4InfoManager::FindPktModMeterByID(uint32 meter_id) const {
  return pkt_mod_meter_map_.FindByID(meter_id);
}

::util::StatusOr<const ::idpf::PacketModMeter>
P4InfoManager::FindPktModMeterByName(const std::string& meter_name) const {
  return pkt_mod_meter_map_.FindByName(meter_name);
}

// FindDirectPktModMeter
::util::StatusOr<const ::idpf::DirectPacketModMeter>
P4InfoManager::FindDirectPktModMeterByID(uint32 meter_id) const {
  return direct_pkt_mod_meter_map_.FindByID(meter_id);
}

::util::StatusOr<const ::idpf::DirectPacketModMeter>
P4InfoManager::FindDirectPktModMeterByName(
    const std::string& meter_name) const {
  return direct_pkt_mod_meter_map_.FindByName(meter_name);
}

// FindValueSet
::util::StatusOr<const ::p4::config::v1::ValueSet>
P4InfoManager::FindValueSetByID(uint32 value_set_id) const {
  return value_set_map_.FindByID(value_set_id);
}

::util::StatusOr<const ::p4::config::v1::ValueSet>
P4InfoManager::FindValueSetByName(const std::string& value_set_name) const {
  return value_set_map_.FindByName(value_set_name);
}

// FindRegister
::util::StatusOr<const ::p4::config::v1::Register>
P4InfoManager::FindRegisterByID(uint32 register_id) const {
  return register_map_.FindByID(register_id);
}

::util::StatusOr<const ::p4::config::v1::Register>
P4InfoManager::FindRegisterByName(const std::string& register_name) const {
  return register_map_.FindByName(register_name);
}

// FindDigest
::util::StatusOr<const ::p4::config::v1::Digest> P4InfoManager::FindDigestByID(
    uint32 digest_id) const {
  return digest_map_.FindByID(digest_id);
}

::util::StatusOr<const ::p4::config::v1::Digest>
P4InfoManager::FindDigestByName(const std::string& digest_name) const {
  return digest_map_.FindByName(digest_name);
}

// FindResourceType
::util::StatusOr<const std::string> P4InfoManager::FindResourceTypeByID(
    uint32 id_key) const {
  auto iter = id_to_resource_type_map_.find(id_key);
  if (iter == id_to_resource_type_map_.end()) {
    return MAKE_ERROR(ERR_INVALID_P4_INFO)
           << "P4Info ID " << PrintP4ObjectID(id_key) << " is not found";
  }
  return iter->second;
}

::util::StatusOr<P4Annotation> P4InfoManager::GetSwitchStackAnnotations(
    const std::string& p4_object_name) const {
  auto preamble_ptr_ptr = gtl::FindOrNull(all_resource_names_, p4_object_name);
  if (preamble_ptr_ptr == nullptr) {
    return MAKE_ERROR(ERR_INVALID_P4_INFO)
           << "P4Info object " << p4_object_name << " does not exist or does "
           << "not contain a Preamble";
  }

  const ::p4::config::v1::Preamble* preamble_ptr = *preamble_ptr_ptr;
  P4Annotation p4_annotation;
  for (const auto& annotation : preamble_ptr->annotations()) {
    // TODO(unknown): Investigate to what degree p4c enforces annotation
    // syntax and whether something beyond the simple parsing below is needed.

    auto stripped_annotation = absl::StripAsciiWhitespace(annotation);
    if (!absl::ConsumePrefix(&stripped_annotation, "@switchstack(\"")) {
      // Any prefixes that don't match are assumed to be other
      // types of annotations.
      continue;
    }
    if (!absl::ConsumeSuffix(&stripped_annotation, "\")")) {
      // Improperly terminated annotations are errors.
      return MAKE_ERROR(ERR_INVALID_P4_INFO)
             << "@switchstack annotation in P4Info object " << p4_object_name
             << " has invalid syntax";
    }
    P4Annotation parsed_annotation;
    ::util::Status parse_status = ParseProtoFromString(
        std::string(stripped_annotation), &parsed_annotation);
    if (!parse_status.ok()) {
      return parse_status;
    }
    p4_annotation.MergeFrom(parsed_annotation);
  }
  return p4_annotation;
}

::util::Status P4InfoManager::VerifyRegisterEntry(
    const ::p4::v1::RegisterEntry& register_entry) const {
  ASSIGN_OR_RETURN(auto reg, FindRegisterByID(register_entry.register_id()));

  // Check the register index, if it's not a wildcard read/write.
  if (register_entry.has_index()) {
    RET_CHECK(register_entry.index().index() >= 0);
    RET_CHECK(register_entry.index().index() < reg.size());
  }

  // Check the type spec, if the entry carries data.
  if (register_entry.has_data()) {
    RETURN_IF_ERROR(VerifyTypeSpec(register_entry.data(), reg.type_spec()));
  }

  return ::util::OkStatus();
}

::util::Status P4InfoManager::VerifyTypeSpec(
    const ::p4::v1::P4Data& data,
    const ::p4::config::v1::P4DataTypeSpec& type_spec) const {
  switch (data.data_case()) {
    case ::p4::v1::P4Data::kBitstring: {
      RET_CHECK(type_spec.has_bitstring())
          << "The type spec does not specify a bitstring type for P4Data "
          << data.ShortDebugString() << ".";
      int bit_width;
      switch (type_spec.bitstring().type_spec_case()) {
        case ::p4::config::v1::P4BitstringLikeTypeSpec::kBit:
          bit_width = type_spec.bitstring().bit().bitwidth();
          break;
        case ::p4::config::v1::P4BitstringLikeTypeSpec::kInt:
          bit_width = type_spec.bitstring().int_().bitwidth();
          break;
        case ::p4::config::v1::P4BitstringLikeTypeSpec::kVarbit:
          bit_width = type_spec.bitstring().varbit().max_bitwidth();
          break;
        default:
          return MAKE_ERROR(ERR_UNIMPLEMENTED) << "Not implemented.";
      }
      RET_CHECK(data.bitstring().size() * 8 <= bit_width);
      break;
    }
    case ::p4::v1::P4Data::kTuple: {
      RET_CHECK(type_spec.has_tuple())
          << "The type spec does not specify a tuple type for P4Data "
          << data.ShortDebugString() << ".";
      RET_CHECK(data.tuple().members_size() ==
                type_spec.tuple().members_size());
      for (size_t i = 0; i < data.tuple().members_size(); ++i) {
        RETURN_IF_ERROR(VerifyTypeSpec(data.tuple().members(i),
                                       type_spec.tuple().members(i)));
      }
      break;
    }
    default:
      return MAKE_ERROR(ERR_UNIMPLEMENTED)
             << "P4data type " << data.data_case() << " in P4Data "
             << data.ShortDebugString() << " is not supported.";
  }

  return ::util::OkStatus();
}

void P4InfoManager::DumpNamesToIDs() const {
  table_map_.DumpNamesToIDs();
  action_map_.DumpNamesToIDs();
  action_profile_map_.DumpNamesToIDs();
  counter_map_.DumpNamesToIDs();
  direct_counter_map_.DumpNamesToIDs();
  meter_map_.DumpNamesToIDs();
  direct_meter_map_.DumpNamesToIDs();
  value_set_map_.DumpNamesToIDs();
  register_map_.DumpNamesToIDs();
  digest_map_.DumpNamesToIDs();
}

::util::Status P4InfoManager::VerifyRequiredObjects() {
  if (FLAGS_skip_p4_min_objects_check) return ::util::OkStatus();

  std::string missing_objects;
  if (!p4_info_.tables_size()) {
    missing_objects +=
        absl::Substitute(" $0s", table_map_.resource_type().c_str());
  }
  if (!p4_info_.actions_size()) {
    missing_objects +=
        absl::Substitute(" $0s", action_map_.resource_type().c_str());
  }

  if (!missing_objects.empty()) {
    return MAKE_ERROR(ERR_INTERNAL)
           << "P4Info is missing these required resources:" << missing_objects;
  }

  return ::util::OkStatus();
}

// Validates preamble's name and ID fields and makes sure they are globally
// unique.
::util::Status P4InfoManager::ProcessPreamble(
    const ::p4::config::v1::Preamble& preamble,
    const std::string& resource_type) {
  auto status = P4InfoManager::VerifyID(preamble, resource_type);
  auto name_status = P4InfoManager::VerifyName(preamble, resource_type);
  APPEND_STATUS_IF_ERROR(status, name_status);

  if (status.ok()) {
    uint32 id_key = preamble.id();
    auto id_result = all_resource_ids_.insert(id_key);
    if (!id_result.second) {
      ::util::Status insert_id_status = MAKE_ERROR(ERR_INVALID_P4_INFO)
                                        << "P4Info " << resource_type << " ID "
                                        << PrintP4ObjectID(id_key)
                                        << " is not unique";
      APPEND_STATUS_IF_ERROR(status, insert_id_status);
    }

    auto id_type_result =
        id_to_resource_type_map_.emplace(id_key, resource_type);
    if (!id_type_result.second) {
      ::util::Status insert_id_status = MAKE_ERROR(ERR_INVALID_P4_INFO)
                                        << "P4Info " << resource_type << " ID "
                                        << PrintP4ObjectID(id_key)
                                        << " is not unique";
      APPEND_STATUS_IF_ERROR(status, insert_id_status);
    }

    const std::string name_key = preamble.name();
    auto name_result =
        all_resource_names_.insert(std::make_pair(name_key, &preamble));
    if (!name_result.second) {
      ::util::Status insert_name_status = MAKE_ERROR(ERR_INVALID_P4_INFO)
                                          << "P4Info " << resource_type
                                          << " name " << name_key
                                          << " is not unique";
      APPEND_STATUS_IF_ERROR(status, insert_name_status);
    }
  }

  return status;
}

::util::Status P4InfoManager::VerifyTableXrefs() {
  ::util::Status status = ::util::OkStatus();

  // This pass through all the P4 tables confirms that:
  //  - Every match field refers to a validly defined header field.
  //  - Every action ID refers to a validly defined action.
  for (const auto& table : p4_info_.tables()) {
    if (!table_map_.FindByID(table.preamble().id()).ok())
      continue;  // Skips tables that were invalid in the initial pass.

    for (const auto& action_ref : table.action_refs()) {
      if (!action_map_.FindByID(action_ref.id()).ok()) {
        ::util::Status action_xref_status =
            MAKE_ERROR(ERR_INVALID_P4_INFO)
            << "P4Info Table " << table.preamble().name() << " refers to an "
            << "invalid " << action_map_.resource_type() << " with ID "
            << PrintP4ObjectID(action_ref.id());
        APPEND_STATUS_IF_ERROR(status, action_xref_status);
      }
    }
  }

  return status;
}

::util::Status P4InfoManager::VerifyID(
    const ::p4::config::v1::Preamble& preamble,
    const std::string& resource_type) {
  if (preamble.id() == 0) {
    return MAKE_ERROR(ERR_INVALID_P4_INFO)
           << "P4Info " << resource_type
           << " requires a non-zero ID in preamble";
  }
  return ::util::OkStatus();
}

::util::Status P4InfoManager::VerifyName(
    const ::p4::config::v1::Preamble& preamble,
    const std::string& resource_type) {
  if (preamble.name().empty()) {
    return MAKE_ERROR(ERR_INVALID_P4_INFO)
           << "P4Info " << resource_type
           << " requires a non-empty name in preamble";
  }
  return ::util::OkStatus();
}

}  // namespace hal
}  // namespace stratum
