#include "svc_discovery/HandlerManager.h"

#include "folly/MapUtil.h"

namespace psychopomp {

HandlerManager::HandlerManager(std::shared_ptr<BinManager> binManager)
    : binManager_(binManager) {}

void HandlerManager::addHandler() {
  auto requestHandler = std::make_shared<SyncedRequestHandler>(
      std::in_place, service_, completionQueue_, this);
  auto tag = reinterpret_cast<void*>(requestHandler.get());
  {
    auto handlerMapPtr = requestHandlerMap_.wlock();
    (*handlerMapPtr)[tag] = requestHandler;
  }
}

void HandlerManager::process(void* tag, bool ok) {
  HandlerTag* handlerTag =
      reinterpret_cast<HandlerTag*>(tag);
  std::shared_ptr<SyncedRequestHandler> requestHandler =
      getSyncedRequestHandler(handlerTag->tag);
  if (!requestHandler) {
    // log error;
    return;
  }

  auto requestHandlerPtr = requestHandler->wlock();
  requestHandlerPtr->process(handlerTag->op, ok);
  if (requestHandlerPtr->hasStopped()) {
    removeSyncedRequestHandler(handlerTag->tag);
  }
  if(handlerTag->op == HandlerTag::Op::CONNECT) {
    addHandler();
  }
}

bool HandlerManager::registerBin(std::string serviceName, std::string binName) {
  
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