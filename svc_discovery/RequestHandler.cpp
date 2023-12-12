#include "svc_discovery/RequestHandler.h"

namespace psychopomp {

RequestHandler::RequestHandler(Psychopomp::AsyncService* service,
                               grpc::ServerCompletionQueue* completionQueue,
                               HandlerManager* handlerManager)
    : handlerManager_(handlerManager) {
  handlerTags_.emplace_back(
      new HandlerTag{reinterpret_cast<void*>(this), HandlerTag::Op::CONNECT});
  handlerTags_.emplace_back(
      new HandlerTag{reinterpret_cast<void*>(this), HandlerTag::Op::READ});
  handlerTags_.emplace_back(
      new HandlerTag{reinterpret_cast<void*>(this), HandlerTag::Op::WRITE});
  handlerTags_.emplace_back(
      new HandlerTag{reinterpret_cast<void*>(this), HandlerTag::Op::FINISH});

  stream_.reset(
      new grpc::ServerAsyncReaderWriter<ServerMessage, ClientMessage>(&ctx_));
  service->RequestRegisterStream(&ctx_, stream_.get(), completionQueue,
                                 completionQueue,
                                 getOpTag(HandlerTag::Op::CONNECT));
}

void RequestHandler::process(HandlerTag::Op op, bool ok) {
  switch (op) {
    case HandlerTag::Op::CONNECT:
      std::cout << "Client connected" << std::endl;

      // Wait for read
      stream_->Read(&message_, getOpTag(HandlerTag::Op::READ));
      break;
    case HandlerTag::Op::READ: {
      // Process read
      std::cout << "Read message from stream" << std::endl;

      if (!hasAuthenticated) {
        /* authenticate */
      }

      // Check if should wait for next read
      if (!ok) {
        std::cout << "Unsuccessful read" << std::endl;
        break;
      }
      // Wait for next read
      stream_->Read(&message_, getOpTag(HandlerTag::Op::READ));
      break;
    }
    case HandlerTag::Op::WRITE:
      // Check if write finished or channel is dropped
      if (!ok) {
        break;
      }
      opStatusMap_[HandlerTag::Op::WRITE] = OpStatus::INACTIVE;
      writeQueue_.pop();

      if (status_ == HandlerStatus::STOPPING && writeQueue_.empty()) {
        stream_->Finish(grpc::Status::OK, getOpTag(HandlerTag::Op::FINISH));
        break;
      }

      // Process finished write
      std::cout << "Wrote message to stream" << std::endl;

      // Check next write in queue
      attemptSendMessage();
      break;
    case HandlerTag::Op::FINISH:
      /*
      Logging etc
      */
      status_ = HandlerStatus::STOPPED;
      break;
  }
}

bool RequestHandler::sendMessage(const ServerMessage& message) {
  if (status_ == HandlerStatus::STOPPING || status_ == HandlerStatus::STOPPED) {
    return false;
  }
  writeQueue_.emplace(message);
  attemptSendMessage();
  return true;
}

void RequestHandler::stop() {
  status_ = HandlerStatus::STOPPING;
  if (!writeQueue_.empty()) {
    return;
  }
  stream_->Finish(grpc::Status::OK, getOpTag(HandlerTag::Op::FINISH));
}

void* RequestHandler::getOpTag(HandlerTag::Op op) const {
  auto& ptr = handlerTags_[static_cast<int>(op)];
  return reinterpret_cast<void*>(ptr.get());
}

void RequestHandler::attemptSendMessage() {
  if (status_ == HandlerStatus::STOPPED) {
    return;
  }
  if (opStatusMap_[HandlerTag::Op::WRITE] != OpStatus::INACTIVE) {
    return;
  }
  if (writeQueue_.empty()) {
    return;
  }
  auto message = writeQueue_.front();
  stream_->Write(message, getOpTag(HandlerTag::Op::WRITE));
}

bool RequestHandler::hasStopped() const {
  return status_ == HandlerStatus::STOPPED;
}
}  // namespace psychopomp