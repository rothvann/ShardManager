#pragma once

#include <folly/executors/CPUThreadPoolExecutor.h>
#include <folly/Synchronized.h>
#include <grpcpp/grpcpp.h>
#include <stddef.h>
#include <stdint.h>

#include <memory>

#include "server_utils/HandlerManager.h"

namespace server_utils {

template <typename Service>
class AsyncServer {
 public:
  AsyncServer(size_t numIoThreads, std::string serverAddress,
              std::shared_ptr<HandlerManager<Service>> requestHandlerManager)
      : numIoThreads_(numIoThreads),
        serverAddress_(serverAddress),
        ioThreadPool_(numIoThreads),
        requestHandlerManager_(requestHandlerManager),
        service_() {}

  ~AsyncServer() {
    server_->Shutdown();
    for (auto& completionQueue : completionQueues_) {
      completionQueue->Shutdown();
    }

    ioThreadPool_.join();
  }

  void run() {
    for (size_t i = 0; i < numIoThreads; i++) {
      ioThreadPool_.add([&]() {
        void* tag;
        bool ok;
        // Use flag to end task
        while (shouldRun_.copy()) {
          auto& completionQueue = completionQueues_[i];
          requestHandlerManager_->addHandler(service_, completionQueue);

          completionQueue->Next(&tag, &ok);
          if (!ok) {
            requestHandlerManager_->removeHandler(tag);
          } else {
            requestHandlerManager_->process(tag);
          }
        }
      });
    }
    server_ = serverBuilder_.BuildAndStart();
  }

 private:
  void init() {
    {
      auto runPtr = shouldRun_.wlock();
      *runPtr = true;
    }
    builder_.AddListeningPort(server_address, InsecureServerCredentials());
    builder.RegisterService(&service_);

    for (size_t i = 0; i < numIoThreads; i++) {
      completionQueues_.emplace_back(builder.AddCompletionQueue());
    }
  }

  std::string serverAddress_;
  size_t numIoThreads;
  Service service_;
  folly::Synchronized<bool> shouldRun_;

  grpc::ServerBuilder builder_;
  folly::CPUThreadPoolExecutor ioThreadPool_;
  std::shared_ptr<HandlerManager<Service>> requestHandlerManager_;
  std::vector<grpc::ServerCompletionQueue*> completionQueues_;
  std::unique_ptr<Server> server_;
};
}  // namespace server_utils