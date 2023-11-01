#pragma once

#include <folly/Synchronized.h>
#include <grpcpp/grpcpp.h>

#include <memory>
#include <optional>
#include <unordered_map>

#include "ServiceDiscovery.grpc.pb.h"
#include "folly/MapUtil.h"
#include "server_utils/HandlerManager.h"
#include "svc_discovery/RequestHandler.h"
#include "svc_discovery/ServiceConnections.h"

namespace psychopomp {
typedef folly::Synchronized<RequestHandler> SyncedRequestHandler;

class HandlerManager
    : public server_utils::HandlerManager<Psychopomp::AsyncService> {
 public:
  HandlerManager(std::shared_ptr<ServiceConnections> svcConnections);

  void addHandler(Psychopomp::AsyncService* service,
                  grpc::ServerCompletionQueue* completionQueue) override;

  void process(void* handlerTag, bool ok) override;

  std::shared_ptr<SyncedRequestHandler> getSyncedRequestHandler(void* tag);

 private:
  void removeSyncedRequestHandler(void* tag);

  std::shared_ptr<ServiceConnections> svcConnections_;
  folly::Synchronized<
      std::unordered_map<void*, std::shared_ptr<SyncedRequestHandler>>>
      requestHandlerMap_;
};
}  // namespace psychopomp