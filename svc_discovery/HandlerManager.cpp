#include "svc_discovery/HandlerManager.h"

#include "folly/MapUtil.h"

namespace psychopomp {

using server_utils::Operation;

HandlerManager::HandlerManager() {}

void HandlerManager::addHandler() {
  auto requestHandler =
      std::make_shared<RequestHandler>(service_, completionQueue_, this);
  auto tag = requestHandler.get();
  {
    auto handlerMapPtr = requestHandlerMap_.wlock();
    (*handlerMapPtr)[tag] = requestHandler;
  }
}

void HandlerManager::process(void* tag, bool ok) {
  std::cout << "got tag" << std::endl;
  HandlerTag* handlerTag = reinterpret_cast<HandlerTag*>(tag);
  std::shared_ptr<RequestHandler> requestHandler =
      getRequestHandler(handlerTag->tag);
  if (!requestHandler) {
    // log error;
    return;
  }

  requestHandler->process(handlerTag->op, ok);
}

std::shared_ptr<RequestHandler> HandlerManager::getRequestHandler(
    RequestHandler* tag) {
  const auto handlerMap = requestHandlerMap_.rlock();
  return folly::get_default(*handlerMap, tag, nullptr);
}

void HandlerManager::removeRequestHandler(RequestHandler* tag) {
  const auto handlerMap = requestHandlerMap_.wlock();
  handlerMap->erase(tag);
}

void HandlerManager::registerBin(RequestHandler* tag, ServiceId serviceId,
                                 BinId binName) {
  std::shared_ptr<RequestHandler> requestHandler = getRequestHandler(tag);
  if (!requestHandler) {
    // log error;
    return;
  }

  auto connectionsMap = serviceConnectionsMap_.wlock();
  (*connectionsMap)[serviceId].emplace(binName, requestHandler);
}

void HandlerManager::removeBin(ServiceId serviceId, BinId binName) {
  auto connectionsMap = serviceConnectionsMap_.wlock();
  (*connectionsMap)[serviceId].erase(binName);
  if ((*connectionsMap)[serviceId].empty()) {
    (*connectionsMap).erase(serviceId);
  }
}

std::unordered_map<ServiceId, std::unordered_map<BinId, std::vector<ShardInfo>>>
HandlerManager::getServiceMappings() {
  // return serviceConnectionsMap_.copy();
}
}  // namespace psychopomp