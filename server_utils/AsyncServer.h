#pragma once

#include <folly/Synchronized.h>
#include <folly/executors/CPUThreadPoolExecutor.h>
#include <grpcpp/grpcpp.h>
#include <stddef.h>
#include <stdint.h>

#include <memory>

#include "server_utils/HandlerManager.h"

namespace server_utils {

template <typename Service>
class AsyncServer {
 public:
  AsyncServer(size_t numIoThreads, const std::string& serverAddress,
              std::shared_ptr<HandlerManager<Service>> requestHandlerManager)
      : serverAddress_(serverAddress),
        numIoThreads_(numIoThreads),
        ioThreadPool_(numIoThreads),
        requestHandlerManager_(requestHandlerManager),
        service_() {
    init();
  }

  ~AsyncServer() {
    std::cerr << "Closing" << std::endl;
    server_->Shutdown();
    
    for (auto& completionQueue : completionQueues_) {
      std::cerr << "Shtudown cq" << std::endl;
      completionQueue->Shutdown();
    }
    ioThreadPool_.join();
  }

  void run() {
    server_ = builder_.BuildAndStart();
    if (server_.get() == nullptr) {
      std::cerr << "Cannot bind" << std::endl;
    }
    std::cerr << "Running server" << std::endl;
    for (size_t i = 0; i < numIoThreads_; i++) {
      ioThreadPool_.add([&, i]() {
        void* tag;
        bool ok;

        auto* completionQueue = completionQueues_[i].get();
        requestHandlerManager_->registerService(&service_, completionQueue);
        while (true) {

          bool shutdown = completionQueue->Next(&tag, &ok);

          if (shutdown) {
            std::cerr << "Shutdown in thread" << std::endl;
            break;
          }

          requestHandlerManager_->process(tag, ok);
        }
      });
    }
  }

 private:
  void init() {
    builder_.AddListeningPort(serverAddress_,
                              grpc::InsecureServerCredentials());
    builder_.RegisterService(&service_);

    for (size_t i = 0; i < numIoThreads_; i++) {
      auto cq = builder_.AddCompletionQueue();
      completionQueues_.emplace_back(std::move(cq));
    }
  }

  std::string serverAddress_;
  size_t numIoThreads_;
  folly::CPUThreadPoolExecutor ioThreadPool_;
  std::shared_ptr<HandlerManager<Service>> requestHandlerManager_;
  Service service_;

  grpc::ServerBuilder builder_;

  std::vector<std::unique_ptr<grpc::ServerCompletionQueue>> completionQueues_;
  std::unique_ptr<grpc::Server> server_;
};
}  // namespace server_utils