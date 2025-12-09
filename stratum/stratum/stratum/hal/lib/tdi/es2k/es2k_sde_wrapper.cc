// Copyright 2019-present Barefoot Networks, Inc.
// Copyright 2022-2023 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

// ES2K-specific SDE wrapper.

#include "stratum/hal/lib/tdi/es2k/es2k_sde_wrapper.h"

#include <stdio.h>
#include <string.h>

#include <algorithm>
#include <memory>
#include <ostream>
#include <string>
#include <utility>
#include <vector>

#include "absl/memory/memory.h"
#include "absl/strings/str_cat.h"
#include "absl/synchronization/mutex.h"
#include "stratum/glue/integral_types.h"
#include "stratum/glue/logging.h"
#include "stratum/glue/status/status.h"
#include "stratum/glue/status/status_macros.h"
#include "stratum/glue/status/statusor.h"
#include "stratum/hal/lib/tdi/tdi.pb.h"
#include "stratum/hal/lib/tdi/tdi_sde_common.h"
#include "stratum/hal/lib/tdi/tdi_sde_helpers.h"
#include "stratum/hal/lib/tdi/tdi_status.h"
#include "stratum/lib/utils.h"
#include "tdi_rt/tdi_rt_defs.h"

extern "C" {
#include "ipu_p4d/lib/ipu_p4d_lib_init.h"
#include "ipu_pal/dev_intf.h"
}

namespace stratum {
namespace hal {
namespace tdi {

using namespace stratum::hal::tdi::helpers;

Es2kSdeWrapper* Es2kSdeWrapper::singleton_ = nullptr;
ABSL_CONST_INIT absl::Mutex Es2kSdeWrapper::init_lock_(absl::kConstInit);

//------------------------------------------------------------------------------
// Constructor
//------------------------------------------------------------------------------
Es2kSdeWrapper::Es2kSdeWrapper() : device_to_packet_rx_writer_() {}

//------------------------------------------------------------------------------
// CreateSingleton
//------------------------------------------------------------------------------
Es2kSdeWrapper* Es2kSdeWrapper::CreateSingleton() {
  absl::WriterMutexLock l(&init_lock_);
  if (!singleton_) {
    singleton_ = new Es2kSdeWrapper();
  }

  return singleton_;
}

Es2kSdeWrapper* Es2kSdeWrapper::GetSingleton() {
  absl::ReaderMutexLock l(&init_lock_);
  return singleton_;
}

//------------------------------------------------------------------------------
// Identification
//------------------------------------------------------------------------------
namespace {

std::string GetBfChipFamilyAndType(int device) { return "UNKNOWN"; }

std::string GetBfChipRevision(int device) { return "UNKNOWN"; }

std::string GetBfChipId(int device) {
  uint64 chip_id = 0;
  return absl::StrCat("0x", absl::Hex(chip_id));
}

}  // namespace

::util::StatusOr<bool> Es2kSdeWrapper::IsSoftwareModel(int device) {
  bool is_sw_model = true;
  return is_sw_model;
}

std::string Es2kSdeWrapper::GetSdeVersion() const { return "1.0.0"; }

std::string Es2kSdeWrapper::GetChipType(int device) const {
  return absl::StrCat(GetBfChipFamilyAndType(device), ", revision ",
                      GetBfChipRevision(device), ", chip_id ",
                      GetBfChipId(device));
}

//------------------------------------------------------------------------------
// InitializeSde
//------------------------------------------------------------------------------
::util::Status Es2kSdeWrapper::InitializeSde(
    const std::string& sde_install_path, const std::string& sde_config_file,
    bool run_in_background) {
  RET_CHECK(sde_install_path != "") << "sde_install_path is required";
  RET_CHECK(sde_config_file != "") << "sde_config_file is required";

  // Parse bf_switchd arguments.
  auto switchd_main_ctx = absl::make_unique<ipu_p4d_context_t>();
  switchd_main_ctx->install_dir = strdup(sde_install_path.c_str());
  switchd_main_ctx->conf_file = strdup(sde_config_file.c_str());
  switchd_main_ctx->skip_p4 = true;
  if (run_in_background) {
    switchd_main_ctx->running_in_background = true;
  } else {
    switchd_main_ctx->shell_set_ucli = true;
  }

  RETURN_IF_TDI_ERROR(ipu_p4d_lib_init(switchd_main_ctx.get()))
      << "Error when starting switchd.";
  LOG(INFO) << "switchd started successfully";

  return ::util::OkStatus();
}

//------------------------------------------------------------------------------
// AddDevice
//------------------------------------------------------------------------------
::util::Status Es2kSdeWrapper::AddDevice(int dev_id,
                                         const TdiDeviceConfig& device_config) {
  const ::tdi::Device* device = nullptr;
  absl::WriterMutexLock l(&data_lock_);

  RET_CHECK(device_config.programs_size() > 0);

  tdi_id_mapper_.reset();

  RETURN_IF_TDI_ERROR(
      ipu_pal_device_warm_init_begin(dev_id, TDI_DEV_WARM_INIT_FAST_RECFG,
                                     /* upgrade_agents */ true));
  tdi_device_profile_t device_profile = {};

  // Commit new files to disk and build device profile for SDE to load.
  RETURN_IF_ERROR(RecursivelyCreateDir(FLAGS_tdi_sde_config_dir));
  // Need to extend the lifetime of the path strings until the SDE read them.
  std::vector<std::unique_ptr<std::string>> path_strings;
  device_profile.num_p4_programs = device_config.programs_size();
  for (int i = 0; i < device_config.programs_size(); ++i) {
    const auto& program = device_config.programs(i);
    const std::string program_path =
        absl::StrCat(FLAGS_tdi_sde_config_dir, "/", program.name());
    auto tdirt_path = absl::make_unique<std::string>(
        absl::StrCat(program_path, "/bfrt.json"));
    RETURN_IF_ERROR(RecursivelyCreateDir(program_path));
    RETURN_IF_ERROR(WriteStringToFile(program.bfrt(), *tdirt_path));

    tdi_p4_program_t* p4_program = &device_profile.p4_programs[i];
    ::snprintf(p4_program->prog_name, _PI_UPDATE_MAX_NAME_SIZE, "%s",
               program.name().c_str());
    p4_program->tdi_json_file = &(*tdirt_path)[0];
    p4_program->num_p4_pipelines = program.pipelines_size();
    path_strings.emplace_back(std::move(tdirt_path));
    RET_CHECK(program.pipelines_size() > 0);
    for (int j = 0; j < program.pipelines_size(); ++j) {
      const auto& pipeline = program.pipelines(j);
      const std::string pipeline_path =
          absl::StrCat(program_path, "/", pipeline.name());
      auto context_path = absl::make_unique<std::string>(
          absl::StrCat(pipeline_path, "/context.json"));
      auto config_path = absl::make_unique<std::string>(
          absl::StrCat(pipeline_path, "/tofino.bin"));
      RETURN_IF_ERROR(RecursivelyCreateDir(pipeline_path));
      RETURN_IF_ERROR(WriteStringToFile(pipeline.context(), *context_path));
      RETURN_IF_ERROR(WriteStringToFile(pipeline.config(), *config_path));

      tdi_p4_pipeline_t* pipeline_profile = &p4_program->p4_pipelines[j];
      ::snprintf(pipeline_profile->p4_pipeline_name, _PI_UPDATE_MAX_NAME_SIZE,
                 "%s", pipeline.name().c_str());
      pipeline_profile->cfg_file = &(*config_path)[0];
      pipeline_profile->runtime_context_file = &(*context_path)[0];
      path_strings.emplace_back(std::move(config_path));
      path_strings.emplace_back(std::move(context_path));

      RET_CHECK(pipeline.scope_size() <= MAX_P4_PIPELINES);
      pipeline_profile->num_pipes_in_scope = pipeline.scope_size();
      for (int p = 0; p < pipeline.scope_size(); ++p) {
        const auto& scope = pipeline.scope(p);
        pipeline_profile->pipe_scope[p] = scope;
      }
    }
  }

  // This call re-initializes most SDE components.
  RETURN_IF_TDI_ERROR(ipu_pal_device_add(dev_id, &device_profile));
  RETURN_IF_TDI_ERROR(ipu_pal_device_warm_init_end(dev_id));

  // Set SDE log levels for modules of interest.
  // TODO(max): create story around SDE logs. How to get them into glog? What
  // levels to enable for which modules?
  RET_CHECK(
      bf_sys_log_level_set(BF_MOD_BFRT, BF_LOG_DEST_STDOUT, BF_LOG_WARN) == 0);
  RET_CHECK(bf_sys_log_level_set(BF_MOD_PKT, BF_LOG_DEST_STDOUT, BF_LOG_WARN) ==
            0);
  RET_CHECK(
      bf_sys_log_level_set(BF_MOD_PIPE, BF_LOG_DEST_STDOUT, BF_LOG_WARN) == 0);
  if (VLOG_IS_ON(2)) {
    RET_CHECK(bf_sys_log_level_set(BF_MOD_PIPE, BF_LOG_DEST_STDOUT,
                                   BF_LOG_WARN) == 0);
  }

  ::tdi::DevMgr::getInstance().deviceGet(dev_id, &device);
  RETURN_IF_TDI_ERROR(
      device->tdiInfoGet(device_config.programs(0).name(), &tdi_info_));

  // FIXME: if all we ever do is create and push, this could be one call.
  tdi_id_mapper_ = TdiIdMapper::CreateInstance();
  RETURN_IF_ERROR(
      tdi_id_mapper_->PushForwardingPipelineConfig(device_config, tdi_info_));

  return ::util::OkStatus();
}

//------------------------------------------------------------------------------
// IPsec notification
//------------------------------------------------------------------------------
::util::Status Es2kSdeWrapper::InitNotificationTableWithCallback(
    int dev_id, std::shared_ptr<TdiSdeInterface::SessionInterface> session,
    const std::string& table_name,
    void (*ipsec_notif_cb)(uint32_t dev_id, uint32_t ipsec_sa_spi,
                           bool soft_lifetime_expire, uint8_t ipsec_sa_protocol,
                           char* ipsec_sa_dest_address, bool ipv4, void* cke),
    void* cookie) const {
  if (!tdi_info_) {
    return MAKE_ERROR(ERR_INTERNAL) << "Unable to initialize notification "
                                       "table due to TDI internal error";
  }

  auto real_session = std::dynamic_pointer_cast<Session>(session);
  RET_CHECK(real_session);

  const ::tdi::Table* notifTable;
  RETURN_IF_TDI_ERROR(tdi_info_->tableFromNameGet(table_name, &notifTable));

  // tdi_attributes_allocate
  std::unique_ptr<::tdi::TableAttributes> attributes_field;
  ::tdi::TableAttributes* attributes_object;
  RETURN_IF_TDI_ERROR(notifTable->attributeAllocate(
      (tdi_attributes_type_e)TDI_RT_ATTRIBUTES_TYPE_IPSEC_SADB_EXPIRE_NOTIF,
      &attributes_field));
  attributes_object = attributes_field.get();

  // tdi_attributes_set_values
  const uint64_t enable = 1;
  RETURN_IF_TDI_ERROR(attributes_object->setValue(
      (tdi_attributes_field_type_e)
          TDI_RT_ATTRIBUTES_IPSEC_SADB_EXPIRE_TABLE_FIELD_TYPE_ENABLE,
      enable));

  RETURN_IF_TDI_ERROR(attributes_object->setValue(
      (tdi_attributes_field_type_e)
          TDI_RT_ATTRIBUTES_IPSEC_SADB_EXPIRE_TABLE_FIELD_TYPE_CALLBACK_C,
      (uint64_t)ipsec_notif_cb));

  RETURN_IF_TDI_ERROR(attributes_object->setValue(
      (tdi_attributes_field_type_e)
          TDI_RT_ATTRIBUTES_IPSEC_SADB_EXPIRE_TABLE_FIELD_TYPE_COOKIE,
      (uint64_t)cookie));

  // target & flag create
  const ::tdi::Device* device = nullptr;
  ::tdi::DevMgr::getInstance().deviceGet(dev_id, &device);
  std::unique_ptr<::tdi::Target> target;
  device->createTarget(&target);

  const auto flags = ::tdi::Flags(0);
  RETURN_IF_TDI_ERROR(notifTable->tableAttributesSet(
      *real_session->tdi_session_, *target, flags, *attributes_object));

  return ::util::OkStatus();
}

//------------------------------------------------------------------------------
// Virtual Port State change notification
//------------------------------------------------------------------------------
::util::Status Es2kSdeWrapper::InitNotificationTableWithCallback(
    int dev_id, std::shared_ptr<TdiSdeInterface::SessionInterface> session,
    const std::string& table_name,
    void (*vport_state_notif_cb)(uint32_t dev_id, uint32_t glort_id,
                                 uint8_t state, void* cke),
    void* cookie) const {
  if (!tdi_info_) {
    return MAKE_ERROR(ERR_INTERNAL) << "Unable to initialize notification "
                                       "table due to TDI internal error";
  }

  auto real_session = std::dynamic_pointer_cast<Session>(session);
  RET_CHECK(real_session);

  const ::tdi::Table* notifTable;
  RETURN_IF_TDI_ERROR(tdi_info_->tableFromNameGet(table_name, &notifTable));

  // tdi_attributes_allocate
  std::unique_ptr<::tdi::TableAttributes> attributes_field;
  ::tdi::TableAttributes* attributes_object;
  RETURN_IF_TDI_ERROR(notifTable->attributeAllocate(
      (tdi_attributes_type_e)TDI_RT_ATTRIBUTES_TYPE_VPORT_STATUS_NOTIF,
      &attributes_field));
  attributes_object = attributes_field.get();

  // tdi_attributes_set_values
  const uint64_t enable = 1;
  RETURN_IF_TDI_ERROR(attributes_object->setValue(
      (tdi_attributes_field_type_e)
          TDI_RT_ATTRIBUTES_VPORT_STATUS_TABLE_FIELD_TYPE_ENABLE,
      enable));

  RETURN_IF_TDI_ERROR(attributes_object->setValue(
      (tdi_attributes_field_type_e)
          TDI_RT_ATTRIBUTES_VPORT_STATUS_TABLE_FIELD_TYPE_CALLBACK_C,
      (uint64_t)vport_state_notif_cb));

  RETURN_IF_TDI_ERROR(attributes_object->setValue(
      (tdi_attributes_field_type_e)
          TDI_RT_ATTRIBUTES_VPORT_STATUS_TABLE_FIELD_TYPE_COOKIE,
      (uint64_t)cookie));

  // target & flag create
  const ::tdi::Device* device = nullptr;
  ::tdi::DevMgr::getInstance().deviceGet(dev_id, &device);
  std::unique_ptr<::tdi::Target> target;
  device->createTarget(&target);

  const auto flags = ::tdi::Flags(0);
  RETURN_IF_TDI_ERROR(notifTable->tableAttributesSet(
      *real_session->tdi_session_, *target, flags, *attributes_object));

  return ::util::OkStatus();
}

}  // namespace tdi
}  // namespace hal
}  // namespace stratum
