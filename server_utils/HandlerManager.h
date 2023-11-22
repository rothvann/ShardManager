#pragma once

#include <grpcpp/grpcpp.h>

#include <unordered_map>

namespace server_utils {
template <typename Service>
class HandlerManager {
 public:
  virtual void registerService(Service* service,
                               grpc::ServerCompletionQueue* completionQueue) {
    service_ = service;
    completionQueue_ = completionQueue;

    addHandler();
  }

  virtual void addHandler() = 0;
  // Return true if should add new handler
  virtual void process(void* tag, bool ok) = 0;

 protected:
  Service* service_;
  grpc::ServerCompletionQueue* completionQueue_;
};
}  // namespace server_utils