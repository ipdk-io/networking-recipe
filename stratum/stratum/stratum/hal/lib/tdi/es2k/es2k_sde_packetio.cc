// Copyright 2023 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

#include <algorithm>
#include <memory>
#include <ostream>
#include <string>
#include <utility>
#include <vector>

#include "absl/memory/memory.h"
#include "absl/strings/str_cat.h"
#include "absl/synchronization/mutex.h"
#include "stratum/glue/gtl/map_util.h"
#include "stratum/glue/integral_types.h"
#include "stratum/glue/logging.h"
#include "stratum/glue/status/status.h"
#include "stratum/glue/status/status_macros.h"
#include "stratum/glue/status/statusor.h"
#include "stratum/hal/lib/tdi/es2k/es2k_sde_wrapper.h"
#include "stratum/hal/lib/tdi/tdi.pb.h"
#include "stratum/hal/lib/tdi/tdi_sde_common.h"
#include "stratum/hal/lib/tdi/tdi_sde_helpers.h"
#include "stratum/lib/utils.h"
#include "tdi/common/tdi_notifications.hpp"
#include "tdi_rt/tdi_rt_defs.h"
#include "tdi_rt/tdi_rt_operations.hpp"

#ifdef PKTIO_DUMP_PKT
#include <arpa/inet.h>
#include <netinet/if_ether.h>
#include <netinet/ip.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#endif

// The below macros and enum values are derived from
// $SDE_INSTALL/share/bf_rt_shared/tdi_pktio.json
#define PKTIO_TABLE_NAME "pktio.packetio"
#define RX_BURST_FIELD_ID 3

enum PktIoField {
  PORT_ID = 1,
  QUEUE_ID = 2,
  NB_PKTS = 3,
  PKT_LEN = 4,
  PKT_DATA = 5,
  BURST_SZ = 6
};

enum NotificationId { RX = 1, TX = 2 };

namespace stratum {
namespace hal {
namespace tdi {

using namespace stratum::hal::tdi::helpers;

#ifdef PKTIO_DUMP_PKT
void dump_pkt(const char* raw_data) {
  struct ip* ip_hdr;
  char src_ip_str[INET_ADDRSTRLEN];
  char dst_ip_str[INET_ADDRSTRLEN];

  ip_hdr = (struct ip*)(raw_data + 14);

  // Extract and print the source and destination MAC and IP addresses
  printf("pktio: Src MAC: ");
  for (int j = 6; j < 12; ++j) {
    printf("%02X", raw_data[j]);
    if (j < 11) printf(":");
  }
  printf("\n");

  printf("pktio: Dst MAC: ");
  for (int j = 0; j < 6; ++j) {
    printf("%02X", raw_data[j]);
    if (j < 5) printf(":");
  }
  printf("\n");

  inet_ntop(AF_INET, &(ip_hdr->ip_src), src_ip_str, INET_ADDRSTRLEN);
  inet_ntop(AF_INET, &(ip_hdr->ip_dst), dst_ip_str, INET_ADDRSTRLEN);

  printf("pktio: Src IP: %s\n", src_ip_str);
  printf("pktio: Dst IP: %s\n", dst_ip_str);
}
#endif

::util::Status Es2kSdeWrapper::RegisterPacketReceiveWriter(
    int device, std::unique_ptr<ChannelWriter<std::string>> writer) {
  absl::WriterMutexLock l(&packet_rx_callback_lock_);
  device_to_packet_rx_writer_[device] = std::move(writer);
  return ::util::OkStatus();
}

::util::Status Es2kSdeWrapper::UnregisterPacketReceiveWriter(int device) {
  absl::WriterMutexLock l(&packet_rx_callback_lock_);
  device_to_packet_rx_writer_.erase(device);
  return ::util::OkStatus();
}

::util::Status Es2kSdeWrapper::SetPacketIoConfig(
    const PacketIoConfig& pktio_config) {
  pktio_config_.CopyFrom(pktio_config);
  return ::util::OkStatus();
}

// Callback function called by the driver after transmitting the packet.
// Free the buffers now that transmission is complete
void Es2kSdeWrapper::PktIoTxCallback(
    std::unique_ptr<::tdi::TableKey> key,
    std::unique_ptr<::tdi::TableData> data,
    std::unique_ptr<::tdi::NotificationParams> pkts_info_recv, void* cookie) {
  uint64_t nb_pkts;
  std::vector<uint64_t> pkt_data; /* pointers to pkts data */

  pkts_info_recv->getValue(NB_PKTS, &nb_pkts);
  pkts_info_recv->getValue(PKT_DATA, &pkt_data);

  for (uint64_t i = 0; i < nb_pkts; i++) {
    delete[] reinterpret_cast<uint8_t*>(pkt_data[i]);
  }
}

// TxPacket involves three steps
// 1. Allocate operations of type transmit_pkt
// 2. Fill operations object with packet's data
// 3. Transmit the packet by calling OperationExecute
::util::Status Es2kSdeWrapper::TxPacket(int dev_id, const std::string& buffer) {
  if (pktio_config_.ports_size() == 0) {
    return MAKE_ERROR(::util::error::Code::UNAVAILABLE)
           << "packetIo not configured, can't transmit packet";
  }

  // create Target object for the device
  const ::tdi::Device* device = nullptr;
  ::tdi::DevMgr::getInstance().deviceGet(dev_id, &device);
  std::unique_ptr<::tdi::Target> dev_tgt;
  device->createTarget(&dev_tgt);
  if (dev_tgt == nullptr) {
    return MAKE_ERROR(::util::error::Code::RESOURCE_EXHAUSTED)
           << "Error allocating target object";
  }

  // Get pointer to packet i/o table.
  const ::tdi::Table* table;
  RETURN_IF_TDI_ERROR(tdi_info_->tableFromNameGet(PKTIO_TABLE_NAME, &table));

  // 1. allocate operations
  std::unique_ptr<::tdi::TableOperations> ops;
  auto status = table->operationsAllocate(
      static_cast<tdi_operations_type_e>(TDI_RT_OPERATIONS_TYPE_TRANSMIT_PKTS),
      &ops);
  if (status != IPU_SUCCESS) {
    return MAKE_ERROR(::util::error::Code::RESOURCE_EXHAUSTED)
           << "Error allocating TableOperations object";
  }

  // 2. Create TxPktsInfo object to provide information about the packet
  // to be transmitted.
  ::tdi::pna::rt::TxPktsInfo pkts_info;
  // Packet is transmitted on the first port in the ports list and queue 0
  pkts_info.port_id = pktio_config_.ports(0);
  pkts_info.queue_id = 0;
  pkts_info.burst_sz = 1;

  // pkt_buf to be freed in the tx callback function
  auto pkt_buf = new uint8_t[buffer.size()];
  std::memcpy(pkt_buf, buffer.c_str(), buffer.size());

#ifdef PKTIO_DUMP_PKT
  auto raw_data = reinterpret_cast<const char*>(pkt_buf);
  dump_pkt(raw_data);
#endif

  pkts_info.pkt_len[0] = buffer.size();
  pkts_info.pkt_data[0] = pkt_buf;

  ops->setValue(static_cast<tdi_operations_field_type_e>(
                    TDI_RT_OPERATIONS_TX_PKT_FIELD_TYPE_PKTS_INFO),
                reinterpret_cast<uint64_t>(&pkts_info));

  // 3. Transmit the pkt
  status = table->operationsExecute(*dev_tgt, *ops);
  if (status != IPU_SUCCESS) {
    // deallocate operations object and return
    ::tdi::TableOperations* rawPtr = ops.release();
    delete rawPtr;
    return MAKE_ERROR(::util::error::Code::INTERNAL) << "Packet Tx failed";
  }

  return ::util::OkStatus();
}

::util::Status Es2kSdeWrapper::HandlePacketRx(ipu_dev_id_t device,
                                              const char* pkt_data,
                                              const uint64_t pkt_len) {
  absl::ReaderMutexLock l(&packet_rx_callback_lock_);
  auto rx_writer = gtl::FindOrNull(device_to_packet_rx_writer_, device);
  RET_CHECK(rx_writer) << "No Rx callback registered for device id " << device
                       << ".";

  std::string buffer(pkt_data, pkt_data + pkt_len);
  if (!(*rx_writer)->TryWrite(buffer).ok()) {
    LOG_EVERY_N(INFO, 500) << "Dropped packet received from CPU.";
  }
  VLOG(1) << "Received " << buffer.size() << " byte packet from CPU "
          << StringToHex(buffer);

  return ::util::OkStatus();
}

// Callback function called by the driver when packets are received.
// NotificationParams contains the packets data and lengths
void Es2kSdeWrapper::PktIoRxCallback(
    std::unique_ptr<::tdi::TableKey> key,
    std::unique_ptr<::tdi::TableData> data,
    std::unique_ptr<::tdi::NotificationParams> pkts_info_recv, void* cookie) {
  uint64_t nb_pkts;
  std::vector<uint64_t> pkt_len;  /* len of pkts */
  std::vector<uint64_t> pkt_data; /* pointers to pkts data */

  pkts_info_recv->getValue(NB_PKTS, &nb_pkts);
  pkts_info_recv->getValue(PKT_LEN, &pkt_len);
  pkts_info_recv->getValue(PKT_DATA, &pkt_data);

  Es2kSdeWrapper* tdi_sde_wrapper = Es2kSdeWrapper::GetSingleton();

  const char* raw_data;
  for (uint64_t i = 0; i < nb_pkts; i++) {
    raw_data = reinterpret_cast<const char*>(pkt_data[i]);
#ifdef PKTIO_DUMP_PKT
    dump_pkt(raw_data);
#endif
    // TODO: Need to find a way to get device_id
    tdi_sde_wrapper->HandlePacketRx(0, raw_data, pkt_len[i]);
  }
}

// StartPacketIo initializes callback registration for all the ports and queues
// defined in PacketIoConfig. For Rx operations, the driver code continuously
// polls packets on the configured ports and queues, invoking the registered
// callbacks with packet details. As for Tx operations, packets are transmitted
// via the first available port and queue from the configuration.
::util::Status Es2kSdeWrapper::StartPacketIo(int dev_id) {
  if (pktio_config_.ports_size() == 0) {
    LOG(INFO) << "packetIo not configured";
    return ::util::OkStatus();
  }

  // create Target object for the device
  const ::tdi::Device* device = nullptr;
  ::tdi::DevMgr::getInstance().deviceGet(dev_id, &device);
  std::unique_ptr<::tdi::Target> dev_tgt;
  device->createTarget(&dev_tgt);
  if (dev_tgt == nullptr) {
    return MAKE_ERROR(::util::error::Code::RESOURCE_EXHAUSTED)
           << "Error allocating target object";
  }

  // Get pointer to packet i/o table.
  const ::tdi::Table* table;
  RETURN_IF_TDI_ERROR(tdi_info_->tableFromNameGet(PKTIO_TABLE_NAME, &table));

  // Allocate rx params
  std::unique_ptr<::tdi::NotificationParams> rx_notification_params;
  tdi_status_t status = table->notificationRegistrationParamsAllocate(
      RX, &rx_notification_params);
  if (status != IPU_SUCCESS) {
    return MAKE_ERROR(::util::error::Code::RESOURCE_EXHAUSTED)
           << "rx_notification allocation fail ";
  }

  // Allocate tx params
  std::unique_ptr<::tdi::NotificationParams> tx_notification_params;
  table->notificationRegistrationParamsAllocate(TX, &tx_notification_params);
  if (status != IPU_SUCCESS) {
    return MAKE_ERROR(::util::error::Code::RESOURCE_EXHAUSTED)
           << "tx_notification allocation fail ";
  }

  // Register rx and tx callbacks for all the ports and queues
  int port_id = 0;
  for (int port_idx = 0; port_idx < pktio_config_.ports_size(); port_idx++) {
    port_id = pktio_config_.ports(port_idx);
    for (int queue_id = 0; queue_id < pktio_config_.nb_rxqs(); queue_id++) {
      // set values
      rx_notification_params->setValue(PORT_ID, port_id);
      rx_notification_params->setValue(QUEUE_ID, queue_id);
      rx_notification_params->setValue(RX_BURST_FIELD_ID, 1);

      // register rx callback
      status = table->notificationRegister(*dev_tgt, RX,
                                           Es2kSdeWrapper::PktIoRxCallback,
                                           *rx_notification_params, nullptr);

      if (status != IPU_SUCCESS) {
        // deallocate notification_params and return error
        ::tdi::NotificationParams* rawPtr = rx_notification_params.release();
        delete rawPtr;
        return MAKE_ERROR(::util::error::Code::INTERNAL)
               << "rx notification registration fail ";
      }
    }

    for (int queue_id = 0; queue_id < pktio_config_.nb_txqs(); queue_id++) {
      // set values
      tx_notification_params->setValue(PORT_ID, port_id);
      tx_notification_params->setValue(QUEUE_ID, queue_id);

      // register tx callback
      status = table->notificationRegister(*dev_tgt, TX,
                                           Es2kSdeWrapper::PktIoTxCallback,
                                           *tx_notification_params, nullptr);

      if (status != IPU_SUCCESS) {
        // deallocate notification_params and return error
        ::tdi::NotificationParams* rawPtr = tx_notification_params.release();
        delete rawPtr;
        return MAKE_ERROR(::util::error::Code::INTERNAL)
               << "tx notification registration fail ";
      }
    }
  }
  return ::util::OkStatus();
}

// Deregisters callbacks for the configured ports and queues.
// After this, ports and queues will no longer be polled, and packet
// transmission will fail. This operation is currently invoked only during the
// exit of infrap4d.
::util::Status Es2kSdeWrapper::StopPacketIo(int dev_id) {
  if (pktio_config_.ports_size() == 0) {
    LOG(INFO) << "packetIo not configured";
    return ::util::OkStatus();
  }

  // create Target object for the device
  const ::tdi::Device* device = nullptr;
  ::tdi::DevMgr::getInstance().deviceGet(dev_id, &device);
  std::unique_ptr<::tdi::Target> dev_tgt;
  device->createTarget(&dev_tgt);
  if (dev_tgt == nullptr) {
    return MAKE_ERROR(::util::error::Code::RESOURCE_EXHAUSTED)
           << "Error allocating target object";
  }

  // Get pointer to packet i/o table.
  const ::tdi::Table* table;
  RETURN_IF_TDI_ERROR(tdi_info_->tableFromNameGet(PKTIO_TABLE_NAME, &table));

  // Allocate rx params
  std::unique_ptr<::tdi::NotificationParams> rx_notification_params;
  tdi_status_t status = table->notificationRegistrationParamsAllocate(
      RX, &rx_notification_params);
  if (status != IPU_SUCCESS) {
    return MAKE_ERROR(::util::error::Code::RESOURCE_EXHAUSTED)
           << "rx_notification allocation fail ";
  }

  // Allocate tx params
  std::unique_ptr<::tdi::NotificationParams> tx_notification_params;
  table->notificationRegistrationParamsAllocate(TX, &tx_notification_params);
  if (status != IPU_SUCCESS) {
    return MAKE_ERROR(::util::error::Code::RESOURCE_EXHAUSTED)
           << "tx_notification allocation fail ";
  }

  // Deregister rx and tx callbacks for all the ports and queues
  int port_id = 0;
  for (int port_idx = 0; port_idx < pktio_config_.ports_size(); port_idx++) {
    port_id = pktio_config_.ports(port_idx);
    for (int queue_id = 0; queue_id < pktio_config_.nb_rxqs(); queue_id++) {
      // set values
      rx_notification_params->setValue(PORT_ID, port_id);
      rx_notification_params->setValue(QUEUE_ID, queue_id);
      rx_notification_params->setValue(RX_BURST_FIELD_ID, 1);

      // deregister rx callback
      status =
          table->notificationDeregister(*dev_tgt, RX, *rx_notification_params);

      if (status != IPU_SUCCESS) {
        // deallocate notification_params and return error
        ::tdi::NotificationParams* rawPtr = rx_notification_params.release();
        delete rawPtr;
        return MAKE_ERROR(::util::error::Code::INTERNAL)
               << "rx notification deregistration fail ";
      }
    }

    for (int queue_id = 0; queue_id < pktio_config_.nb_txqs(); queue_id++) {
      // set values
      tx_notification_params->setValue(PORT_ID, port_id);
      tx_notification_params->setValue(QUEUE_ID, queue_id);

      // deregister tx callback
      status =
          table->notificationDeregister(*dev_tgt, TX, *tx_notification_params);

      if (status != IPU_SUCCESS) {
        // deallocate notification_params and return error
        ::tdi::NotificationParams* rawPtr = tx_notification_params.release();
        delete rawPtr;
        return MAKE_ERROR(::util::error::Code::INTERNAL)
               << "tx notification deregistration fail ";
      }
    }
  }

  return ::util::OkStatus();
}

}  // namespace tdi
}  // namespace hal
}  // namespace stratum
