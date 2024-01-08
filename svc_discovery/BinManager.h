#pragma once

#include <folly/Synchronized.h>

#include <memory>
#include <string>
#include <unordered_map>
#include <unordered_set>

#include "svc_discovery/RequestHandler.h"

namespace psychopomp {
class BinManager {
 public:
  void add(std::shared_ptr<RequestHandler> requestHandler,
           std::string serviceName, std::string binName);
  void remove(std::string serviceName, std::string binName);

  std::unordered_map<
      std::string,
      std::unordered_map<std::string, std::shared_ptr<RequestHandler>>>
  getServices();

 private:
  folly::Synchronized<std::unordered_map<
      std::string,
      std::unordered_map<std::string, std::shared_ptr<RequestHandler>>>>
      serviceConnectionsMap_;
};
}  // namespace psychopomp