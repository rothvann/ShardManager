#include "svc_discovery/HandlerManager.h"

#include "folly/MapUtil.h"

namespace psychopomp {

void HandlerManager::addHandler(Psychopomp::AsyncService* service,
                                grpc::ServerCompletionQueue* completionQueue) {}

void HandlerManager::process(void* tag) {}

void HandlerManager::removeHandler(void* tag) {}

std::optional<HandlerManager::SyncedRequestHandler>
HandlerManager::getSyncedRequestHandler(void* tag) {
  const auto handlerMap = requestHandlerMap_.rlock();
  auto* ptr = folly::get_ptr(*handlerMap, tag);
  if (ptr) {
    return *ptr;
  }
  return std::nullopt;
}

}  // namespace psychopomp