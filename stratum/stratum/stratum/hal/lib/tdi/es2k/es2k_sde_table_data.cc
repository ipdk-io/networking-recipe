// Copyright 2023 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

// ES2K-specific PktModMeterConfig methods.

#include <string>

#include "stratum/hal/lib/tdi/tdi_constants.h"
#include "stratum/hal/lib/tdi/tdi_pkt_mod_meter_config.h"
#include "stratum/hal/lib/tdi/tdi_sde_helpers.h"
#include "stratum/hal/lib/tdi/tdi_sde_wrapper.h"

namespace stratum {
namespace hal {
namespace tdi {

using namespace stratum::hal::tdi::helpers;

::util::Status TableData::SetPktModMeterConfig(
    const TdiPktModMeterConfig& cfg) {
  if (cfg.isPktModMeter) {
    RETURN_IF_ERROR(SetField(table_data_.get(), kMeterCirPps, cfg.cir));
    RETURN_IF_ERROR(
        SetField(table_data_.get(), kMeterCommitedBurstPackets, cfg.cburst));
    RETURN_IF_ERROR(SetField(table_data_.get(), kMeterPirPps, cfg.pir));
    RETURN_IF_ERROR(
        SetField(table_data_.get(), kMeterPeakBurstPackets, cfg.pburst));
  } else {
    RETURN_IF_ERROR(SetField(table_data_.get(), kEs2kMeterProfileIdKbps,
                             cfg.meter_prof_id));
    RETURN_IF_ERROR(
        SetField(table_data_.get(), kEs2kMeterCirKbpsUnit, cfg.cir_unit));
    RETURN_IF_ERROR(SetField(
        table_data_.get(), kEs2kMeterCommitedBurstKbitsUnit, cfg.cburst_unit));
    RETURN_IF_ERROR(
        SetField(table_data_.get(), kEs2kMeterPirKbpsUnit, cfg.pir_unit));
    RETURN_IF_ERROR(SetField(table_data_.get(), kEs2kMeterPeakBurstKbitsUnit,
                             cfg.pburst_unit));
    RETURN_IF_ERROR(SetField(table_data_.get(), kEs2kMeterCirKbps, cfg.cir));
    RETURN_IF_ERROR(
        SetField(table_data_.get(), kEs2kMeterCommitedBurstKbits, cfg.cburst));
    RETURN_IF_ERROR(SetField(table_data_.get(), kEs2kMeterPirKbps, cfg.pir));
    RETURN_IF_ERROR(
        SetField(table_data_.get(), kEs2kMeterPeakBurstKbits, cfg.pburst));
    RETURN_IF_ERROR(
        SetField(table_data_.get(), kEs2kMeterPeakBurstKbits, cfg.pburst));
    RETURN_IF_ERROR(
        SetField(table_data_.get(), kEs2kMeterPeakBurstKbits, cfg.pburst));
    RETURN_IF_ERROR(SetField(table_data_.get(), kEs2kMeterGreenCounterBytes,
                             cfg.greenBytes));
    RETURN_IF_ERROR(SetField(table_data_.get(), kEs2kMeterGreenCounterPackets,
                             cfg.greenPackets));
    RETURN_IF_ERROR(SetField(table_data_.get(), kEs2kMeterYellowCounterBytes,
                             cfg.yellowBytes));
    RETURN_IF_ERROR(SetField(table_data_.get(), kEs2kMeterYellowCounterPackets,
                             cfg.yellowPackets));
    RETURN_IF_ERROR(
        SetField(table_data_.get(), kEs2kMeterRedCounterBytes, cfg.redBytes));
    RETURN_IF_ERROR(SetField(table_data_.get(), kEs2kMeterRedCounterPackets,
                             cfg.redPackets));
  }
  return ::util::OkStatus();
}

::util::Status TableData::GetPktModMeterConfig(
    TdiPktModMeterConfig& cfg) const {
  // Condition checks for Indirect PacketModMeter
  if (cfg.isPktModMeter) {
    RETURN_IF_ERROR(GetField(*table_data_.get(), kMeterCirPps, &cfg.cir));
    RETURN_IF_ERROR(
        GetField(*table_data_.get(), kMeterCommitedBurstPackets, &cfg.cburst));
    RETURN_IF_ERROR(GetField(*table_data_.get(), kMeterPirPps, &cfg.pir));
    RETURN_IF_ERROR(
        GetField(*table_data_.get(), kMeterPeakBurstPackets, &cfg.pburst));
  }

  // Condition checks for Direct PacketModMeter
  else {
    RETURN_IF_ERROR(GetField(*table_data_.get(), kEs2kMeterProfileIdKbps,
                             &cfg.meter_prof_id));
    RETURN_IF_ERROR(
        GetField(*table_data_.get(), kEs2kMeterCirKbpsUnit, &cfg.cir_unit));
    RETURN_IF_ERROR(GetField(*table_data_.get(),
                             kEs2kMeterCommitedBurstKbitsUnit,
                             &cfg.cburst_unit));
    RETURN_IF_ERROR(
        GetField(*table_data_.get(), kEs2kMeterPirKbpsUnit, &cfg.pir_unit));
    RETURN_IF_ERROR(GetField(*table_data_.get(), kEs2kMeterPeakBurstKbitsUnit,
                             &cfg.pburst_unit));
    RETURN_IF_ERROR(GetField(*table_data_.get(), kEs2kMeterCirKbps, &cfg.cir));
    RETURN_IF_ERROR(GetField(*table_data_.get(), kEs2kMeterCommitedBurstKbits,
                             &cfg.cburst));
    RETURN_IF_ERROR(GetField(*table_data_.get(), kEs2kMeterPirKbps, &cfg.pir));
    RETURN_IF_ERROR(
        GetField(*table_data_.get(), kEs2kMeterPeakBurstKbits, &cfg.pburst));
    RETURN_IF_ERROR(GetField(*table_data_.get(), kEs2kMeterGreenCounterBytes,
                             &cfg.greenBytes));
    RETURN_IF_ERROR(GetField(*table_data_.get(), kEs2kMeterGreenCounterPackets,
                             &cfg.greenPackets));
    RETURN_IF_ERROR(GetField(*table_data_.get(), kEs2kMeterYellowCounterBytes,
                             &cfg.yellowBytes));
    RETURN_IF_ERROR(GetField(*table_data_.get(), kEs2kMeterYellowCounterPackets,
                             &cfg.yellowPackets));
    RETURN_IF_ERROR(
        GetField(*table_data_.get(), kEs2kMeterRedCounterBytes, &cfg.redBytes));
    RETURN_IF_ERROR(GetField(*table_data_.get(), kEs2kMeterRedCounterPackets,
                             &cfg.redPackets));

    cfg.cir_unit = cfg.cir_unit;
    cfg.cburst_unit = cfg.cburst_unit;
    cfg.pir_unit = cfg.pir_unit;
    cfg.pburst_unit = cfg.pburst_unit;
    cfg.cir = cfg.cir;
    cfg.cburst = cfg.cburst;
    cfg.pir = cfg.pir;
    cfg.pburst = cfg.pburst;
    cfg.greenBytes = cfg.greenBytes;
    cfg.greenPackets = cfg.greenPackets;
    cfg.yellowBytes = cfg.yellowBytes;
    cfg.yellowPackets = cfg.yellowPackets;
    cfg.redBytes = cfg.redBytes;
    cfg.redPackets = cfg.redPackets;
  }
  return ::util::OkStatus();
}

}  // namespace tdi
}  // namespace hal
}  // namespace stratum
