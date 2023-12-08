#include "svc_discovery/BinManager.h"

namespace psychopomp {

void BinManager::add(std::shared_ptr<SyncedRequestHandler> requestHandler,
                     std::string serviceName, std::string binName) {
  Bin bin(binName, requestHandler);
  auto connectionsMap = serviceConnectionsMap_.wlock();
  (*connectionsMap)[serviceName].emplace(binName, std::move(bin));
}

void BinManager::remove(std::string serviceName, std::string binName) {
  auto connectionsMap = serviceConnectionsMap_.wlock();
  (*connectionsMap)[serviceName].erase(binName);
  if ((*connectionsMap)[serviceName].empty()) {
    (*connectionsMap).erase(serviceName);
  }
}

std::unordered_map<std::string, std::unordered_map<std::string, Bin>>
BinManager::getServices() {
  return serviceConnectionsMap_.copy();
}
}  // namespace psychopomp
