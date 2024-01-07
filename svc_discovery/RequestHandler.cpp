#include "svc_discovery/RequestHandler.h"

#include "svc_discovery/HandlerManager.h"

namespace psychopomp {

using server_utils::Operation;

RequestHandler::RequestHandler(Psychopomp::AsyncService* service,
                               grpc::ServerCompletionQueue* completionQueue,
                               HandlerManager* handlerManager)
    : handlerManager_(handlerManager) {
  handlerTags_.emplace_back(
      new HandlerTag{reinterpret_cast<void*>(this), Operation::CONNECT});
  handlerTags_.emplace_back(
      new HandlerTag{reinterpret_cast<void*>(this), Operation::READ});
  handlerTags_.emplace_back(
      new HandlerTag{reinterpret_cast<void*>(this), Operation::WRITE});
  handlerTags_.emplace_back(
      new HandlerTag{reinterpret_cast<void*>(this), Operation::FINISH});

  stream_.reset(
      new grpc::ServerAsyncReaderWriter<ServerMessage, ClientMessage>(&ctx_));
  service->RequestRegisterStream(&ctx_, stream_.get(), completionQueue,
                                 completionQueue, getOpTag(Operation::CONNECT));
}

void RequestHandler::handleConnect(bool ok) {
  if (!ok) {
    return;
  }
  std::cout << "Client connected" << std::endl;
  handlerManager_->addHandler();
  // Wait for read
  readFromStream();
};

void RequestHandler::handleRead(bool ok, bool& shouldAttemptNext) {
  // Process read
  std::cout << "Read message from stream" << std::endl;

  // Failed read from client disconnected etc
  // Should close handler after
  if (!ok) {
    std::cout << "Unsuccessful read" << std::endl;
    stop();
    return;
  }

  if (!hasAuthenticated) {
    /* authenticate */
  }

  shouldAttemptNext = true;
};

void RequestHandler::handleWrite(bool ok, bool& shouldAttemptNext) {
  // Check if write finished or channel is dropped
  if (!ok || status_ == HandlerStatus::STOPPED) {
    return;
  }

  shouldAttemptNext = true;
};

void RequestHandler::handleFinish(bool ok) {
  /*
  Logging etc
  */

  std::cout << "Finishing" << std::endl;
  handlerManager_->removeSyncedRequestHandler(this);
};

void RequestHandler::readFromStream() {
  stream_->Read(&message_, getOpTag(Operation::READ));
};

void RequestHandler::writeToStream(const ServerMessage& msg){

};

void* RequestHandler::getOpTag(Operation op) const {
  auto& ptr = handlerTags_[static_cast<int>(op)];
  return reinterpret_cast<void*>(ptr.get());
}

void RequestHandler::stop() {
  std::cout << "Stopping" << std::endl;
  status_ = HandlerStatus::STOPPED;
  stream_->Finish(grpc::Status::OK, getOpTag(Operation::FINISH));
}

bool RequestHandler::hasStopped() const {
  return status_ == HandlerStatus::STOPPED;
}
}  // namespace psychopomp