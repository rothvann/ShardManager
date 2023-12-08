#pragma once

#include <folly/Synchronized.h>

#include "svc_discovery/RequestHandler.h"

namespace psychopomp {
class Bin {
 public:
  Bin(std::string binName, std::shared_ptr<SyncedRequestHandler> requestHandler)
      : binName_(binName), requestHandler_(requestHandler) {}

  std::string getName() {
    return binName_;
  }

  BinState getState() { return state_.copy(); }

  void updateState(BinState binState) {
    auto state = state_.wlock();
    (*state) = binState;
  }

  void sendMessage(ServerMessage msg) {
    auto requestHandler = requestHandler_->wlock();
    requestHandler->sendMessage(msg);
  }

 private:
  std::string binName_;
  std::shared_ptr<SyncedRequestHandler> requestHandler_;
  folly::Synchronized<BinState> state_;
};
}  // namespace psychopomp