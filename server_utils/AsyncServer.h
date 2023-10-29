#pragma once

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
  AsyncServer(size_t numIoThreads, std::string serverAddress,
              std::shared_ptr<HandlerManager<Service>> requestHandlerManager);

  void run();

 private:
  folly::CPUThreadPoolExecutor ioThreadPool_;
  std::string serverAddress_;
  std::shared_ptr<HandlerManager<Service>> requestHandlerManager_;
  std::vector<grpc::ServerCompletionQueue> completionQueues_;
};
}  // namespace server_utils