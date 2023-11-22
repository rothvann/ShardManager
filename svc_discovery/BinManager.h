#pragma once

#include <folly/Synchronized.h>

#include "svc_discovery/Bin.h"

#include <memory>
#include <string>
#include <unordered_map>
#include <unordered_set>

namespace psychopomp {
class BinManager {
 public:
  void add(std::shared_ptr<SyncedRequestHandler> requestHandler,
           std::string serviceName, std::string binName);

  std::unordered_map<std::string, Bin> getBins(std::string serviceName);

  std::unordered_map<std::string, Bin> getServices(std::string serviceName);

 private:
  folly::Synchronized<
      std::unordered_map<std::string, std::unordered_map<std::string, Bin>>>
      serviceConnectionsMap_;
};
}  // namespace psychopomp