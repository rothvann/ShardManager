#include "svc_discovery/BinManager.h"

namespace psychopomp {

void BinManager::add(std::shared_ptr<SyncedRequestHandler> requestHandler,
                     std::string serviceName, std::string binName) {
  auto bin = std::make_shared<Bin>(binName, requestHandler);
  auto connectionsMap = serviceConnectionsMap_.wlock();
  (*connectionsMap)[serviceName].emplace(binName, bin);
}

void BinManager::remove(std::string serviceName, std::string binName) {
  auto connectionsMap = serviceConnectionsMap_.wlock();
  (*connectionsMap)[serviceName].erase(binName);
  if ((*connectionsMap)[serviceName].empty()) {
    (*connectionsMap).erase(serviceName);
  }
}

std::unordered_map<std::string, std::unordered_map<std::string, std::shared_ptr<Bin>>>
BinManager::getServices() {
  return serviceConnectionsMap_.copy();
}
}  // namespace psychopomp
