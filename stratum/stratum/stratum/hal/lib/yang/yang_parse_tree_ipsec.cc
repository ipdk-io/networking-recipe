// Copyright 2022-2023 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

// Implements the YangParseTreePaths::AddSubtreeIPsec() method and its
// supporting functions.

#include "gnmi/gnmi.pb.h"
#include "stratum/glue/logging.h"
#include "stratum/glue/status/status_macros.h"
#include "stratum/hal/lib/common/gnmi_events.h"
#include "stratum/hal/lib/common/gnmi_publisher.h"
#include "stratum/hal/lib/common/utils.h"
#include "stratum/hal/lib/yang/yang_parse_tree.h"
#include "stratum/hal/lib/yang/yang_parse_tree_helpers.h"
#include "stratum/hal/lib/yang/yang_parse_tree_paths.h"

namespace stratum {
namespace hal {

using namespace stratum::hal::yang::helpers;

namespace {

// Notification message that is sent to gnmi-client
std::string ConvertIPsecNotificationToString(const IPsecNotification& notif) {
  std::stringstream ss;
  ss << "ipsec-sa-spi: " << notif.ipsec_sa_spi();
  ss << ", soft-lifetime-expire: " << notif.soft_lifetime_expire();
  ss << ", ipsec-sa-protocol: " << notif.ipsec_sa_protocol();
  ss << ", ipsec-sa-dest-address: " << notif.ipsec_sa_dest_address();
  ss << ", address-family: " << notif.address_family();
  return ss.str();
}

////////////////////////////////////////////////////////////////////////////////
// /ipsec-offload/ipsec-spi/rx-spi
void SetUpIPsecFetchSPI(TreeNode* node, YangParseTree* tree) {
  auto poll_functor = [tree](const GnmiEvent& event, const ::gnmi::Path& path,
                             GnmiSubscribeStream* stream) {
    // Create a data retrieval request.
    DataRequest req;
    auto* request = req.add_requests()->mutable_ipsec_offload_info();
    // In-place definition of method retrieving data from generic response
    // and saving into 'resp' local variable.
    uint32 resp{};
    DataResponseWriter writer([&resp](const DataResponse& in) {
      if (!in.has_ipsec_offload_info()) return false;
      resp = in.ipsec_offload_info().spi();
      return true;
    });
    // Query the switch. The returned status is ignored as there is no way to
    // notify the controller that something went wrong. The error is logged when
    // it is created.
    tree->GetSwitchInterface()
        ->RetrieveValue(/*node_id*/ 0, req, &writer, /* details= */ nullptr)
        .IgnoreError();
    return SendResponse(GetResponse(path, resp), stream);
  };
  auto on_change_functor = UnsupportedFunc();
  node->SetOnPollHandler(poll_functor)
      ->SetOnTimerHandler(poll_functor)
      ->SetOnChangeHandler(on_change_functor);
}

////////////////////////////////////////////////////////////////////////////////
// /ipsec-offload/sad/sad-entry/config
void SetUpIPsecSAConfig(TreeNode* node, YangParseTree* tree) {
  auto unsupported_functor = UnsupportedFunc();

  auto on_set_functor =
      [node, tree](const ::gnmi::Path& path,
                   const ::google::protobuf::Message& val,
                   CopyOnWriteChassisConfig* config) -> ::util::Status {
    const gnmi::TypedValue* typed_val =
        dynamic_cast<const gnmi::TypedValue*>(&val);
    if (typed_val == nullptr) {
      return MAKE_ERROR(ERR_INVALID_PARAM) << "not a TypedValue message!";
    }
    if (typed_val->value_case() != gnmi::TypedValue::kProtoBytes) {
      return MAKE_ERROR(ERR_INVALID_PARAM) << "Expects a Proto Bytes stream!";
    }

    // printf("on_set_functor called with:\n%s\n",
    //                  typed_val->ShortDebugString().c_str());

    // Conversion from yang proto to common proto
    IPsecSADBConfig msg;
    RETURN_IF_ERROR(ParseProtoFromString(typed_val->proto_bytes(), &msg));

    // Send the message through SwitchInterface to TdiIpsecManager
    RETURN_IF_ERROR(
        SetValue(tree, IPsecSadbConfigOp::IPSEC_SADB_CONFIG_OP_ADD_ENTRY, msg));

    // In gnmi_publisher.cc::HandleUpdate() we had stripped the key from the
    // incoming gnmi::Path message (because this tree node is generic for all
    // Ipsec ConfigSADB messages). In order to return the correct path back
    // to gnmi client, re-add the key back to the path.
    // Re-add the key to path
    ::gnmi::Path* path_ptr = const_cast<::gnmi::Path*>(&path);
    auto* elem = path_ptr->mutable_elem(2);
    (*elem->mutable_key())["offload-id"] = std::to_string(msg.offload_id());
    (*elem->mutable_key())["direction"] = std::to_string(msg.direction());

    return ::util::OkStatus();
  };

  auto on_del_functor =
      [node, tree](const ::gnmi::Path& path,
                   const std::vector<std::string>& val,
                   CopyOnWriteChassisConfig* config) -> ::util::Status {
    // Needed for re-adding keys back to path
    ::gnmi::Path* path_ptr = const_cast<::gnmi::Path*>(&path);
    auto* elem = path_ptr->mutable_elem(2);

    // TODO: remove later after gnmi_cli is fixed
    // gnmi_cli does not support multiple keys. For now, will make two
    // calls to delete in both directions if only one key passed.
    // Note: if using a gnmi client which supports multiple keys,
    // will work as expected

    // Delete op in TdiFixedFunctionManager::WriteSadbEntry() only accesses
    // the offload_id and direction, so create a IPsecSADBConfig with the
    // offload_id and direction set and call writeConfigSADEntry

    uint32 offload_id = static_cast<uint32>(std::stoul(val.at(0)));
    IPsecSADBConfig msg;
    msg.set_offload_id(offload_id);

    if (val.size() == 1) {
      msg.set_direction(true);
      // Send the message through SwitchInterface to TdiIpsecManager
      RETURN_IF_ERROR(SetValue(
          tree, IPsecSadbConfigOp::IPSEC_SADB_CONFIG_OP_DEL_ENTRY, msg));
      msg.set_direction(false);
      RETURN_IF_ERROR(SetValue(
          tree, IPsecSadbConfigOp::IPSEC_SADB_CONFIG_OP_DEL_ENTRY, msg));

      // Update response path
      (*elem->mutable_key())["offload-id"] = val.at(0);
    } else if (val.size() == 2) {
      bool direction = (val.at(1).compare("1") == 0) ? true : false;
      msg.set_direction(direction);
      RETURN_IF_ERROR(SetValue(
          tree, IPsecSadbConfigOp::IPSEC_SADB_CONFIG_OP_DEL_ENTRY, msg));
      // Update response path
      (*elem->mutable_key())["offload-id"] = val.at(0);
      (*elem->mutable_key())["direction"] = val.at(1);
    } else {
      LOG(ERROR) << "Unexpected number of keys in IPsec delete handler";
    }

    return ::util::OkStatus();
  };

  node->SetOnPollHandler(unsupported_functor)
      ->SetOnTimerHandler(unsupported_functor)
      ->SetOnUpdateHandler(on_set_functor)
      ->SetOnReplaceHandler(on_set_functor)
      ->SetOnDeleteWithValHandler(on_del_functor)
      ->SetOnChangeHandler(unsupported_functor);
}

////////////////////////////////////////////////////////////////////////////////
// /ipsec-offload/sad/sad-entrys/state
void SetUpIPsecSADEntryState(TreeNode* node, YangParseTree* tree) {
  // TODO: Unsupported feature for now
  auto poll_functor = UnsupportedFunc();
  node->SetOnTimerHandler(poll_functor)
      ->SetOnPollHandler(poll_functor)
      ->SetOnChangeHandler(poll_functor);
}

////////////////////////////////////////////////////////////////////////////////
// /ipsec-offload
// (used for IPsec notifications)
void SetUpIPsecNotification(TreeNode* node, YangParseTree* tree) {
  auto unsupported_functor = UnsupportedFunc();
  auto register_functor = RegisterFunc<IPsecNotificationEvent>();
  auto on_change_functor =
      GetOnChangeFunctor(&IPsecNotificationEvent::GetNotification,
                         ConvertIPsecNotificationToString);
  node->SetOnTimerHandler(unsupported_functor)
      ->SetOnPollHandler(unsupported_functor)
      ->SetOnChangeRegistration(register_functor)
      ->SetOnChangeHandler(on_change_functor);
}

}  // namespace

////////////////////////
//  AddSubtreeIPsec   //
////////////////////////

void YangParseTreePaths::AddSubtreeIPsec(YangParseTree* tree) {
  TreeNode* node =
      tree->AddNode(GetPath("ipsec-offload")("ipsec-spi")("rx-spi")());
  SetUpIPsecFetchSPI(node, tree);
  node =
      tree->AddNode(GetPath("ipsec-offload")("sad")("sad-entry")("config")());
  SetUpIPsecSAConfig(node, tree);
  node = tree->AddNode(GetPath("ipsec-offload")("sad")("sad-entry")("state")());
  SetUpIPsecSADEntryState(node, tree);
  node = tree->AddNode(
      GetPath("ipsec-offload")("sadb-expire")());  // IPsec notification support
  SetUpIPsecNotification(node, tree);
}

}  // namespace hal
}  // namespace stratum
