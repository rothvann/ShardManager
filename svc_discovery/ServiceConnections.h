#pragma once

#include <folly/Synchronized.h>

#include <memory>
#include <string>
#include <unordered_map>
#include <unordered_set>

namespace psychopomp {
class ServiceConnections {
 public:
  typedef std::unordered_set<void*> Connections;
  typedef std::shared_ptr<folly::Synchronized<Connections>> SyncedConnections;

  void add(void* tag, std::string serviceName);
  void remove(void* tag);

  SyncedConnections getConnections(std::string serviceName);

 private:
  folly::Synchronized<std::unordered_map<std::string, SyncedConnections>>
      serviceConnectionsMap_;
};
}  // namespace psychopomp