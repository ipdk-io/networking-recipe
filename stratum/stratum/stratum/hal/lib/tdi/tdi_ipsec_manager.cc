// Copyright 2022-2024 Intel Corporation.
// SPDX-License-Identifier: Apache-2.0

#include "stratum/hal/lib/tdi/tdi_ipsec_manager.h"

#include <map>
#include <memory>
#include <set>
#include <utility>

#include "stratum/glue/thread_annotations_compat.h"
#include "absl/memory/memory.h"
#include "absl/synchronization/mutex.h"
#include "absl/time/time.h"
#include "absl/types/optional.h"
#include "stratum/glue/integral_types.h"
#include "stratum/glue/logging.h"
#include "stratum/hal/lib/common/constants.h"
#include "stratum/hal/lib/common/utils.h"
#include "stratum/hal/lib/common/writer_interface.h"
#include "stratum/lib/channel/channel.h"
#include "stratum/lib/constants.h"
#include "stratum/lib/macros.h"
#include "stratum/lib/utils.h"

#define IPSEC_CONFIG_SADB_TABLE_NAME \
  "ipsec-offload.ipsec-offload.sad.sad-entry.ipsec-sa-config"
#define IPSEC_FETCH_SPI_TABLE_NAME "ipsec-offload.ipsec-offload.ipsec-spi"
#define IPSEC_NOTIFICATION_TABLE_NAME "ipsec-offload"
#define IPSEC_STATE_SADB_TABLE_NAME \
  "ipsec-offload.ipsec-offload.sad.sad-entry.ipsec-sa-state"

namespace stratum {
namespace hal {
namespace tdi {

ABSL_CONST_INIT absl::Mutex _ipsec_mgr_lock(absl::kConstInit);

/* #### C function callback #### */
static void ipsec_notification_callback(uint32_t dev_id, uint32_t ipsec_sa_spi,
                                        bool soft_lifetime_expire,
                                        uint8_t ipsec_sa_protocol,
                                        char* ipsec_sa_dest_address, bool ipv4,
                                        void* cookie) {
  auto ipsec_mgr_hdl = reinterpret_cast<TdiIpsecManager*>(cookie);
  ipsec_mgr_hdl->SendSADExpireNotificationEvent(
      dev_id, ipsec_sa_spi, soft_lifetime_expire, ipsec_sa_protocol,
      ipsec_sa_dest_address, ipv4);
}

TdiIpsecManager::TdiIpsecManager(
    TdiSdeInterface* tdi_sde_interface,
    TdiFixedFunctionManager* tdi_fixed_function_manager)
    : gnmi_event_writer_(nullptr),
      tdi_sde_interface_(ABSL_DIE_IF_NULL(tdi_sde_interface)),
      tdi_fixed_function_manager_(ABSL_DIE_IF_NULL(tdi_fixed_function_manager)),
      notif_initialized_(false) {}

TdiIpsecManager::TdiIpsecManager()
    : gnmi_event_writer_(nullptr),
      tdi_sde_interface_(nullptr),
      tdi_fixed_function_manager_(nullptr),
      notif_initialized_(false) {}

TdiIpsecManager::~TdiIpsecManager() = default;

::util::Status TdiIpsecManager::InitializeNotificationCallback() {
  auto status = tdi_fixed_function_manager_->InitNotificationTableWithCallback(
      IPSEC_NOTIFICATION_TABLE_NAME, &ipsec_notification_callback, this);

  if (!status.ok()) {
    LOG(ERROR) << "Failed to register IPsec notification callback";
  }
  return status;
}

::util::Status TdiIpsecManager::GetSpiData(uint32& fetched_spi) {
  // TODO (5abeel): Initializing the notification callback on FetchSPI because
  // TDI layer is not initialized until 'set-pipe' is completed by user via P4RT
  if (!notif_initialized_) {
    auto status = InitializeNotificationCallback();
    if (status.ok()) {
      notif_initialized_ = true;
    }
  }

  ASSIGN_OR_RETURN(auto session, tdi_sde_interface_->CreateSession());
  auto status = tdi_fixed_function_manager_->FetchSpi(
      session, IPSEC_FETCH_SPI_TABLE_NAME, &fetched_spi);
  if (!status.ok()) {
    return MAKE_ERROR(ERR_AT_LEAST_ONE_OPER_FAILED)
           << "One or more read operations failed.";
  }
  return ::util::OkStatus();
}

::util::Status TdiIpsecManager::WriteConfigSADBEntry(
    const IPsecSadbConfigOp op_type, IPsecSADBConfig& msg) {
  // TODO (5abeel): Initializing the notification callback on FetchSPI because
  // TDI layer is not initialized until 'set-pipe' is completed by user via P4RT
  if (!notif_initialized_) {
    auto status = InitializeNotificationCallback();
    if (status.ok()) {
      notif_initialized_ = true;
    }
  }

  if (op_type == IPSEC_SADB_CONFIG_OP_ADD_ENTRY) {
    // Convert encryption key from hex to ASCII
    std::string hex_encoded = msg.esp_payload().encryption().key();
    RETURN_IF_ERROR(ValidateYangHexString(hex_encoded));

    std::string ascii_encoded = ConvertEncryptionKeyEncoding(hex_encoded);
    msg.mutable_esp_payload()->mutable_encryption()->set_key(ascii_encoded);
  }

  ASSIGN_OR_RETURN(auto session, tdi_sde_interface_->CreateSession());
  auto status = tdi_fixed_function_manager_->WriteSadbEntry(
      session, IPSEC_CONFIG_SADB_TABLE_NAME, op_type, msg);
  if (!status.ok()) {
    return MAKE_ERROR(ERR_AT_LEAST_ONE_OPER_FAILED)
           << "One or more write operations failed. "
           << "offload-id=" << msg.offload_id()
           << ", direction=" << msg.direction() << ", op type=" << op_type
           << ", table_name=" << IPSEC_CONFIG_SADB_TABLE_NAME;
  }
  return ::util::OkStatus();
}

void TdiIpsecManager::SendSADExpireNotificationEvent(
    uint32_t dev_id, uint32_t ipsec_sa_spi, bool soft_lifetime_expire,
    uint8_t ipsec_sa_protocol, char* ipsec_sa_dest_address, bool ipv4) {
  absl::ReaderMutexLock l(&gnmi_event_lock_);
  if (!gnmi_event_writer_) return;
  // Allocate and initialize an IPsecNotificationEvent event and pass it to
  // the gNMI publisher using the gNMI event notification channel.
  // The GnmiEventPtr is a smart pointer (shared_ptr<>) and it takes care of
  // the memory allocated to this event object once the event is handled by
  // the GnmiPublisher.
  if (!gnmi_event_writer_->Write(GnmiEventPtr(new IPsecNotificationEvent(
          absl::ToUnixNanos(absl::UnixEpoch()), dev_id, ipsec_sa_spi,
          soft_lifetime_expire, ipsec_sa_protocol, ipsec_sa_dest_address,
          ipv4)))) {
    // Remove WriterInterface if it is no longer operational.
    gnmi_event_writer_.reset();
  }
}

std::string TdiIpsecManager::ConvertEncryptionKeyEncoding(std::string hex) {
  std::string ascii = "";
  for (size_t i = 0; i < hex.length(); i += 3) {
    std::string part = hex.substr(i, 2);
    // change it into base 16 and typecast to byte char
    char ch = (char)(int)strtol(part.data(), nullptr, 16);
    ascii.push_back(ch);
  }
  return ascii;
}

std::unique_ptr<TdiIpsecManager> TdiIpsecManager::CreateInstance(
    TdiSdeInterface* tdi_sde_interface,
    TdiFixedFunctionManager* tdi_fixed_function_manager) {
  return absl::WrapUnique(
      new TdiIpsecManager(tdi_sde_interface, tdi_fixed_function_manager));
}

}  // namespace tdi
}  // namespace hal
}  // namespace stratum
