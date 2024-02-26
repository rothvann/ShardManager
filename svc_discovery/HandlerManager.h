#pragma once

#include <folly/Synchronized.h>
#include <grpcpp/grpcpp.h>

#include <memory>
#include <optional>
#include <unordered_map>

#include "ServiceDiscovery.grpc.pb.h"
#include "folly/MapUtil.h"
#include "psychopomp/ServiceMappingProvider.h"
#include "server_utils/HandlerManager.h"
#include "svc_discovery/RequestHandler.h"
namespace psychopomp {

struct HandlerTag {
  RequestHandler* tag;
  server_utils::Operation op;
};

class HandlerManager
    : public server_utils::HandlerManager<Psychopomp::AsyncService>,
      public ServiceMappingProvider {
 public:
  HandlerManager();

  void addHandler() override;

  void process(void* tag, bool ok) override;

  std::shared_ptr<RequestHandler> getRequestHandler(RequestHandler* tag);
  void removeRequestHandler(RequestHandler* tag);

  virtual std::unordered_map<ServiceId, std::unordered_map<BinId, BinInfo>>
  getServiceMappings() override;

  void registerBin(RequestHandler* tag, ServiceId serviceId, BinId binId);
  void removeBin(ServiceId serviceId, BinId binId);

 private:
  folly::Synchronized<
      std::unordered_map<RequestHandler*, std::shared_ptr<RequestHandler>>>
      requestHandlerMap_;

  folly::Synchronized<std::unordered_map<
      ServiceId, std::unordered_map<BinId, std::shared_ptr<RequestHandler>>>>
      serviceConnectionsMap_;
};
}  // namespace psychopomp