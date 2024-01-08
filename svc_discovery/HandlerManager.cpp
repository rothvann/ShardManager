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

void HandlerManager::registerBin(RequestHandler* tag, std::string serviceName,
                                 std::string binName) {
  std::shared_ptr<RequestHandler> requestHandler = getRequestHandler(tag);
  if (!requestHandler) {
    // log error;
    return;
  }

  auto connectionsMap = serviceConnectionsMap_.wlock();
  (*connectionsMap)[serviceName].emplace(binName, requestHandler);
}

void HandlerManager::removeBin(std::string serviceName, std::string binName) {
  auto connectionsMap = serviceConnectionsMap_.wlock();
  (*connectionsMap)[serviceName].erase(binName);
  if ((*connectionsMap)[serviceName].empty()) {
    (*connectionsMap).erase(serviceName);
  }
}

std::unordered_map<
      ServiceName, std::unordered_map<BinName, std::vector<ShardInfo>>>
HandlerManager::getServiceMappings() {
  
  //return serviceConnectionsMap_.copy();
}
}  // namespace psychopomp