#include "svc_discovery/HandlerManager.h"

#include "folly/MapUtil.h"

namespace psychopomp {
HandlerManager::HandlerManager(
    std::shared_ptr<ServiceConnections> svcConnections)
    : svcConnections_(svcConnections) {}

void HandlerManager::addHandler(Psychopomp::AsyncService* service,
                                grpc::ServerCompletionQueue* completionQueue) {
  auto requestHandler = std::make_shared<SyncedRequestHandler>(
      std::in_place, service, completionQueue);
  auto tag = reinterpret_cast<void*>(requestHandler.get());
  {
    auto handlerMapPtr = requestHandlerMap_.wlock();
    (*handlerMapPtr)[tag] = requestHandler;
  }
}

void HandlerManager::process(void* handlerTag, bool ok) {
  RequestHandlerTag* requestHandlerTag =
      reinterpret_cast<RequestHandlerTag*>(handlerTag);
  std::shared_ptr<SyncedRequestHandler> requestHandler =
      getSyncedRequestHandler(requestHandlerTag->tag);
  if (requestHandler) {
    auto requestHandlerPtr = requestHandler->wlock();
    requestHandlerPtr->process(requestHandlerTag->op, ok);
    if (requestHandlerPtr->hasEnded()) {
      removeSyncedRequestHandler(requestHandlerTag->tag);
    }
  }
}

std::shared_ptr<SyncedRequestHandler> HandlerManager::getSyncedRequestHandler(
    void* tag) {
  const auto handlerMap = requestHandlerMap_.rlock();
  return folly::get_default(*handlerMap, tag, nullptr);
}

void HandlerManager::removeSyncedRequestHandler(void* tag) {
  const auto handlerMap = requestHandlerMap_.wlock();
  handlerMap->erase(tag);
}
}  // namespace psychopomp