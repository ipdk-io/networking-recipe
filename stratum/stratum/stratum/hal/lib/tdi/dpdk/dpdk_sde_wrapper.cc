// Copyright 2019-present Barefoot Networks, Inc.
// Copyright 2022-2023 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

// DPDK-specific SDE wrapper.

#include "stratum/hal/lib/tdi/dpdk/dpdk_sde_wrapper.h"

#include <stdio.h>
#include <string.h>

#include <algorithm>
#include <memory>
#include <ostream>
#include <string>
#include <utility>
#include <vector>

#include "absl/synchronization/mutex.h"
#include "stratum/glue/logging.h"
#include "stratum/glue/status/status.h"
#include "stratum/glue/status/statusor.h"
#include "stratum/hal/lib/tdi/tdi_sde_common.h"
#include "stratum/hal/lib/tdi/tdi_sde_helpers.h"
#include "stratum/hal/lib/tdi/tdi_status.h"
#include "stratum/lib/utils.h"

extern "C" {
#include "bf_pal/dev_intf.h"
#include "bf_switchd/lib/bf_switchd_lib_init.h"
}

namespace stratum {
namespace hal {
namespace tdi {

using namespace stratum::hal::tdi::helpers;

DpdkSdeWrapper* DpdkSdeWrapper::singleton_ = nullptr;
ABSL_CONST_INIT absl::Mutex DpdkSdeWrapper::init_lock_(absl::kConstInit);

//------------------------------------------------------------------------------
// Constructor
//------------------------------------------------------------------------------
DpdkSdeWrapper::DpdkSdeWrapper() {}

//------------------------------------------------------------------------------
// CreateSingleton
//------------------------------------------------------------------------------
DpdkSdeWrapper* DpdkSdeWrapper::CreateSingleton() {
  absl::WriterMutexLock l(&init_lock_);
  if (!singleton_) {
    singleton_ = new DpdkSdeWrapper();
  }

  return singleton_;
}

DpdkSdeWrapper* DpdkSdeWrapper::GetSingleton() {
  absl::ReaderMutexLock l(&init_lock_);
  return singleton_;
}

//------------------------------------------------------------------------------
// Identification
//------------------------------------------------------------------------------
::util::StatusOr<bool> DpdkSdeWrapper::IsSoftwareModel(int device) {
  return true;
}

std::string DpdkSdeWrapper::GetChipType(int device) const { return "DPDK"; }

std::string DpdkSdeWrapper::GetSdeVersion() const {
  // TODO tdi version
  return "1.0.0";
}

//------------------------------------------------------------------------------
// InitializeSde
//------------------------------------------------------------------------------
::util::Status DpdkSdeWrapper::InitializeSde(
    const std::string& sde_install_path, const std::string& sde_config_file,
    bool run_in_background) {
  RET_CHECK(sde_install_path != "") << "sde_install_path is required";
  RET_CHECK(sde_config_file != "") << "sde_config_file is required";

  // Parse bf_switchd arguments.
  auto switchd_main_ctx = absl::make_unique<bf_switchd_context_t>();
  switchd_main_ctx->install_dir = strdup(sde_install_path.c_str());
  switchd_main_ctx->conf_file = strdup(sde_config_file.c_str());
  switchd_main_ctx->skip_p4 = true;
  if (run_in_background) {
    switchd_main_ctx->running_in_background = true;
  } else {
    switchd_main_ctx->shell_set_ucli = true;
  }

  // Determine if kernel mode packet driver is loaded.
  std::string bf_sysfs_fname;
  {
    char buf[128] = {};
    RETURN_IF_TDI_ERROR(switch_pci_sysfs_str_get(buf, sizeof(buf)));
    bf_sysfs_fname = buf;
  }
  absl::StrAppend(&bf_sysfs_fname, "/dev_add");
  LOG(INFO) << "bf_sysfs_fname: " << bf_sysfs_fname;
  if (PathExists(bf_sysfs_fname)) {
    // Override previous parsing if bf_kpkt KLM was loaded.
    LOG(INFO)
        << "kernel mode packet driver present, forcing kernel_pkt option!";
  }

  RETURN_IF_TDI_ERROR(bf_switchd_lib_init(switchd_main_ctx.get()))
      << "Error when starting switchd.";
  LOG(INFO) << "switchd started successfully";

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

  return ::util::OkStatus();
}

//------------------------------------------------------------------------------
// AddDevice
//------------------------------------------------------------------------------
::util::Status DpdkSdeWrapper::AddDevice(int dev_id,
                                         const TdiDeviceConfig& device_config) {
  const ::tdi::Device* device = nullptr;
  absl::WriterMutexLock l(&data_lock_);

  RET_CHECK(device_config.programs_size() > 0);

  tdi_id_mapper_.reset();

  RETURN_IF_TDI_ERROR(bf_pal_device_warm_init_begin(dev_id,
                                                    BF_DEV_WARM_INIT_FAST_RECFG,
                                                    /* upgrade_agents */ true));
  bf_device_profile_t device_profile = {};

  // Commit new files to disk and build device profile for SDE to load.
  RETURN_IF_ERROR(RecursivelyCreateDir(FLAGS_tdi_sde_config_dir));
  // Need to extend the lifetime of the path strings until the SDE reads them.
  std::vector<std::unique_ptr<std::string>> path_strings;
  device_profile.num_p4_programs = device_config.programs_size();
  for (int i = 0; i < device_config.programs_size(); ++i) {
    const auto& program = device_config.programs(i);
    const std::string program_path =
        absl::StrCat(FLAGS_tdi_sde_config_dir, "/", program.name());
    auto tdi_path = absl::make_unique<std::string>(
        absl::StrCat(program_path, "/bfrt.json"));
    RETURN_IF_ERROR(RecursivelyCreateDir(program_path));
    RETURN_IF_ERROR(WriteStringToFile(program.bfrt(), *tdi_path));

    bf_p4_program_t* p4_program = &device_profile.p4_programs[i];
    ::snprintf(p4_program->prog_name, _PI_UPDATE_MAX_NAME_SIZE, "%s",
               program.name().c_str());
    p4_program->bfrt_json_file = &(*tdi_path)[0];
    p4_program->num_p4_pipelines = program.pipelines_size();
    path_strings.emplace_back(std::move(tdi_path));
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

      bf_p4_pipeline_t* pipeline_profile = &p4_program->p4_pipelines[j];
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
  RETURN_IF_TDI_ERROR(bf_pal_device_add(dev_id, &device_profile));
  RETURN_IF_TDI_ERROR(bf_pal_device_warm_init_end(dev_id));

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
// PacketModMeter
//------------------------------------------------------------------------------
::util::Status TableData::SetPktModMeterConfig(
    const TdiPktModMeterConfig& cfg) {
  return MAKE_ERROR(ERR_OPER_NOT_SUPPORTED) << "PacketModMeter not supported";
}

::util::Status TableData::GetPktModMeterConfig(
    TdiPktModMeterConfig& cfg) const {
  return MAKE_ERROR(ERR_OPER_NOT_SUPPORTED) << "PacketModMeter not supported";
}

}  // namespace tdi
}  // namespace hal
}  // namespace stratum
