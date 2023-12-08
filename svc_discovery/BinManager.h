#pragma once

#include <folly/Synchronized.h>

#include <memory>
#include <string>
#include <unordered_map>
#include <unordered_set>

#include "svc_discovery/Bin.h"
#include "svc_discovery/RequestHandler.h"

namespace psychopomp {
class BinManager {
 public:
  void add(std::shared_ptr<SyncedRequestHandler> requestHandler,
           std::string serviceName, std::string binName);
  void remove(std::string serviceName, std::string binName);

  std::unordered_map<std::string, std::unordered_map<std::string, Bin>>
  getServices();

 private:
  folly::Synchronized<
      std::unordered_map<std::string, std::unordered_map<std::string, Bin>>>
      serviceConnectionsMap_;
};
}  // namespace psychopomp