#pragma once

#include <grpc++/grpc++.h>

#include <iostream>
#include <map>
#include <queue>
#include <thread>
#include <vector>

#include "ServiceDiscovery.grpc.pb.h"
#include "ServiceDiscovery.pb.h"
#include "server_utils/RequestHandler.h"

namespace {
template <typename T>
std::vector<T> copyRepeatedField(
    const google::protobuf::RepeatedPtrField<T>& field) {
  return std::vector<T>(field.begin(), field.end());
}
}  // namespace

namespace Camfer {

using server_utils::Operation;

class Client : server_utils::RequestHandler<psychopomp::ClientMessage,
                                            psychopomp::ServerMessage> {
 public:
  Client(std::string serviceName, std::string serviceKey)
      : serviceName_(serviceName), serviceKey_(serviceKey) {}

  ~Client() {
    if (context_) {
      context_->TryCancel();
    }
    if (stream_) {
      grpc::Status* status;
      stream_->Finish(status, getOpTag(Operation::FINISH));
    }
    cq_.Shutdown();
    std::lock_guard<std::mutex> lockGuard(threadStateLock_);
    for (auto [id, thread] : threads_) {
      thread->join();
    }
  }

  void start() {
    auto ioThread = std::make_shared<std::thread>([&]() {
      connect();
      runIoLoop();
    });
    threads_.emplace(ioThread->get_id(), ioThread);
  }

  virtual bool updateRanges(
      std::vector<psychopomp::ShardRange> assignedRanges,
      std::vector<psychopomp::ShardRange> unassignedRanges) {
    return true;
  };

  virtual bool forwardRanges(std::vector<psychopomp::ShardRange> ranges,
                             std::vector<std::string> nextBins) {
    return true;
  };

 private:
  void connect() {
    // TODO: Get target from relay service
    std::this_thread::sleep_until(lastConnectionAttempt_ +
                                  std::chrono::seconds(5));
    lastConnectionAttempt_ = std::chrono::system_clock::now();

    std::cout << "Attempting connect" << std::endl;
    grpc::ChannelArguments args;
    args.SetInt(GRPC_ARG_KEEPALIVE_TIME_MS, 2 * 1000 /*2 sec*/);
    args.SetInt(GRPC_ARG_KEEPALIVE_TIMEOUT_MS, 5 * 1000 /*5 sec*/);
    args.SetInt(GRPC_ARG_KEEPALIVE_PERMIT_WITHOUT_CALLS, 1);
    channel_ = grpc::CreateCustomChannel(
        "0.0.0.0:50051", grpc::InsecureChannelCredentials(), args);

    stub_ = psychopomp::Psychopomp::NewStub(channel_);
    context_ = std::make_unique<grpc::ClientContext>();
    stream_ = stub_->AsyncRegisterStream(
        context_.get(), &cq_, getOpTag(Operation::CONNECT));
  }

  void runIoLoop() {
    void* tag;
    bool ok;
    while (true) {
      bool shutdown = !cq_.Next(&tag, &ok);
      if (shutdown) {
        std::cout << "Shutting down" << std::endl;
        break;
      }

      std::cout << "Processing tag" << std::endl;
      process(Operation(reinterpret_cast<size_t>(tag)), ok);
    }
  }

  virtual void handleConnect(bool ok) override {
    if (!ok) {
      std::cout << "Not connected" << std::endl;
      std::this_thread::sleep_for(std::chrono::seconds(5));
      connect();
      return;
    }
    std::cout << "Connected" << std::endl;
    setConnectedState(true);
    
    attemptRead();
    sendAuthMessage();
  };

  virtual void handleRead(bool ok, bool& shouldAttemptNext) override {
    std::cout << "Read message" << std::endl;

    // Create thread to handle request
    if (!ok) {
      std::cout << "Client disconnected" << std::endl;
      return;
    }
    // Process messages for updates that need to occur sequentially
    auto& message = getMessage();
    processMessageSequentially(message);

    // Create copy of message_ and process async
    auto processThread = std::make_shared<std::thread>(
        [&, message = message]() { processMessageAsync(message); });
    threads_.emplace(processThread->get_id(), processThread);

    shouldAttemptNext = true;
  };

  virtual void handleWrite(bool ok, bool& shouldAttemptNext) override {
    shouldAttemptNext = ok;
  };

  virtual void handleFinish(bool ok) override {
    std::cout << "Client finished" << std::endl;
    setConnectedState(false);
  };

  virtual void readFromStream() override {
    stream_->Read(&getMessage(), getOpTag(Operation::READ));
  };

  virtual void writeToStream(const psychopomp::ClientMessage& msg) override {
    stream_->Write(msg, getOpTag(Operation::WRITE));
  }

  virtual void* getOpTag(Operation op) const override {
    return reinterpret_cast<void*>(op);
  }

  void processMessageSequentially(const psychopomp::ServerMessage& message) {
    std::cout << "Processing message sync" << std::endl;
    if (message.has_shard_update_request()) {
      const auto& shardUpdateRequest = message.shard_update_request();
      auto unassignedRanges =
          copyRepeatedField(shardUpdateRequest.unassigned_ranges());
      setShardStatus(shardRangePendingStatus_, unassignedRanges,
                     psychopomp::SHARD_RANGE_STATUS_PENDING_DROPPED);

      auto assignedRanges =
          copyRepeatedField(shardUpdateRequest.assigned_ranges());
      setShardStatus(shardRangePendingStatus_, assignedRanges,
                     psychopomp::SHARD_RANGE_STATUS_PENDING_ADDED);
    } else if (message.has_shard_forward_request()) {
      const auto& shardForwardRequest = message.shard_forward_request();
      auto forwardedRanges =
          copyRepeatedField(shardForwardRequest.forwarded_ranges());
      setShardStatus(shardRangePendingStatus_, forwardedRanges,
                     psychopomp::SHARD_RANGE_STATUS_PENDING_FORWARDING);
    }
  }

  void processMessageAsync(psychopomp::ServerMessage message) {
    std::cout << "Processing message async" << std::endl;
    if (message.has_bin_state_request()) {
      const auto& binStateRequest = message.bin_state_request();

      // Send back bin state

    } else if (message.has_connection_response()) {
      const auto& connectionResponse = message.connection_response();
      // Log response
      if (connectionResponse.success()) {
        std::cout << "Connection successful response" << std::endl;
      } else {
        std::cout << "Connection rejected" << std::endl;
      }
    } else if (message.has_shard_update_request()) {
      const auto& shardUpdateRequest = message.shard_update_request();

      auto unassignedRanges =
          copyRepeatedField(shardUpdateRequest.unassigned_ranges());
      auto assignedRanges =
          copyRepeatedField(shardUpdateRequest.assigned_ranges());

      bool success = updateRanges(unassignedRanges, assignedRanges);

      // Update shard statuses
      if (success) {
        setShardStatus(shardRangeStatus_, unassignedRanges,
                       psychopomp::SHARD_RANGE_STATUS_DROPPED);
        setShardStatus(shardRangeStatus_, assignedRanges,
                       psychopomp::SHARD_RANGE_STATUS_ADDED);
      } else {
        setShardStatus(shardRangePendingStatus_, unassignedRanges,
                       psychopomp::SHARD_RANGE_STATUS_DROPPED);
      }

      std::vector<psychopomp::ShardRange> ranges(unassignedRanges);
      ranges.insert(ranges.end(), assignedRanges.begin(), assignedRanges.end());
      sendShardStatusUpdate(ranges);
    } else if (message.has_shard_forward_request()) {
      const auto& shardForwardRequest = message.shard_forward_request();
      auto forwardedRanges =
          copyRepeatedField(shardForwardRequest.forwarded_ranges());
      std::vector<std::string> nextBins(shardForwardRequest.next_bins().begin(),
                                        shardForwardRequest.next_bins().end());

      bool success = forwardRanges(forwardedRanges, nextBins);

      // Update shard statuses
      if (success) {
        setShardStatus(shardRangeStatus_, forwardedRanges,
                       psychopomp::SHARD_RANGE_STATUS_FORWARDING);
      } else {
        setShardStatus(shardRangePendingStatus_, forwardedRanges,
                       psychopomp::SHARD_RANGE_STATUS_DROPPED);
      }
      sendShardStatusUpdate(forwardedRanges);
    }
  }

  void removeDestructThread(std::thread::id id) {
    std::lock_guard<std::mutex> lockGuard(threadStateLock_);
    threads_.erase(id);
  }

  bool getConnectedState() {
    std::lock_guard<std::mutex> lockGuard(stateLock_);
    return isConnected_;
  }

  void setConnectedState(bool isConnected) {
    std::lock_guard<std::mutex> lockGuard(stateLock_);
    isConnected_ = isConnected;
  }

  void sendAuthMessage() {
    std::cout << "Sending auth message" << std::endl;
    psychopomp::ClientMessage msg;
    auto& connRequest = *msg.mutable_connection_request();
    connRequest.set_service_name(serviceName_);
    connRequest.set_service_key(serviceKey_);
    connRequest.set_bin_name("test bin");

    auto currentShardState = getShardStatus();
    auto& binState = *connRequest.mutable_bin_state();
    binState.set_status(psychopomp::BIN_STATUS_HEALTHY);

    auto& shardRangeInfos = *binState.mutable_shard_range_info();
    shardRangeInfos.Reserve(currentShardState.size());
    for (auto [rangePair, status] : currentShardState) {
      auto& shardRangeInfo = *shardRangeInfos.Add();
      shardRangeInfo.mutable_range()->set_range_start(rangePair.first);
      shardRangeInfo.mutable_range()->set_range_end(rangePair.second);
      shardRangeInfo.set_status(status);
    }

    write(msg);
  }

  void sendShardStatusUpdate(
      const std::vector<psychopomp::ShardRange>& ranges) {
    psychopomp::ClientMessage msg;
    auto& updateResponse = *msg.mutable_shard_status_update_response();
    auto statuses = getShardStatus(ranges);
    auto& shardRanges = *updateResponse.mutable_ranges();

    shardRanges.Reserve(ranges.size());
    for (const auto& range : ranges) {
      *shardRanges.Add() = range;
    }

    auto& shardRangeStatuses = *updateResponse.mutable_statuses();
    shardRangeStatuses.Reserve(statuses.size());
    for (const auto& status : statuses) {
      *shardRangeStatuses.Add() = status;
    }

    write(msg);
  }

  std::map<std::pair<int64_t, int64_t>, psychopomp::ShardRangeStatus>
  getShardStatus() {
    std::lock_guard<std::mutex> lockGuard(stateLock_);
    auto mapCopy = shardRangeStatus_;
    for (const auto& [range, status] : shardRangePendingStatus_) {
      mapCopy[range] = status;
    }
    return mapCopy;
  }

  std::vector<psychopomp::ShardRangeStatus> getShardStatus(
      const std::vector<psychopomp::ShardRange>& ranges) {
    std::vector<psychopomp::ShardRangeStatus> statuses;
    statuses.reserve(ranges.size());

    std::lock_guard<std::mutex> lockGuard(stateLock_);
    for (const auto& range : ranges) {
      auto rangePair = std::make_pair<int64_t, int64_t>(range.range_start(),
                                                        range.range_end());
      auto it = shardRangePendingStatus_.find(rangePair);
      if (it != shardRangePendingStatus_.end()) {
        statuses.push_back(it->second);
        continue;
      }

      it = shardRangeStatus_.find(rangePair);
      if (it != shardRangeStatus_.end()) {
        statuses.push_back(it->second);
        continue;
      }

      statuses.push_back(psychopomp::SHARD_RANGE_STATUS_DROPPED);
    }

    return statuses;
  }

  void setShardStatus(
      std::map<std::pair<int64_t, int64_t>, psychopomp::ShardRangeStatus>
          shardRangeStatusMap,
      const std::vector<psychopomp::ShardRange>& ranges,
      psychopomp::ShardRangeStatus status) {
    std::lock_guard<std::mutex> lockGuard(stateLock_);
    if (status == psychopomp::SHARD_RANGE_STATUS_DROPPED) {
      for (const auto& range : ranges) {
        shardRangeStatusMap.erase(std::make_pair<int64_t, int64_t>(
            range.range_start(), range.range_end()));
      }
    } else {
      for (const auto& range : ranges) {
        shardRangeStatusMap[std::make_pair<int64_t, int64_t>(
            range.range_start(), range.range_end())] = status;
      }
    }
  }

  // Threading
  // Should be destructed last
  std::mutex threadStateLock_;
  std::unordered_map<std::thread::id, std::shared_ptr<std::thread>> threads_;

  // Config
  std::string serviceName_;
  std::string serviceKey_;

  // Grpc
  std::shared_ptr<grpc::Channel> channel_;
  std::unique_ptr<psychopomp::Psychopomp::Stub> stub_;
  std::unique_ptr<grpc::ClientContext> context_;
  grpc::CompletionQueue cq_;
  std::unique_ptr<grpc::ClientAsyncReaderWriter<psychopomp::ClientMessage,
                                                psychopomp::ServerMessage>>
      stream_;

  // Client logic
  std::mutex stateLock_;
  std::chrono::time_point<std::chrono::system_clock> lastConnectionAttempt_;
  std::map<std::pair<int64_t, int64_t>, psychopomp::ShardRangeStatus>
      shardRangeStatus_;
  std::map<std::pair<int64_t, int64_t>, psychopomp::ShardRangeStatus>
      shardRangePendingStatus_;
  bool isConnected_;
};
}  // namespace Camfer