// Copyright 2019-present Open Networking Foundation
// Copyright 2021-2024 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

#include <csignal>
#include <iostream>
#include <memory>
#include <regex>  // NOLINT
#include <string>
#include <vector>

#include "../client_cert_options.h"
#include "absl/cleanup/cleanup.h"
#include "gflags/gflags.h"
#include "gnmi/gnmi.grpc.pb.h"
#include "gnmi_ctl_utils.h"
#include "grpcpp/grpcpp.h"
#include "grpcpp/security/credentials.h"
#include "grpcpp/security/tls_credentials_options.h"
#include "re2/re2.h"
#include "stratum/glue/init_google.h"
#include "stratum/glue/status/status.h"
#include "stratum/glue/status/status_macros.h"
#include "stratum/lib/constants.h"
#include "stratum/lib/macros.h"
#include "stratum/lib/security/credentials_manager.h"
#include "stratum/lib/utils.h"

DEFINE_bool(grpc_use_insecure_mode, false,
            "grpc communication channel in insecure mode");
DECLARE_bool(grpc_use_insecure_mode);

const char kUsage[] =
    R"USAGE(usage: gnmi-ctl [Options] {get,set,cap,sub-onchange,sub-sample} parameters

Basic gNMI CLI

positional arguments:
  {get,set,cap,sub-onchange,sub-sample}         gNMI command
  parameter                                     gNMI config parameters

parameters:
  "device:<type>,name=<name>,<key:value>"

example:
   gnmi-ctl set "device:<type>,name=<name>,<key:value>,<key:value>,..."
   gnmi-ctl get "device:<type>,name=<name>,key"
)USAGE";

// Pipe file descriptors used to transfer signals from the handler to the cancel
// function.
int pipe_read_fd_ = -1;
int pipe_write_fd_ = -1;

// Pointer to the client context to cancel the blocking calls.
grpc::ClientContext* ctx_ = nullptr;

#define GNMI_PREDICT_ERROR(x) (__builtin_expect(false || (x), false))

#define PRINT_MSG(msg, prompt)                   \
  do {                                           \
    std::cout << prompt << std::endl;            \
    std::cout << msg.DebugString() << std::endl; \
  } while (0)

#define RETURN_IF_GRPC_ERROR(expr)                                           \
  do {                                                                       \
    const ::grpc::Status _grpc_status = (expr);                              \
    if (ABSL_PREDICT_FALSE(!_grpc_status.ok() &&                             \
                           _grpc_status.error_code() != grpc::CANCELLED)) {  \
      ::util::Status _status(                                                \
          static_cast<::util::error::Code>(_grpc_status.error_code()),       \
          _grpc_status.error_message());                                     \
      LOG(ERROR) << "Return Error: " << #expr << " failed with " << _status; \
      return _status;                                                        \
    }                                                                        \
  } while (0)

void HandleSignal(int signal) {
  static_assert(sizeof(signal) <= PIPE_BUF,
                "PIPE_BUF is smaller than the number of bytes that can be "
                "written atomically to a pipe.");
  // We must restore any changes made to errno at the end of the handler:
  // https://www.gnu.org/software/libc/manual/html_node/POSIX-Safety-Concepts.html
  int saved_errno = errno;
  // No reasonable error handling possible.
  write(pipe_write_fd_, &signal, sizeof(signal));
  errno = saved_errno;
}

void* ContextCancelThreadFunc(void*) {
  int signal_value;
  int ret = read(pipe_read_fd_, &signal_value, sizeof(signal_value));
  if (ret == 0) {  // Pipe has been closed.
    return nullptr;
  } else if (ret != sizeof(signal_value)) {
    LOG(ERROR) << "Error reading complete signal from pipe: " << ret << ": "
               << strerror(errno);
    return nullptr;
  }
  if (ctx_) ctx_->TryCancel();
  LOG(INFO) << "Client context cancelled.";
  return nullptr;
}

#define MAX_STR_LENGTH 128

// FIXME: For now it is connecting to localhost and later it has to be fixed
// to support any host, by providing a CLI option to the user.
DEFINE_string(grpc_addr, "localhost:9339", "gNMI server address");

DEFINE_uint64(interval, 5000, "Subscribe poll interval in ms");
DEFINE_bool(replace, false, "Use replace instead of update");
DEFINE_string(get_type, "ALL", "The gNMI get request type");

DEFINE_string(root_node, "/interfaces/", "The gNMI root node name");
DEFINE_string(device_key, "device", "The gNMI cli device key");
DEFINE_string(device_type_virtual_interface, "virtual-interface",
              "The gNMI cli device type");

DEFINE_string(name_key, "name", "The gNMI cli name key");
DEFINE_string(subtree_config, "config", "The gNMI path subtree of type config");

namespace gnmi {

using namespace stratum;

void add_path_elem(std::string elem_name, std::string elem_kv,
                   ::gnmi::PathElem* elem) {
  elem->set_name(elem_name);
  if (!elem_kv.empty()) {
    std::regex ex("\\[([^=]+)=([^\\]]+)\\]");
    std::smatch sm;
    std::regex_match(elem_kv, sm, ex);
    (*elem->mutable_key())[sm.str(1)] = sm.str(2);
  }
}

void build_gnmi_path(std::string path_str, ::gnmi::Path* path) {
  try {
    std::regex ex("/([^/\\[]+)(\\[([^=]+=[^\\]]+)\\])?");
    std::sregex_iterator iter(path_str.begin(), path_str.end(), ex);
    std::sregex_iterator end;
    while (iter != end) {
      std::smatch sm = *iter;
      auto* elem = path->add_elem();
      add_path_elem(sm.str(1), sm.str(2), elem);
      iter++;
    }
  } catch (std::exception& e) {
    std::cout << "An exception occurred in build_gnmi_path. Exception nr "
              << e.what() << std::endl;
  }
}

::gnmi::GetRequest build_gnmi_get_req(std::string path) {
  ::gnmi::GetRequest req;
  build_gnmi_path(path, req.add_path());
  req.set_encoding(::gnmi::PROTO);
  ::gnmi::GetRequest::DataType data_type;
  if (!::gnmi::GetRequest::DataType_Parse(FLAGS_get_type, &data_type)) {
    std::cout << "Invalid gNMI get data type: " << FLAGS_get_type
              << " , use ALL as data type." << std::endl;
    data_type = ::gnmi::GetRequest::ALL;
  }
  req.set_type(data_type);
  return req;
}

::gnmi::SetRequest build_gnmi_set_req(std::string path, std::string val) {
  ::gnmi::SetRequest req;
  ::gnmi::Update* update;
  char* check;

  if (FLAGS_replace) {
    update = req.add_replace();
  } else {
    update = req.add_update();
  }
  build_gnmi_path(path, update->mutable_path());
  strtol(val.c_str(), &check, 10);
  if (*check) {
    update->mutable_val()->set_string_val(val);
  } else {
    update->mutable_val()->set_int_val(stoull(val));
  }
  return req;
}

::gnmi::SetRequest build_gnmi_del_req(std::string path) {
  ::gnmi::SetRequest req;
  auto* del = req.add_delete_();
  build_gnmi_path(path, del);
  return req;
}

::gnmi::SubscribeRequest build_gnmi_sub_onchange_req(std::string path) {
  ::gnmi::SubscribeRequest sub_req;
  auto* sub_list = sub_req.mutable_subscribe();
  sub_list->set_mode(::gnmi::SubscriptionList::STREAM);
  sub_list->set_updates_only(true);
  auto* sub = sub_list->add_subscription();
  sub->set_mode(::gnmi::ON_CHANGE);
  build_gnmi_path(path, sub->mutable_path());
  return sub_req;
}

::gnmi::SubscribeRequest build_gnmi_sub_sample_req(
    std::string path, ::google::protobuf::uint64 interval) {
  ::gnmi::SubscribeRequest sub_req;
  auto* sub_list = sub_req.mutable_subscribe();
  sub_list->set_mode(::gnmi::SubscriptionList::STREAM);
  sub_list->set_updates_only(true);
  auto* sub = sub_list->add_subscription();
  sub->set_mode(::gnmi::SAMPLE);
  sub->set_sample_interval(interval);
  build_gnmi_path(path, sub->mutable_path());
  return sub_req;
}

bool extract_interface_node(char** path, char* node_path, bool* vhost_dev) {
  char* key = NULL;
  char* value = NULL;
  int found_node = 0;

  while (client_parse_key_value(path, &key, &value)) {
    if (strcmp(key, FLAGS_device_key.c_str()) == 0) {
      snprintf(node_path + strlen(node_path),
               strlen(FLAGS_device_type_virtual_interface.c_str()) + 1, "%s",
               FLAGS_device_type_virtual_interface.c_str());
      // Validate the device value
      if ((strcmp(value, "virtual-device") != 0) &&
          (strcmp(value, "physical-device") != 0)) {
        return -1;
      }
      if (strcmp(value, "virtual-device") == 0) {
        std::cout << "setting vhost_dev = true.";
        *vhost_dev = true;
      }
      found_node += 1;
    }
    if (strcmp(key, FLAGS_name_key.c_str()) == 0) {
      // Hardcoded length of "[name=]/"
      snprintf(node_path + strlen(node_path), strlen(value) + 9, "[name=%s]/",
               value);
      found_node += 1;
    }
    if (found_node == 2) return 0;
  }
  return -1;
}

void traverse_params(char** path, char* node_path, char* config_value,
                     bool& flag) {
  char* key = NULL;
  char* value = NULL;

  if (client_parse_key_value(path, &key, &value)) {
    if ((value != NULL) && value[0] != '\0') {
      // This should be executed for a <key=value> pair, specifically for
      // SET operation.
      snprintf(node_path + strlen(node_path),
               strlen(FLAGS_subtree_config.c_str()) + strlen(key) + 2, "%s/%s",
               FLAGS_subtree_config.c_str(), key);
      strcpy(config_value, value);
      return;
    } else if (key != NULL && key[0] != '\0') {
      // This should be executed for a <key>, specifically for
      // GET operation.
      snprintf(node_path + strlen(node_path),
               strlen(FLAGS_subtree_config.c_str()) + strlen(key) + 2, "%s/%s",
               FLAGS_subtree_config.c_str(), key);
      return;
    }
  }
  flag = false;
  return;
}

::grpc::ClientReaderWriterInterface<
    ::gnmi::SubscribeRequest, ::gnmi::SubscribeResponse>* stream_reader_writer;

::util::Status Main(int argc, char** argv) {
  // Default certificate file location for TLS-mode
  set_client_cert_defaults();

  // Parse command line flags
  gflags::ParseCommandLineFlags(&argc, &argv, true);

  ::gflags::SetUsageMessage(kUsage);
  InitGoogle(argv[0], &argc, &argv, true);
  stratum::InitStratumLogging();

  bool vhost_device = false;
  if (argc < 2) {
    std::cout << kUsage << std::endl;
    return MAKE_ERROR(ERR_INVALID_PARAM) << "Invalid number of arguments.";
  }

  ::grpc::ClientContext ctx;
  ctx_ = &ctx;
  // Create the pipe to transfer signals.
  {
    RETURN_IF_ERROR(
        CreatePipeForSignalHandling(&pipe_read_fd_, &pipe_write_fd_));
  }
  RET_CHECK(std::signal(SIGINT, HandleSignal) != SIG_ERR);
  pthread_t context_cancel_tid;
  RET_CHECK(pthread_create(&context_cancel_tid, nullptr,
                           ContextCancelThreadFunc, nullptr) == 0);
  auto cleaner = absl::MakeCleanup([&context_cancel_tid, &ctx] {
    int signal = SIGINT;
    write(pipe_write_fd_, &signal, sizeof(signal));
    if (pthread_join(context_cancel_tid, nullptr) != 0) {
      LOG(ERROR) << "Failed to join the context cancel thread.";
    }
    close(pipe_write_fd_);
    close(pipe_read_fd_);
    // We call this to synchronize the internal client context state.
    ctx.TryCancel();
  });

  std::shared_ptr<::grpc::Channel> channel;
  if (FLAGS_grpc_use_insecure_mode) {
    std::shared_ptr<::grpc::ChannelCredentials> channel_credentials =
        ::grpc::InsecureChannelCredentials();
    channel = ::grpc::CreateChannel(FLAGS_grpc_addr, channel_credentials);
  } else {
    ASSIGN_OR_RETURN(auto credentials_manager,
                     CredentialsManager::CreateInstance(true));
    channel = ::grpc::CreateChannel(
        FLAGS_grpc_addr,
        credentials_manager->GenerateExternalFacingClientCredentials());
  }
  std::string cmd = std::string(argv[1]);

  if (cmd == "cap") {
    auto stub = ::gnmi::gNMI::NewStub(channel);
    ::grpc::ClientContext ctx;
    ::gnmi::CapabilityRequest req;
    PRINT_MSG(req, "REQUEST");
    ::gnmi::CapabilityResponse resp;
    RETURN_IF_GRPC_ERROR(stub->Capabilities(&ctx, req, &resp));
    PRINT_MSG(resp, "RESPONSE");
    return ::util::OkStatus();
  }

  if (argc < 3) {
    std::cout << "Missing path for " << cmd << " request\n";
    return ::util::OkStatus();
  }

  char* path = argv[2];
  char buffer[MAX_STR_LENGTH];
  bool params = true;

  client_strzcpy(buffer, FLAGS_root_node.c_str(), MAX_STR_LENGTH);
  if (extract_interface_node(&path, buffer, &vhost_device)) {
    std::cout << "Invalid device and name information\n";
    return ::util::OkStatus();
  }

  if (cmd == "get") {
    auto stub = ::gnmi::gNMI::NewStub(channel);
    ::grpc::ClientContext ctx;
    char path1[MAX_STR_LENGTH] = {0};
    char config_value[MAX_STR_LENGTH] = {0};

    strcpy(path1, buffer);
    traverse_params(&path, path1, config_value, params);
    ::gnmi::GetRequest req = build_gnmi_get_req(path1);
    ::gnmi::GetResponse resp;
    RETURN_IF_GRPC_ERROR(stub->Get(&ctx, req, &resp));
    PRINT_MSG(resp, "Get Response from Server");
  } else if (cmd == "set") {
    while (params) {
      char path1[MAX_STR_LENGTH] = {0};
      char config_value[MAX_STR_LENGTH] = {0};

      strcpy(path1, buffer);
      traverse_params(&path, path1, config_value, params);
      // If device is 'virtual-device' and port type is 'link', consider it a
      // 'vhost' type.
      if (((strcmp(config_value, "link") == 0) ||
           (strcmp(config_value, "LINK") == 0)) &&
          vhost_device) {
        strcpy(config_value, "vhost");
      }
      if (params) {
        auto stub = ::gnmi::gNMI::NewStub(channel);
        ::grpc::ClientContext ctx;
        ::gnmi::SetRequest req = build_gnmi_set_req(path1, config_value);
        ::gnmi::SetResponse resp;
        RETURN_IF_GRPC_ERROR(stub->Set(&ctx, req, &resp));
      }
    }
    std::cout << "Set request, successful...!!!" << std::endl;
  } else if (cmd == "del") {
    auto stub = ::gnmi::gNMI::NewStub(channel);
    ::grpc::ClientContext ctx;
    ::gnmi::SetRequest req = build_gnmi_del_req(path);
    ::gnmi::SetResponse resp;
    RETURN_IF_GRPC_ERROR(stub->Set(&ctx, req, &resp));
    PRINT_MSG(resp, "RESPONSE");
  } else if (cmd == "sub-onchange") {
    auto stub = ::gnmi::gNMI::NewStub(channel);
    ::grpc::ClientContext ctx;
    auto stream_reader_writer_ptr = stub->Subscribe(&ctx);
    stream_reader_writer = stream_reader_writer_ptr.get();
    ::gnmi::SubscribeRequest req = build_gnmi_sub_onchange_req(path);
    // RET_CHECK(stream_reader_writer->Write(req))
    //     << "Cannot write request.";
    stream_reader_writer->Write(req);
    ::gnmi::SubscribeResponse resp;
    while (stream_reader_writer->Read(&resp)) {
      PRINT_MSG(resp, "RESPONSE");
    }
    RETURN_IF_GRPC_ERROR(stream_reader_writer->Finish());
  } else if (cmd == "sub-sample") {
    auto stub = ::gnmi::gNMI::NewStub(channel);
    ::grpc::ClientContext ctx;
    auto stream_reader_writer_ptr = stub->Subscribe(&ctx);
    stream_reader_writer = stream_reader_writer_ptr.get();
    ::gnmi::SubscribeRequest req =
        build_gnmi_sub_sample_req(path, FLAGS_interval);
    // RET_CHECK(stream_reader_writer->Write(req))
    //     << "Cannot write request.";
    stream_reader_writer->Write(req);
    ::gnmi::SubscribeResponse resp;
    while (stream_reader_writer->Read(&resp)) {
      PRINT_MSG(resp, "RESPONSE");
    }
    RETURN_IF_GRPC_ERROR(stream_reader_writer->Finish());
  } else {
    return MAKE_ERROR(ERR_INVALID_PARAM) << "Unknown command: " << cmd;
  }

  return ::util::OkStatus();
}

}  // namespace gnmi

int main(int argc, char** argv) { return gnmi::Main(argc, argv).error_code(); }
