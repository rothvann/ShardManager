#pragma once

#include <folly/Synchronized.h>
#include <grpcpp/grpcpp.h>

#include <memory>
#include <optional>
#include <unordered_map>

#include "ServiceDiscovery.grpc.pb.h"
#include "folly/MapUtil.h"
#include "server_utils/HandlerManager.h"
#include "svc_discovery/BinManager.h"
#include "svc_discovery/RequestHandler.h"

namespace psychopomp {
class HandlerManager
    : public server_utils::HandlerManager<Psychopomp::AsyncService> {
 public:

  HandlerManager(std::shared_ptr<BinManager> binManager);

  void addHandler() override;

  void process(void* tag, bool ok) override;

  bool registerBin(std::string serviceName, std::string binName);

  std::shared_ptr<SyncedRequestHandler> getSyncedRequestHandler(void* tag);

 private:
  void removeSyncedRequestHandler(void* tag);

  std::shared_ptr<BinManager> binManager_;
  folly::Synchronized<
      std::unordered_map<void*, std::shared_ptr<SyncedRequestHandler>>>
      requestHandlerMap_;
};
}  // namespace psychopomp