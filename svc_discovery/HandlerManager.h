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
class HandlerManager
    : public server_utils::HandlerManager<Psychopomp::AsyncService> {
 public:
  typedef std::shared_ptr<folly::Synchronized<RequestHandler>>
      SyncedRequestHandler;

  HandlerManager(std::shared_ptr<ServiceConnections> svcConnections);

  void addHandler(Psychopomp::AsyncService* service,
                  grpc::ServerCompletionQueue* completionQueue) override;

  void process(void* tag) override;
  void removeHandler(void* tag) override;

  std::optional<SyncedRequestHandler> getSyncedRequestHandler(void* tag);

 private:
  std::shared_ptr<ServiceConnections> svcConnections_;
  folly::Synchronized<std::unordered_map<void*, SyncedRequestHandler>>
      requestHandlerMap_;
};
}  // namespace psychopomp