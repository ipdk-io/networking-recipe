// Copyright 2018-2019 Barefoot Networks, Inc.
// Copyright 2020-present Open Networking Foundation
// Copyright 2022-2025 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

#include <exception>
#include <map>
#include <memory>
#include <ostream>
#include <string>

#include "absl/synchronization/notification.h"
#include "gflags/gflags.h"
#include "stratum/glue/init_google.h"
#include "stratum/glue/logging.h"
#include "stratum/glue/status/status_macros.h"
#include "stratum/glue/status/statusor.h"
#include "stratum/hal/bin/tdi/main.h"
#include "stratum/hal/lib/common/common.pb.h"
#include "stratum/hal/lib/common/target_options.h"
#include "stratum/hal/lib/tdi/es2k/es2k_chassis_manager.h"
#include "stratum/hal/lib/tdi/es2k/es2k_hal.h"
#include "stratum/hal/lib/tdi/es2k/es2k_node.h"
#include "stratum/hal/lib/tdi/es2k/es2k_port_manager.h"
#include "stratum/hal/lib/tdi/es2k/es2k_sde_wrapper.h"
#include "stratum/hal/lib/tdi/es2k/es2k_switch.h"
#include "stratum/hal/lib/tdi/es2k/es2k_virtual_port_manager.h"
#include "stratum/hal/lib/tdi/tdi_action_profile_manager.h"
#include "stratum/hal/lib/tdi/tdi_counter_manager.h"
#include "stratum/hal/lib/tdi/tdi_fixed_function_manager.h"
#include "stratum/hal/lib/tdi/tdi_ipsec_manager.h"
#include "stratum/hal/lib/tdi/tdi_packetio_manager.h"
#include "stratum/hal/lib/tdi/tdi_pre_manager.h"
#include "stratum/hal/lib/tdi/tdi_table_manager.h"
#include "stratum/lib/macros.h"
#include "stratum/lib/security/auth_policy_checker.h"
#include "stratum/lib/security/credentials_manager.h"

#define DEFAULT_CERTS_DIR "/usr/share/stratum/certs/"
#define DEFAULT_CONFIG_DIR "/usr/share/stratum/es2k/"
#define DEFAULT_LOG_DIR "/var/log/stratum/"

DEFINE_string(es2k_sde_install, "/usr",
              "Absolute path to the directory where the SDE is installed");
DEFINE_string(es2k_infrap4d_cfg, DEFAULT_CONFIG_DIR "es2k_skip_p4.conf",
              "Path to the infrap4d json config file");
DECLARE_string(chassis_config_file);

namespace stratum {
namespace hal {
namespace tdi {

static void SetDefault(const char* name, const char* value) {
  ::gflags::SetCommandLineOptionWithMode(name, value,
                                         ::gflags::SET_FLAGS_DEFAULT);
}

static void InitCommandLineFlags() {
  // Chassis config file
  SetDefault("chassis_config_file",
             DEFAULT_CONFIG_DIR "es2k_port_config.pb.txt");

  // Logging options
  SetDefault("log_dir", DEFAULT_LOG_DIR);
  SetDefault("logtostderr", "false");
  SetDefault("alsologtostderr", "false");

  // Certificate options
  SetDefault("ca_cert_file", DEFAULT_CERTS_DIR "ca.crt");
  SetDefault("server_key_file", DEFAULT_CERTS_DIR "stratum.key");
  SetDefault("server_cert_file", DEFAULT_CERTS_DIR "stratum.crt");
  SetDefault("client_key_file", DEFAULT_CERTS_DIR "client.key");
  SetDefault("client_cert_file", DEFAULT_CERTS_DIR "client.crt");

  // Client certificate verification requirement
  SetDefault("grpc_client_cert_req_type", "REQUIRE_CLIENT_CERT_AND_VERIFY");
}

void ParseCommandLine(int argc, char* argv[], bool remove_flags) {
  // Set our own default values
  InitCommandLineFlags();

  // Parse command-line flags
  gflags::ParseCommandLineFlags(&argc, &argv, remove_flags);
}

::util::Status Main(int argc, char* argv[]) {
  try {
    ParseCommandLine(argc, argv, true);
  } catch (const std::exception& e) {
    return MAKE_ERROR(ERR_INTERNAL) << e.what();
  }
  return Main(nullptr, nullptr);
}

::util::Status Main(absl::Notification* ready_sync,
                    absl::Notification* done_sync) {
  InitStratumLogging();

  // TODO(antonin): The SDE expects 0-based device ids, so we instantiate
  // components with "device_id" instead of "node_id".
  const int device_id = 0;
  const uint64 node_id = 0;
  const bool initialized = false;
  const bool es2k_infrap4d_background = true;

  auto sde_wrapper = Es2kSdeWrapper::CreateSingleton();

  RETURN_IF_ERROR(sde_wrapper->InitializeSde(FLAGS_es2k_sde_install,
                                             FLAGS_es2k_infrap4d_cfg,
                                             es2k_infrap4d_background));

  const OperationMode mode = OPERATION_MODE_STANDALONE;

  VLOG(1) << "SDE version: " << sde_wrapper->GetSdeVersion();
  VLOG(1) << "Switch SKU: " << sde_wrapper->GetChipType(device_id);

  auto table_manager =
      TdiTableManager::CreateInstance(mode, sde_wrapper, device_id);

  auto fixed_function_manager =
      TdiFixedFunctionManager::CreateInstance(mode, sde_wrapper, device_id);

  auto action_profile_manager =
      TdiActionProfileManager::CreateInstance(sde_wrapper, device_id);

  auto packetio_manager =
      TdiPacketioManager::CreateInstance(sde_wrapper, device_id);

  auto pre_manager = TdiPreManager::CreateInstance(sde_wrapper, device_id);

  auto counter_manager =
      TdiCounterManager::CreateInstance(sde_wrapper, device_id);

  auto es2k_node = Es2kNode::CreateInstance(
      table_manager.get(), action_profile_manager.get(), packetio_manager.get(),
      pre_manager.get(), counter_manager.get(), sde_wrapper, device_id,
      initialized, node_id);

  std::map<int, Es2kNode*> device_id_to_es2k_node = {
      {device_id, es2k_node.get()},
  };

  auto port_manager = Es2kPortManager::CreateSingleton();

  auto virtual_port_manager = Es2kVirtualPortManager::CreateSingleton();

  auto chassis_manager = Es2kChassisManager::CreateInstance(
      mode, port_manager, virtual_port_manager);

  // Es2kVirtualPortManager needs reference to SetTdiSdeInterface &
  // TdiFixedFunctionManager
  virtual_port_manager->SetTdiSdeInterface(sde_wrapper);
  virtual_port_manager->SetTdiFixedFunctionManager(
      fixed_function_manager.get());

  auto ipsec_manager = TdiIpsecManager::CreateInstance(
      sde_wrapper, fixed_function_manager.get());

  auto es2k_switch =
      Es2kSwitch::CreateInstance(chassis_manager.get(), ipsec_manager.get(),
                                 virtual_port_manager, device_id_to_es2k_node);

  auto auth_policy_checker = AuthPolicyChecker::CreateInstance();

  TargetOptions target_options;
  target_options.secureChassisConfig = true;

  // Create the 'Hal' class instance.
  auto* hal = Es2kHal::CreateSingleton(mode, es2k_switch.get(),
                                       auth_policy_checker.get(), ready_sync,
                                       done_sync, &target_options);
  RET_CHECK(hal) << "Failed to create the Stratum Hal instance.";

  // Set up P4 runtime servers.
  ::util::Status status = hal->Setup();
  if (!status.ok()) {
    LOG(ERROR)
        << "Error when setting up Stratum HAL (but we will continue running): "
        << status.error_message();
  }

  // Start serving RPCs.
  RETURN_IF_ERROR(hal->Run());  // blocking

  LOG(INFO) << "See you later!";
  return ::util::OkStatus();
}

}  // namespace tdi
}  // namespace hal
}  // namespace stratum
