// Copyright 2023,2025 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

#include "stratum/hal/lib/tdi/tdi_fixed_function_manager.h"

#include <string>

#include "absl/strings/match.h"
#include "absl/synchronization/notification.h"
#include "stratum/hal/lib/tdi/tdi_constants.h"
#include "stratum/hal/lib/tdi/utils.h"
#include "stratum/lib/utils.h"

extern "C" {
#include "openssl/crypto.h"
}

namespace stratum {
namespace hal {
namespace tdi {

class SadbConfigWrapper {
 public:
  explicit SadbConfigWrapper(IPsecSADBConfig& sadb_config)
      : config_(sadb_config) {}
  ~SadbConfigWrapper() {
    std::string* key = get_key_ptr();
    if (key) {
      OPENSSL_cleanse(&key[0], key->size());
    }
  }

 private:
  // Returns mutable pointer to key field.
  inline std::string* get_key_ptr() {
    if (auto payload = config_.mutable_esp_payload()) {
      if (auto encryption = payload->mutable_encryption()) {
        return encryption->mutable_key();
      }
    }
    return nullptr;
  }
  IPsecSADBConfig& config_;
};

TdiFixedFunctionManager::TdiFixedFunctionManager(
    OperationMode mode, TdiSdeInterface* tdi_sde_interface, int device)
    : mode_(mode),
      tdi_sde_interface_(ABSL_DIE_IF_NULL(tdi_sde_interface)),
      device_(device) {}

std::unique_ptr<TdiFixedFunctionManager>
TdiFixedFunctionManager::CreateInstance(OperationMode mode,
                                        TdiSdeInterface* tdi_sde_interface,
                                        int device) {
  return absl::WrapUnique(
      new TdiFixedFunctionManager(mode, tdi_sde_interface, device));
}

//////////////////////////////////
// IPsec fixed-function TDI calls
//////////////////////////////////

::util::Status TdiFixedFunctionManager::InitNotificationTableWithCallback(
    std::string table_name,
    void (*ipsec_notif_cb)(uint32_t, uint32_t, bool, uint8_t, char*, bool,
                           void*),
    void* cookie) {
  absl::ReaderMutexLock l(&lock_);

  ASSIGN_OR_RETURN(auto session, tdi_sde_interface_->CreateSession());

  return tdi_sde_interface_->InitNotificationTableWithCallback(
      device_, session, table_name, ipsec_notif_cb, cookie);
}

::util::Status TdiFixedFunctionManager::WriteSadbEntry(
    std::shared_ptr<TdiSdeInterface::SessionInterface> session,
    std::string table_name, const IPsecSadbConfigOp op_type,
    IPsecSADBConfig& sadb_config) {
  absl::ReaderMutexLock l(&lock_);

  ASSIGN_OR_RETURN(uint32 table_id, tdi_sde_interface_->GetTableId(table_name));
  ASSIGN_OR_RETURN(auto table_key,
                   tdi_sde_interface_->CreateTableKey(table_id));
  RETURN_IF_ERROR(BuildSadbTableKey(table_key.get(), sadb_config));
  ASSIGN_OR_RETURN(auto table_data,
                   tdi_sde_interface_->CreateTableData(table_id, 0));
  if (op_type == IPSEC_SADB_CONFIG_OP_ADD_ENTRY ||
      op_type == IPSEC_SADB_CONFIG_OP_MOD_ENTRY) {
    RETURN_IF_ERROR(BuildSadbTableData(table_data.get(), sadb_config));
  }
  switch (op_type) {
    case IPSEC_SADB_CONFIG_OP_ADD_ENTRY:
      RETURN_IF_ERROR(tdi_sde_interface_->InsertTableEntry(
          device_, session, table_id, table_key.get(), table_data.get()));
      break;
    case IPSEC_SADB_CONFIG_OP_DEL_ENTRY:
      RETURN_IF_ERROR(tdi_sde_interface_->DeleteTableEntry(
          device_, session, table_id, table_key.get()));
      break;
    default:
      return MAKE_ERROR(ERR_INTERNAL) << "Unsupported update type: " << op_type
                                      << " for IPSEC SADB table.";
  }
  return ::util::OkStatus();
}

::util::Status TdiFixedFunctionManager::FetchSpi(
    std::shared_ptr<TdiSdeInterface::SessionInterface> session,
    std::string table_name, uint32* spi) {
  absl::ReaderMutexLock l(&lock_);

  uint64 spi_val;
  ASSIGN_OR_RETURN(uint32 table_id, tdi_sde_interface_->GetTableId(table_name));
  ASSIGN_OR_RETURN(auto table_data,
                   tdi_sde_interface_->CreateTableData(table_id, 0));
  RETURN_IF_ERROR(tdi_sde_interface_->GetDefaultTableEntry(
      device_, session, table_id, table_data.get()));
  RETURN_IF_ERROR(table_data->GetParam(kIpsecFetchSpi, &spi_val));
  *spi = (uint32)spi_val;
  return ::util::OkStatus();
}

::util::Status TdiFixedFunctionManager::BuildSadbTableKey(
    TdiSdeInterface::TableKeyInterface* table_key,
    IPsecSADBConfig& sadb_config) {
  RET_CHECK(table_key);

  RETURN_IF_ERROR(
      table_key->SetExact(kIpsecSadbOffloadId, sadb_config.offload_id()));
  RETURN_IF_ERROR(
      table_key->SetExact(kIpsecSadbDir, ((uint8)sadb_config.direction())));
  return ::util::OkStatus();
}

::util::Status TdiFixedFunctionManager::BuildSadbTableData(
    TdiSdeInterface::TableDataInterface* table_data,
    IPsecSADBConfig& sadb_config) {
  SadbConfigWrapper sanitize_on_exit(sadb_config);

  RET_CHECK(table_data);

  RETURN_IF_ERROR(table_data->SetParam(kIpsecSadbReqId, sadb_config.req_id()));
  RETURN_IF_ERROR(table_data->SetParam(kIpsecSadbSpi, sadb_config.spi()));
  RETURN_IF_ERROR(table_data->SetParam(kIpsecSadbSeqNum,
                                       ((uint8)sadb_config.ext_seq_num())));
  RETURN_IF_ERROR(table_data->SetParam(kIpsecSadbReplayWindow,
                                       sadb_config.anti_replay_window_size()));
  RETURN_IF_ERROR(table_data->SetParam(
      kIpsecSadbProtoParams, ((uint8)sadb_config.protocol_parameters())));
  RETURN_IF_ERROR(
      table_data->SetParam(kIpsecSadbMode, ((uint8)sadb_config.mode())));
  if (sadb_config.has_esp_payload()) {
    RETURN_IF_ERROR(table_data->SetParam(kIpsecSadbEspAlgo,
                                         ((uint16)sadb_config.esp_payload()
                                              .encryption()
                                              .encryption_algorithm())));
    RETURN_IF_ERROR(table_data->SetParam(
        kIpsecSadbEspKey, sadb_config.esp_payload().encryption().key()));
    RETURN_IF_ERROR(table_data->SetParam(
        kIpsecSadbEspKeylen,
        ((uint8)sadb_config.esp_payload().encryption().key_len())));
  }
  if (sadb_config.has_sa_hard_lifetime()) {
    RETURN_IF_ERROR(table_data->SetParam(
        kIpsecSaLtHard, sadb_config.sa_hard_lifetime().bytes()));
  }
  if (sadb_config.has_sa_soft_lifetime()) {
    RETURN_IF_ERROR(table_data->SetParam(
        kIpsecSaLtSoft, sadb_config.sa_soft_lifetime().bytes()));
  }
  return ::util::OkStatus();
}

//////////////////////////////////////////
// Virtual-ports fixed-function TDI calls
//////////////////////////////////////////
::util::Status TdiFixedFunctionManager::InitNotificationTableWithCallback(
    std::string table_name,
    void (*vport_state_notif_cb)(uint32_t, uint32_t, uint8_t, void*),
    void* cookie) {
  absl::ReaderMutexLock l(&lock_);

  ASSIGN_OR_RETURN(auto session, tdi_sde_interface_->CreateSession());

  return tdi_sde_interface_->InitNotificationTableWithCallback(
      device_, session, table_name, vport_state_notif_cb, cookie);
}

::util::Status TdiFixedFunctionManager::FetchVportTableData(
    std::shared_ptr<TdiSdeInterface::SessionInterface> session,
    std::string table_name, uint32 global_resource_id, const char* param_name,
    uint64* data) {
  absl::ReaderMutexLock l(&lock_);

  ASSIGN_OR_RETURN(uint32 table_id, tdi_sde_interface_->GetTableId(table_name));
  ASSIGN_OR_RETURN(auto table_key,
                   tdi_sde_interface_->CreateTableKey(table_id));
  RETURN_IF_ERROR(table_key->SetExact(kGlobalResourceId, global_resource_id));
  ASSIGN_OR_RETURN(auto table_data,
                   tdi_sde_interface_->CreateTableData(table_id, 0));
  RETURN_IF_ERROR(tdi_sde_interface_->GetTableEntry(
      device_, session, table_id, table_key.get(), table_data.get()));
  RETURN_IF_ERROR(table_data->GetParam(std::string(param_name), data));

  return ::util::OkStatus();
}

}  // namespace tdi
}  // namespace hal
}  // namespace stratum
