#pragma once

#include <grpc++/grpc++.h>

#include <iostream>
#include <queue>
#include <thread>
#include <unordered_map>
#include <vector>

#include "ServiceDiscovery.grpc.pb.h"
#include "ServiceDiscovery.pb.h"

namespace Camfer {
class Client {
 public:
  Client(std::string serviceName, std::string serviceKey)
      : serviceName_(serviceName), serviceKey_(serviceKey) {
  }

  ~Client() { cq_.Shutdown(); }

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

  void write(psychopomp::ClientMessage msg) {
    addToWriteQueue(msg);
    attemptWrite();
  }

 private:
  enum class Operation {
    CONNECT = 0,
    READ = 1,
    WRITE = 2,
    FINISH = 3,
  };

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
        context_.get(), &cq_, reinterpret_cast<void*>(Operation::CONNECT));
  }

  void runIoLoop() {
    void* tag;
    bool ok;
    while (true) {
      bool shutdown = !cq_.Next(&tag, &ok);
      if (shutdown) {
        break;
      }

      std::cout << "Processing tag" << std::endl;
      process(tag, ok);
    }
  }

  void process(void* tag, bool ok) {
    auto op = Operation(reinterpret_cast<size_t>(tag));
    switch (op) {
      case Operation::CONNECT: {
        if (!ok) {
          std::cout << "Not connected" << std::endl;
          std::this_thread::sleep_for(std::chrono::seconds(5));
          connect();
          break;
        }
        std::cout << "Connected" << std::endl;
        isConnected_ = true;

        stream_->Read(&message_, reinterpret_cast<void*>(Operation::READ));

        sendAuthMessage();
        break;
      }
      case Operation::READ:
        std::cout << "Read message" << std::endl;

        // Create thread to handle request
        if (!ok) {
          std::cout << "Unsucessful read" << std::endl;
          std::cout << "Client disconnected" << std::endl;
          break;
        }
        stream_->Read(&message_, reinterpret_cast<void*>(Operation::READ));
        break;
      case Operation::WRITE:
        finishWrite(ok);
        if (ok) {
          attemptWrite();
        }
        break;
      case Operation::FINISH:
        std::cout << "Client finished" << std::endl;
        isConnected_ = false;
        break;
    }
  }

  void removeDestructThread(std::thread::id id) {
    auto guard = std::lock_guard<std::mutex>(stateLock_);
    threads_.erase(id);
  }

  void addToWriteQueue(psychopomp::ClientMessage msg) {
    auto guard = std::lock_guard<std::mutex>(stateLock_);
    msgQueue_.push(std::move(msg));
  }

  void finishWrite(bool ok) {
    std::cout << "Write finished" << std::endl;
    auto guard = std::lock_guard<std::mutex>(stateLock_);
    if (ok && !msgQueue_.empty()) {
      msgQueue_.pop();
    }
    isWriting_ = false;
  }

  void attemptWrite() {
    auto guard = std::lock_guard<std::mutex>(stateLock_);
    std::cout << "Writing next message" << std::endl;
    if (msgQueue_.empty() || isWriting_) {
      return;
    }
    isWriting_ = true;
    stream_->Write(msgQueue_.front(),
                   reinterpret_cast<void*>(Operation::WRITE));
  }

  void setConnectedState(bool isConnected) {
    auto guard = std::lock_guard<std::mutex>(stateLock_);
    isConnected_ = isConnected;
  }

  void sendAuthMessage() {
    std::cout << "Sending auth message" << std::endl;
    psychopomp::ClientMessage msg;
    auto& connRequest = *msg.mutable_connection_request();
    connRequest.set_service_name(serviceName_);
    connRequest.set_service_key(serviceKey_);
    connRequest.set_bin_name("test bin");
    write(msg);
  }

  // Threading
  
  // Should be destructed last
  std::mutex stateLock_;
  std::unordered_map<std::thread::id, std::shared_ptr<std::thread>> threads_;

  // Config
  std::string serviceName_;
  std::string serviceKey_;

  // Grpc
  std::shared_ptr<grpc::Channel> channel_;
  std::unique_ptr<psychopomp::Psychopomp::Stub> stub_;
  std::unique_ptr<grpc::ClientContext> context_;
  grpc::CompletionQueue cq_;

  // IO
  std::unique_ptr<grpc::ClientAsyncReaderWriter<psychopomp::ClientMessage,
                                                psychopomp::ServerMessage>>
      stream_;
  psychopomp::ServerMessage message_;
  std::queue<psychopomp::ClientMessage> msgQueue_;

  // Client logic
  std::chrono::time_point<std::chrono::system_clock> lastConnectionAttempt_;

  bool isConnected_;
  bool isWriting_;
};
}  // namespace Camfer