// Copyright 2019-present Barefoot Networks, Inc.
// Copyright 2022-2023 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

#ifndef STRATUM_HAL_LIB_TDI_TOFINO_SDE_WRAPPER_H_
#define STRATUM_HAL_LIB_TDI_TOFINO_SDE_WRAPPER_H_

#include <memory>
#include <string>

#include "absl/container/flat_hash_map.h"
#include "absl/synchronization/mutex.h"
#include "pkt_mgr/pkt_mgr_intf.h"
#include "stratum/hal/lib/tdi/tdi.pb.h"
#include "stratum/hal/lib/tdi/tdi_sde_wrapper.h"
#include "stratum/lib/channel/channel.h"

namespace stratum {
namespace hal {
namespace tdi {

class TofinoSdeWrapper : public TdiSdeWrapper {
 public:
  ::util::StatusOr<bool> IsSoftwareModel(int device) override;
  std::string GetChipType(int device) const override;
  std::string GetSdeVersion() const override;

  ::util::Status InitializeSde(const std::string& sde_install_path,
                               const std::string& sde_config_file,
                               bool run_in_background) override;
  ::util::Status AddDevice(int device,
                           const TdiDeviceConfig& device_config) override;

  ::util::Status TxPacket(int device, const std::string& packet) override;
  ::util::Status StartPacketIo(int device) override;
  ::util::Status StopPacketIo(int device) override;
  ::util::Status RegisterPacketReceiveWriter(
      int device, std::unique_ptr<ChannelWriter<std::string>> writer) override;
  ::util::Status UnregisterPacketReceiveWriter(int device) override;

  // Writes a received packet to the registered Rx writer. Called from the SDE
  // callback function.
  ::util::Status HandlePacketRx(bf_dev_id_t device, bf_pkt* pkt,
                                bf_pkt_rx_ring_t rx_ring)
      LOCKS_EXCLUDED(packet_rx_callback_lock_);

  // Creates the singleton instance. Expected to be called once to initialize
  // the instance.
  static TofinoSdeWrapper* CreateSingleton() LOCKS_EXCLUDED(init_lock_);

  // Return the singleton instance to be used in the SDE callbacks.
  static TofinoSdeWrapper* GetSingleton() LOCKS_EXCLUDED(init_lock_);

  // TofinoSdeWrapper is neither copyable nor movable.
  TofinoSdeWrapper(const TofinoSdeWrapper&) = delete;
  TofinoSdeWrapper& operator=(const TofinoSdeWrapper&) = delete;
  TofinoSdeWrapper(TofinoSdeWrapper&&) = delete;
  TofinoSdeWrapper& operator=(TofinoSdeWrapper&&) = delete;

 protected:
  // RW mutex lock for protecting the singleton instance initialization and
  // reading it back from other threads. Unlike other singleton classes, we
  // use RW lock as we need the pointer to class to be returned.
  static absl::Mutex init_lock_;

  // The singleton instance.
  static TofinoSdeWrapper* singleton_ GUARDED_BY(init_lock_);

 private:
  // Private constructor. Use CreateSingleton and GetSingleton().
  TofinoSdeWrapper();

  // Mutex protecting the packet rx writer map.
  mutable absl::Mutex packet_rx_callback_lock_;

  // Callback registed with the SDE for Tx notifications.
  static bf_status_t PacketTxNotifyCallback(bf_dev_id_t device,
                                            bf_pkt_tx_ring_t tx_ring,
                                            uint64 tx_cookie, uint32 status);

  // Callback registed with the SDE for Rx notifications.
  static bf_status_t PacketRxNotifyCallback(bf_dev_id_t device, bf_pkt* pkt,
                                            void* cookie,
                                            bf_pkt_rx_ring_t rx_ring);

  // Map from device ID to packet receive writer.
  absl::flat_hash_map<int, std::unique_ptr<ChannelWriter<std::string>>>
      device_to_packet_rx_writer_ GUARDED_BY(packet_rx_callback_lock_);
};

}  // namespace tdi
}  // namespace hal
}  // namespace stratum

#endif  // STRATUM_HAL_LIB_TDI_TOFINO_SDE_WRAPPER_H_
