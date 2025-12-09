// Copyright 2019-present Barefoot Networks, Inc.
// Copyright 2022-2024 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

// Tofino-specific SDE port methods.

#include "stratum/hal/lib/tdi/tofino/tofino_port_manager.h"

#include <stdint.h>
#include <stdio.h>

#include <algorithm>
#include <memory>
#include <ostream>
#include <string>
#include <utility>

#include "absl/synchronization/mutex.h"
#include "absl/time/clock.h"
#include "absl/time/time.h"
#include "stratum/glue/integral_types.h"
#include "stratum/glue/logging.h"
#include "stratum/glue/status/status.h"
#include "stratum/glue/status/status_macros.h"
#include "stratum/glue/status/statusor.h"
#include "stratum/hal/lib/common/common.pb.h"
#include "stratum/hal/lib/common/utils.h"
#include "stratum/hal/lib/tdi/tdi_status.h"
#include "stratum/lib/channel/channel.h"
#include "stratum/lib/constants.h"

extern "C" {
#include "bf_types/bf_types.h"
#include "port_mgr/bf_port_if.h"
#include "tofino/bf_pal/bf_pal_port_intf.h"
#include "tofino/pdfixed/pd_devport_mgr.h"
#include "tofino/pdfixed/pd_tm.h"
}

namespace stratum {
namespace hal {
namespace tdi {

TofinoPortManager* TofinoPortManager::singleton_ = nullptr;
constexpr int32 TofinoPortManager::kBfDefaultMtu;

namespace {

::util::StatusOr<bf_port_speed_t> PortSpeedHalToBf(uint64 speed_bps) {
  switch (speed_bps) {
    case kOneGigBps:
      return BF_SPEED_1G;
    case kTenGigBps:
      return BF_SPEED_10G;
    case kTwentyFiveGigBps:
      return BF_SPEED_25G;
    case kFortyGigBps:
      return BF_SPEED_40G;
    case kFiftyGigBps:
      return BF_SPEED_50G;
    case kHundredGigBps:
      return BF_SPEED_100G;
    default:
      return MAKE_ERROR(ERR_INVALID_PARAM) << "Unsupported port speed.";
  }
}

::util::StatusOr<int> AutonegHalToBf(TriState autoneg) {
  switch (autoneg) {
    case TRI_STATE_UNKNOWN:
      return 0;
    case TRI_STATE_TRUE:
      return 1;
    case TRI_STATE_FALSE:
      return 2;
    default:
      return MAKE_ERROR(ERR_INVALID_PARAM) << "Invalid autoneg state.";
  }
}

::util::StatusOr<bf_fec_type_t> FecModeHalToBf(FecMode fec_mode,
                                               uint64 speed_bps) {
  if (fec_mode == FEC_MODE_UNKNOWN || fec_mode == FEC_MODE_OFF) {
    return BF_FEC_TYP_NONE;
  } else if (fec_mode == FEC_MODE_ON || fec_mode == FEC_MODE_AUTO) {
    // we have to "guess" the FEC type to use based on the port speed.
    switch (speed_bps) {
      case kOneGigBps:
        return MAKE_ERROR(ERR_INVALID_PARAM)
               << "Invalid FEC mode for 1Gbps mode.";
      case kTenGigBps:
      case kFortyGigBps:
        return BF_FEC_TYP_FIRECODE;
      case kTwentyFiveGigBps:
      case kFiftyGigBps:
      case kHundredGigBps:
      case kTwoHundredGigBps:
      case kFourHundredGigBps:
        return BF_FEC_TYP_REED_SOLOMON;
      default:
        return MAKE_ERROR(ERR_INVALID_PARAM) << "Unsupported port speed.";
    }
  }
  return MAKE_ERROR(ERR_INVALID_PARAM) << "Invalid FEC mode.";
}

::util::StatusOr<bf_loopback_mode_e> LoopbackModeToBf(
    LoopbackState loopback_mode) {
  switch (loopback_mode) {
    case LOOPBACK_STATE_NONE:
      return BF_LPBK_NONE;
    case LOOPBACK_STATE_MAC:
      return BF_LPBK_MAC_NEAR;
    default:
      return MAKE_ERROR(ERR_INVALID_PARAM)
             << "Unsupported loopback mode: "
             << LoopbackState_Name(loopback_mode) << ".";
  }
}

// A callback function executed in SDE port state change thread context.
bf_status_t sde_port_status_callback(bf_dev_id_t device, bf_dev_port_t dev_port,
                                     bool up, void* cookie) {
  absl::Time timestamp = absl::Now();
  TofinoPortManager* tofino_port_manager = TofinoPortManager::GetSingleton();
  if (!tofino_port_manager) {
    LOG(ERROR) << "TofinoPortManager singleton instance is not initialized.";
    return BF_INTERNAL_ERROR;
  }
  // Forward the event.
  auto status =
      tofino_port_manager->OnPortStatusEvent(device, dev_port, up, timestamp);

  return status.ok() ? BF_SUCCESS : BF_INTERNAL_ERROR;
}

}  // namespace

TofinoPortManager* TofinoPortManager::CreateSingleton() {
  absl::WriterMutexLock l(&init_lock_);
  if (!singleton_) {
    singleton_ = new TofinoPortManager();
  }

  return singleton_;
}

TofinoPortManager* TofinoPortManager::GetSingleton() {
  absl::ReaderMutexLock l(&init_lock_);
  return singleton_;
}

::util::StatusOr<PortState> TofinoPortManager::GetPortState(int device,
                                                            int port) {
  int state = 0;
  RETURN_IF_TDI_ERROR(
      bf_pal_port_oper_state_get(static_cast<bf_dev_id_t>(device),
                                 static_cast<bf_dev_port_t>(port), &state));
  return state ? PORT_STATE_UP : PORT_STATE_DOWN;
}

::util::Status TofinoPortManager::GetPortCounters(int device, int port,
                                                  PortCounters* counters) {
  uint64_t stats[BF_NUM_RMON_COUNTERS] = {0};
  RETURN_IF_TDI_ERROR(
      bf_pal_port_all_stats_get(static_cast<bf_dev_id_t>(device),
                                static_cast<bf_dev_port_t>(port), stats));
  counters->set_in_octets(stats[bf_mac_stat_OctetsReceived]);
  counters->set_out_octets(stats[bf_mac_stat_OctetsTransmittedTotal]);
  counters->set_in_unicast_pkts(
      stats[bf_mac_stat_FramesReceivedwithUnicastAddresses]);
  counters->set_out_unicast_pkts(stats[bf_mac_stat_FramesTransmittedUnicast]);
  counters->set_in_broadcast_pkts(
      stats[bf_mac_stat_FramesReceivedwithBroadcastAddresses]);
  counters->set_out_broadcast_pkts(
      stats[bf_mac_stat_FramesTransmittedBroadcast]);
  counters->set_in_multicast_pkts(
      stats[bf_mac_stat_FramesReceivedwithMulticastAddresses]);
  counters->set_out_multicast_pkts(
      stats[bf_mac_stat_FramesTransmittedMulticast]);
  counters->set_in_discards(stats[bf_mac_stat_FramesDroppedBufferFull]);
  counters->set_out_discards(0);       // stat not available
  counters->set_in_unknown_protos(0);  // stat not meaningful
  counters->set_in_errors(stats[bf_mac_stat_FrameswithanyError]);
  counters->set_out_errors(stats[bf_mac_stat_FramesTransmittedwithError]);
  counters->set_in_fcs_errors(stats[bf_mac_stat_FramesReceivedwithFCSError]);

  return ::util::OkStatus();
}

::util::Status TofinoPortManager::OnPortStatusEvent(int device, int port,
                                                    bool up,
                                                    absl::Time timestamp) {
  // Create PortStatusEvent message.
  PortState state = up ? PORT_STATE_UP : PORT_STATE_DOWN;
  PortStatusEvent event = {device, port, state, timestamp};

  {
    absl::ReaderMutexLock l(&port_status_event_writer_lock_);
    if (!port_status_event_writer_) {
      return ::util::OkStatus();
    }
    return port_status_event_writer_->Write(event, kWriteTimeout);
  }
}

::util::Status TofinoPortManager::RegisterPortStatusEventWriter(
    std::unique_ptr<ChannelWriter<PortStatusEvent>> writer) {
  absl::WriterMutexLock l(&port_status_event_writer_lock_);
  port_status_event_writer_ = std::move(writer);
  RETURN_IF_TDI_ERROR(
      bf_pal_port_status_notif_reg(sde_port_status_callback, nullptr));
  return ::util::OkStatus();
}

::util::Status TofinoPortManager::GetPortInfo(int device, int port,
                                              TargetDatapathId* target_dp_id) {
  return ::util::OkStatus();
}

::util::Status TofinoPortManager::AddPort(int device, int port) {
  return ::util::OkStatus();
}

::util::Status TofinoPortManager::AddPort(int device, int port,
                                          uint64 speed_bps, FecMode fec_mode) {
  ASSIGN_OR_RETURN(auto bf_speed, PortSpeedHalToBf(speed_bps));
  ASSIGN_OR_RETURN(auto bf_fec_mode, FecModeHalToBf(fec_mode, speed_bps));
  RETURN_IF_TDI_ERROR(bf_pal_port_add(static_cast<bf_dev_id_t>(device),
                                      static_cast<bf_dev_port_t>(port),
                                      bf_speed, bf_fec_mode));
  return ::util::OkStatus();
}

::util::Status TofinoPortManager::DeletePort(int device, int port) {
  RETURN_IF_TDI_ERROR(bf_pal_port_del(static_cast<bf_dev_id_t>(device),
                                      static_cast<bf_dev_port_t>(port)));
  return ::util::OkStatus();
}

::util::Status TofinoPortManager::EnablePort(int device, int port) {
  RETURN_IF_TDI_ERROR(bf_pal_port_enable(static_cast<bf_dev_id_t>(device),
                                         static_cast<bf_dev_port_t>(port)));
  return ::util::OkStatus();
}

::util::Status TofinoPortManager::DisablePort(int device, int port) {
  RETURN_IF_TDI_ERROR(bf_pal_port_disable(static_cast<bf_dev_id_t>(device),
                                          static_cast<bf_dev_port_t>(port)));
  return ::util::OkStatus();
}

::util::Status TofinoPortManager::SetPortShapingRate(int device, int port,
                                                     bool is_in_pps,
                                                     uint32 burst_size,
                                                     uint64 rate_per_second) {
  if (!is_in_pps) {
    rate_per_second /= 1000;  // The SDE expects the bitrate in kbps.
  }

  RETURN_IF_TDI_ERROR(p4_pd_tm_set_port_shaping_rate(
      device, port, is_in_pps, burst_size, rate_per_second));
  return ::util::OkStatus();
}

::util::Status TofinoPortManager::EnablePortShaping(int device, int port,
                                                    TriState enable) {
  if (enable == TriState::TRI_STATE_TRUE) {
    RETURN_IF_TDI_ERROR(p4_pd_tm_enable_port_shaping(device, port));
  } else if (enable == TriState::TRI_STATE_FALSE) {
    RETURN_IF_TDI_ERROR(p4_pd_tm_disable_port_shaping(device, port));
  }

  return ::util::OkStatus();
}

::util::Status TofinoPortManager::SetPortAutonegPolicy(int device, int port,
                                                       TriState autoneg) {
  ASSIGN_OR_RETURN(auto autoneg_v, AutonegHalToBf(autoneg));
  RETURN_IF_TDI_ERROR(bf_pal_port_autoneg_policy_set(
      static_cast<bf_dev_id_t>(device), static_cast<bf_dev_port_t>(port),
      autoneg_v));
  return ::util::OkStatus();
}

::util::Status TofinoPortManager::SetPortMtu(int device, int port, int32 mtu) {
  if (mtu < 0) {
    return MAKE_ERROR(ERR_INVALID_PARAM) << "Invalid MTU value.";
  }
  if (mtu == 0) mtu = kBfDefaultMtu;
  RETURN_IF_TDI_ERROR(bf_pal_port_mtu_set(
      static_cast<bf_dev_id_t>(device), static_cast<bf_dev_port_t>(port),
      static_cast<uint32>(mtu), static_cast<uint32>(mtu)));
  return ::util::OkStatus();
}

bool TofinoPortManager::IsValidPort(int device, int port) {
  return bf_pal_port_is_valid(device, port) == BF_SUCCESS;
}

::util::Status TofinoPortManager::SetPortLoopbackMode(
    int device, int port, LoopbackState loopback_mode) {
  if (loopback_mode == LOOPBACK_STATE_UNKNOWN) {
    // Do nothing if we try to set loopback mode to the default one (UNKNOWN).
    return ::util::OkStatus();
  }
  ASSIGN_OR_RETURN(bf_loopback_mode_e lp_mode, LoopbackModeToBf(loopback_mode));
  RETURN_IF_TDI_ERROR(
      bf_pal_port_loopback_mode_set(static_cast<bf_dev_id_t>(device),
                                    static_cast<bf_dev_port_t>(port), lp_mode));
  return ::util::OkStatus();
}

::util::StatusOr<uint32> TofinoPortManager::GetPortIdFromPortKey(
    int device, const PortKey& port_key) {
  const int port = port_key.port;
  RET_CHECK(port >= 0) << "Port ID must be non-negative. Attempted to get port "
                       << port << " on dev " << device << ".";

  // PortKey uses three possible values for channel:
  //     > 0: port is channelized (first channel is 1)
  //     0: port is not channelized
  //     < 0: port channel is not important (e.g. for port groups)
  // BF SDK expects the first channel to be 0
  //     Convert base-1 channel to base-0 channel if port is channelized
  //     Otherwise, port is already 0 in the non-channelized case
  const int channel =
      (port_key.channel > 0) ? port_key.channel - 1 : port_key.channel;
  RET_CHECK(channel >= 0) << "Channel must be set for port " << port
                          << " on dev " << device << ".";

  char port_string[MAX_PORT_HDL_STRING_LEN];
  int r = snprintf(port_string, sizeof(port_string), "%d/%d", port, channel);
  RET_CHECK(r > 0 && r < sizeof(port_string))
      << "Failed to build port string for port " << port << " channel "
      << channel << " on dev " << device << ".";

  bf_dev_port_t dev_port;
  RETURN_IF_TDI_ERROR(bf_pal_port_str_to_dev_port_map(
      static_cast<bf_dev_id_t>(device), port_string, &dev_port));
  return static_cast<uint32>(dev_port);
}

::util::StatusOr<int> TofinoPortManager::GetPcieCpuPort(int device) {
  int port = p4_devport_mgr_pcie_cpu_port_get(device);
  RET_CHECK(port != -1);
  return port;
}

::util::Status TofinoPortManager::SetTmCpuPort(int device, int port) {
  RET_CHECK(p4_pd_tm_set_cpuport(device, port) == 0)
      << "Unable to set CPU port " << port << " on device " << device;
  return ::util::OkStatus();
}

::util::Status TofinoPortManager::SetDeflectOnDropDestination(int device,
                                                              int port,
                                                              int queue) {
  // The DoD destination must be a pipe-local port.
  p4_pd_tm_pipe_t pipe = DEV_PORT_TO_PIPE(port);
  RETURN_IF_TDI_ERROR(
      p4_pd_tm_set_negative_mirror_dest(device, pipe, port, queue));
  return ::util::OkStatus();
}

}  // namespace tdi
}  // namespace hal
}  // namespace stratum
