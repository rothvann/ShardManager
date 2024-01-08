#include "svc_discovery/RequestHandler.h"

#include "svc_discovery/HandlerManager.h"

namespace psychopomp {

using server_utils::Operation;

RequestHandler::RequestHandler(Psychopomp::AsyncService* service,
                               grpc::ServerCompletionQueue* completionQueue,
                               HandlerManager* handlerManager)
    : handlerManager_(handlerManager) {
  handlerTags_.emplace_back(new HandlerTag{this, Operation::CONNECT});
  handlerTags_.emplace_back(new HandlerTag{this, Operation::READ});
  handlerTags_.emplace_back(new HandlerTag{this, Operation::WRITE});
  handlerTags_.emplace_back(new HandlerTag{this, Operation::FINISH});

  stream_.reset(
      new grpc::ServerAsyncReaderWriter<ServerMessage, ClientMessage>(&ctx_));
  service->RequestRegisterStream(&ctx_, stream_.get(), completionQueue,
                                 completionQueue, getOpTag(Operation::CONNECT));
}

void RequestHandler::handleConnect(bool ok) {
  std::cout << "Handle connect" << std::endl;
  if (!ok) {
    std::cout << "Connect not ok" << std::endl;
    stop();
    return;
  }
  std::cout << "Client connected" << std::endl;
  handlerManager_->addHandler();
  // Wait for read
  readFromStream();
};

void RequestHandler::handleRead(bool ok, bool& shouldAttemptNext) {
  // Process read
  std::cout << "Handle read" << std::endl;

  // Failed read from client disconnected etc
  // Should close handler after
  if (!ok) {
    std::cout << "Unsuccessful read" << std::endl;
    stop();
    return;
  }
  
  auto& message = getMessage();

  if (!hasAuthenticated.copy()) {
    if (!message.has_connection_request()) {
      std::cout << "Attempting to connect without connection request"
                << std::endl;
      stop();
    }
    // Authenticate
    auto& binName = message.connection_request().bin_name();
    auto& serviceName = message.connection_request().service_name();
    auto& serviceKey = message.connection_request().service_key();
    std::cout << "Bin: " << binName << " Service: " << serviceName
              << " Key: " << serviceKey << std::endl;

    handlerManager_->registerBin(this, serviceName, binName);
  }

  shouldAttemptNext = true;
};

void RequestHandler::handleWrite(bool ok, bool& shouldAttemptNext) {
  std::cout << "Handle write" << std::endl;
  // Check if write finished or channel is dropped
  if (!ok || hasStopped()) {
    return;
  }

  shouldAttemptNext = true;
};

void RequestHandler::handleFinish(bool ok) {
  std::cout << "Handle finish" << std::endl;
  /*
  Logging etc
  */

  std::cout << "Finishing" << std::endl;
  handlerManager_->removeRequestHandler(this);
};

void RequestHandler::readFromStream() {
  stream_->Read(&getMessage(), getOpTag(Operation::READ));
};

void RequestHandler::writeToStream(const ServerMessage& msg) {
  stream_->Write(msg, getOpTag(Operation::WRITE));
};

void* RequestHandler::getOpTag(Operation op) const {
  auto& ptr = handlerTags_[static_cast<int>(op)];
  return reinterpret_cast<void*>(ptr.get());
}

void RequestHandler::stop() {
  if (hasStopped()) {
    return;
  }
  *status_.wlock() = HandlerStatus::STOPPED;
  if (hasAuthenticated.copy()) {
    stream_->Finish(grpc::Status::OK, getOpTag(Operation::FINISH));
  }
}

bool RequestHandler::hasStopped() const {
  return status_.copy() == HandlerStatus::STOPPED;
}

}  // namespace psychopomp