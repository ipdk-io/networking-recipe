// Copyright 2019-present Barefoot Networks, Inc.
// Copyright 2022-2024 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

#ifndef STRATUM_HAL_LIB_TDI_ES2K_SDE_WRAPPER_H_
#define STRATUM_HAL_LIB_TDI_ES2K_SDE_WRAPPER_H_

#include <string>

#include "absl/synchronization/mutex.h"
#include "ipu_types/ipu_types.h"
#include "stratum/hal/lib/tdi/tdi.pb.h"
#include "stratum/hal/lib/tdi/tdi_sde_wrapper.h"

namespace stratum {
namespace hal {
namespace tdi {

class Es2kSdeWrapper : public TdiSdeWrapper {
 public:
  ::util::StatusOr<bool> IsSoftwareModel(int device) override;
  std::string GetChipType(int device) const override;
  std::string GetSdeVersion() const override;

  ::util::Status InitializeSde(const std::string& sde_install_path,
                               const std::string& sde_config_file,
                               bool run_in_background) override;
  ::util::Status AddDevice(int device,
                           const TdiDeviceConfig& device_config) override;

  ::util::Status WritePktModMeter(
      int device, std::shared_ptr<TdiSdeInterface::SessionInterface> session,
      uint32 table_id, absl::optional<uint32> meter_index,
      TdiPktModMeterConfig& cfg) override LOCKS_EXCLUDED(data_lock_);
  ::util::Status ReadPktModMeters(
      int device, std::shared_ptr<TdiSdeInterface::SessionInterface> session,
      uint32 table_id, absl::optional<uint32> meter_index,
      std::vector<uint32>* meter_indices,
      std::vector<TdiPktModMeterConfig>& cfg) override
      LOCKS_EXCLUDED(data_lock_);
  ::util::Status DeletePktModMeterConfig(
      int device, std::shared_ptr<TdiSdeInterface::SessionInterface> session,
      uint32 table_id, absl::optional<uint32> meter_index) override
      LOCKS_EXCLUDED(data_lock_);
  ::util::Status InitNotificationTableWithCallback(
      int dev_id, std::shared_ptr<TdiSdeInterface::SessionInterface> session,
      const std::string& table_name,
      ipsec_notification_table_callback_t callback, void* cookie) const override
      LOCKS_EXCLUDED(data_lock_);
  ::util::Status InitNotificationTableWithCallback(
      int dev_id, std::shared_ptr<TdiSdeInterface::SessionInterface> session,
      const std::string& table_name,
      vport_notification_table_callback_t callback, void* cookie) const override
      LOCKS_EXCLUDED(data_lock_);

  // Creates the singleton instance. Expected to be called once to initialize
  // the instance.
  static Es2kSdeWrapper* CreateSingleton() LOCKS_EXCLUDED(init_lock_);

  // Return the singleton instance to be used in the SDE callbacks.
  static Es2kSdeWrapper* GetSingleton() LOCKS_EXCLUDED(init_lock_);

  ::util::Status SetPacketIoConfig(const PacketIoConfig& pktio_config) override;
  ::util::Status TxPacket(int device, const std::string& packet) override;
  ::util::Status StartPacketIo(int device) override;
  ::util::Status StopPacketIo(int device) override;
  ::util::Status RegisterPacketReceiveWriter(
      int device, std::unique_ptr<ChannelWriter<std::string>> writer) override;
  ::util::Status UnregisterPacketReceiveWriter(int device) override;

  // Writes a received packet to the registered Rx writer. Called from the SDE
  // callback function.
  ::util::Status HandlePacketRx(ipu_dev_id_t device, const char* pkt_data,
                                const uint64_t pkt_len)
      LOCKS_EXCLUDED(packet_rx_callback_lock_);

  // Es2kSdeWrapper is neither copyable nor movable.
  Es2kSdeWrapper(const Es2kSdeWrapper&) = delete;
  Es2kSdeWrapper& operator=(const Es2kSdeWrapper&) = delete;
  Es2kSdeWrapper(Es2kSdeWrapper&&) = delete;
  Es2kSdeWrapper& operator=(Es2kSdeWrapper&&) = delete;

 protected:
  // RW mutex lock for protecting the singleton instance initialization and
  // reading it back from other threads. Unlike other singleton classes, we
  // use RW lock as we need the pointer to class to be returned.
  static absl::Mutex init_lock_;

  // The singleton instance.
  static Es2kSdeWrapper* singleton_ GUARDED_BY(init_lock_);

 private:
  // Private constructor; use CreateSingleton and GetSingleton().
  Es2kSdeWrapper();

  // Callback registered with the SDE, called when packet is received.
  // NotificationParams contains the packet data.
  static void PktIoRxCallback(std::unique_ptr<::tdi::TableKey> key,
                              std::unique_ptr<::tdi::TableData> data,
                              std::unique_ptr<::tdi::NotificationParams> params,
                              void* cookie);

  // Callback registered with the SDE, called when packet is transmitted.
  // NotificationParams contains the packet data.
  static void PktIoTxCallback(std::unique_ptr<::tdi::TableKey> key,
                              std::unique_ptr<::tdi::TableData> data,
                              std::unique_ptr<::tdi::NotificationParams> params,
                              void* cookie);

  // Mutex protecting the packet rx writer map.
  mutable absl::Mutex packet_rx_callback_lock_;

  // packetIo config
  PacketIoConfig pktio_config_;

  // Map from device ID to packet receive writer.
  absl::flat_hash_map<int, std::unique_ptr<ChannelWriter<std::string>>>
      device_to_packet_rx_writer_ GUARDED_BY(packet_rx_callback_lock_);
};

}  // namespace tdi
}  // namespace hal
}  // namespace stratum

#endif  // STRATUM_HAL_LIB_TDI_ES2K_SDE_WRAPPER_H_
