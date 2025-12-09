// Copyright 2025 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

// Implements the YangParseTreePaths::AddSubtreeVirtualPort() method and its
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

////////////////////////////////////////////////////////////////////////////////
// /virtual-ports/virtual-port/state/vsi
void SetUpVirtualPortFetchVSI(TreeNode* node, YangParseTree* tree) {
  auto get_functor = [tree](const ::gnmi::Path& path,
                            const std::vector<std::string>& val,
                            GnmiSubscribeStream* stream) {
    // Create a data retrieval request.
    DataRequest req;
    auto* request = req.add_requests()->mutable_vport_vsi();
    ::gnmi::Path* path_ptr = const_cast<::gnmi::Path*>(&path);

    uint32 global_resource_id = static_cast<uint32>(
        std::stoul(val.at(0)));  // only one will be present at index 0
    request->set_global_resource_id(global_resource_id);

    // In-place definition of method retrieving data from generic response
    // and saving into 'resp' local variable.
    uint64 resp{};
    DataResponseWriter writer([&resp](const DataResponse& in) {
      if (!in.has_vport_vsi()) return false;
      resp = in.vport_vsi().vsi();
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

  auto unsupported_functor = UnsupportedFunc();
  node->SetOnPollHandler(unsupported_functor)
      ->SetOnTimerHandler(unsupported_functor)
      ->SetOnGetWithValHandler(get_functor)
      ->SetOnChangeHandler(unsupported_functor);
}

////////////////////////////////////////////////////////////////////////////////
// /virtual-ports/virtual-port/state/oper-status
void SetUpVirtualPortFetchOperStatus(TreeNode* node, YangParseTree* tree) {
  auto get_functor = [tree](const ::gnmi::Path& path,
                            const std::vector<std::string>& val,
                            GnmiSubscribeStream* stream) {
    DataRequest req;
    auto* request = req.add_requests()->mutable_vport_oper_status();
    ::gnmi::Path* path_ptr = const_cast<::gnmi::Path*>(&path);

    uint32 global_resource_id = static_cast<uint32>(
        std::stoul(val.at(0)));  // only one will be present at index 0
    request->set_global_resource_id(global_resource_id);

    uint64 resp{};
    DataResponseWriter writer([&resp](const DataResponse& in) {
      if (!in.has_oper_status()) return false;
      resp = in.oper_status().state();
      return true;
    });
    tree->GetSwitchInterface()
        ->RetrieveValue(/*node_id*/ 0, req, &writer, /* details= */ nullptr)
        .IgnoreError();
    return SendResponse(GetResponse(path, resp), stream);
  };
  auto unsupported_functor = UnsupportedFunc();
  node->SetOnPollHandler(unsupported_functor)
      ->SetOnTimerHandler(unsupported_functor)
      ->SetOnGetWithValHandler(get_functor)
      ->SetOnChangeHandler(unsupported_functor);
}

////////////////////////////////////////////////////////////////////////////////
// /virtual-ports/virtual-port/state/mac-address
void SetUpVirtualPortFetchMacAddress(TreeNode* node, YangParseTree* tree) {
  auto get_functor = [tree](const ::gnmi::Path& path,
                            const std::vector<std::string>& val,
                            GnmiSubscribeStream* stream) {
    DataRequest req;
    auto* request = req.add_requests()->mutable_vport_mac_address();
    ::gnmi::Path* path_ptr = const_cast<::gnmi::Path*>(&path);

    uint32 global_resource_id = static_cast<uint32>(
        std::stoul(val.at(0)));  // only one will be present at index 0
    request->set_global_resource_id(global_resource_id);

    uint64 resp{};
    DataResponseWriter writer([&resp](const DataResponse& in) {
      if (!in.has_mac_address()) return false;
      resp = in.mac_address().mac_address();
      return true;
    });
    tree->GetSwitchInterface()
        ->RetrieveValue(/*node_id*/ 0, req, &writer, /* details= */ nullptr)
        .IgnoreError();
    return SendResponse(GetResponse(path, MacAddressToYangString(resp)),
                        stream);
  };

  auto unsupported_functor = UnsupportedFunc();
  node->SetOnPollHandler(unsupported_functor)
      ->SetOnTimerHandler(unsupported_functor)
      ->SetOnGetWithValHandler(get_functor)
      ->SetOnChangeHandler(unsupported_functor);
}

// Notification message that is sent to gnmi-client
std::string ConvertVportStateToString(const VportStateNotification& notif) {
  std::stringstream ss;
  ss << "global-resource-id: " << notif.global_resource_id();
  ss << ", oper-status: " << notif.state();
  return ss.str();
}

////////////////////////////////////////////////////////////////////////////////
// /virtual-ports/oper-status-change
void SetUpVportNotification(TreeNode* node, YangParseTree* tree) {
  auto unsupported_functor = UnsupportedFunc();
  auto register_functor = [tree](const EventHandlerRecordPtr& record) {
    // On register notification callback needs to be set in the underlying SDE.
    // If it was already enabled it will be a no-op call.

    // Create a set request.
    SetRequest req;
    auto* request = req.add_requests()->mutable_node();
    request->set_enable_vport_status_notif(true);

    std::vector<::util::Status> details;
    tree->GetSwitchInterface()
        ->SetValue(/*node_id*/ 0, req, &details)
        .IgnoreError();
    // Return status of the operation.
    if (details.at(0) != ::util::OkStatus()) {
      // Log error
    }

    return EventHandlerList<VportStateNotificationEvent>::GetInstance()
        ->Register(record);
  };
  auto on_change_functor = GetOnChangeFunctor(
      &VportStateNotificationEvent::GetNotification, ConvertVportStateToString);
  node->SetOnTimerHandler(unsupported_functor)
      ->SetOnPollHandler(unsupported_functor)
      ->SetOnChangeRegistration(register_functor)
      ->SetOnChangeHandler(on_change_functor);
}
}  // namespace

//////////////////////////////
//  AddSubtreeVirtualPort   //
//////////////////////////////

void YangParseTreePaths::AddSubtreeVirtualPort(YangParseTree* tree) {
  TreeNode* node =
      tree->AddNode(GetPath("virtual-ports")("virtual-port")("state")("vsi")());
  SetUpVirtualPortFetchVSI(node, tree);
  node = tree->AddNode(
      GetPath("virtual-ports")("virtual-port")("state")("oper-status")());
  SetUpVirtualPortFetchOperStatus(node, tree);
  node = tree->AddNode(
      GetPath("virtual-ports")("virtual-port")("state")("mac-address")());
  SetUpVirtualPortFetchMacAddress(node, tree);
  node = tree->AddNode(GetPath("virtual-ports")(
      "oper-status-change")());  // Port state (Up/Down) change notification
  SetUpVportNotification(node, tree);
}

}  // namespace hal
}  // namespace stratum
