#pragma once

#include <grpcpp/grpcpp.h>

#include <unordered_map>

namespace server_utils {
template <typename Service>
class HandlerManager {
 public:
  virtual void addHandler(Service* service,
                          grpc::ServerCompletionQueue* completionQueue) = 0;
  virtual void process(void* tag) = 0;
  virtual void removeHandler(void* tag) = 0;
};
}  // namespace server_utils