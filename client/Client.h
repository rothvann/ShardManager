#pragma once

#include <queue>
#include <thread>
#include <unordered_map>
#include <vector>

#include "ServiceDiscovery.grpc.pb.h"

namespace Camfer {
class Client {
 public:
  Client(std::string serviceName, std::string serviceKey)
      : serviceName_(serviceName), serviceKey_(serviceKey) {
    context_.AsyncNotifyWhenDone(Operation::CANCELLED);
    connect();
  }

  ~Client() { cq_.Shutdown(); }

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
    CANCELLED = 4,
  };

  void connect() {
    // TODO: Get target from relay service
    channel_ = grpc::CreateChannel("localhost:50051",
                                   grpc::InsecureChannelCredentials());
    stub_ = psychopomp::Psychopomp::NewStub(channel_);
    stream_ = stub_->AsyncregisterStream(
        &context_, &cq_, reinterpret_cast<void*>(Operation::CONNECT));
  }

  void runIoLoop() {
    void* tag;
    bool ok;
    while (true) {
      bool shutdown = completionQueue->Next(&tag, &ok);
      if (shutdown) {
        break;
      }

      process(tag, ok);
    }
  }

  void process(void* tag, bool ok) {
    auto handlerTag = reinterpret_cast<Operation>(tag);
    switch (handlerTag->op) {
      case Operation::CONNECT:
        std::cout << "Connect" << std::endl;
        isConnected_ = true;

        stream_->Read(&message_, Operation::READ);
        attemptWrite();
        break;
      case Operation::READ:
        std::cout << "Read message" << std::endl;

        // Create thread to handle request

        stream_->Read(&message_, Operation::READ);
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
      case Operation::CANCELLED:
        std::cout << "Stream finished or lost" << std::endl;
        isConnected_ = false;
        break;
    }
  }

  void removeDestructThread(std::jthread::id id) { 
    auto guard = std::lock_guard<std::mutex>(stateLock_);
    threads_.erase(id); 
  }

  void addToWriteQueue(psychopomp::ClientMessage msg) {
    auto guard = std::lock_guard<std::mutex>(stateLock_);
    msgQueue_.push(std::move(msg));
  }

  void finishWrite(bool ok) {
    auto guard = std::lock_guard<std::mutex>(stateLock_);
    if (ok && !msgQueue_.empty()) {
      msgQueue_.pop();
    }
    isWriting_ = false;
  }

  void attemptWrite() {
    auto guard = std::lock_guard<std::mutex>(stateLock_);
    if (msgQueue_.empty() || isWriting_) {
      return;
    }
    isWriting_ = true;
    stream_->Write(msgQueue_.front(), Operation::WRITE);
  }

  void setConnectedState(bool isConnected) {
    auto guard = std::lock_guard<std::mutex>(stateLock_);
    isConnected_ = isConnected;
  }

  // Should be destructed last
  std::mutex stateLock_;

  std::string serviceName_;
  std::string serviceKey_;

  std::shared_ptr<grpc::Channel> channel_;
  std::unique_ptr<psychopomp::Psychopomp::Stub> stub_;
  grpc::ClientContext context_;
  grpc::CompletionQueue cq_;

  std::unordered_map<std::jthread::id, std::shared_ptr<std::thread>> threads_;

  std::unique_ptr<grpc::ClientAsyncReaderWriter<psychopomp::ClientMessage,
                                                psychopomp::ServerMessage>>
      stream_;
  psychopomp::ServerMessage message_;
  std::queue<ServerMessage> msgQueue_;

  bool isConnected_;
  bool isWriting_;
};
}  // namespace Camfer