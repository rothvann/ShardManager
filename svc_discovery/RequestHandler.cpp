#include "svc_discovery/RequestHandler.h"

namespace psychopomp {

RequestHandler::RequestHandler(Psychopomp::AsyncService* service,
                               grpc::ServerCompletionQueue* completionQueue,
                               HandlerManager* handlerManager)
    : handlerManager_(handlerManager) {
  requestHandlerTags_.emplace_back(new RequestHandlerTag{
      reinterpret_cast<void*>(this), RequestHandlerTag::Op::CONNECT});
  requestHandlerTags_.emplace_back(new RequestHandlerTag{
      reinterpret_cast<void*>(this), RequestHandlerTag::Op::READ});
  requestHandlerTags_.emplace_back(new RequestHandlerTag{
      reinterpret_cast<void*>(this), RequestHandlerTag::Op::WRITE});
  requestHandlerTags_.emplace_back(new RequestHandlerTag{
      reinterpret_cast<void*>(this), RequestHandlerTag::Op::FINISH});

  stream_.reset(
      new grpc::ServerAsyncReaderWriter<ServerMessage, ClientMessage>(&ctx_));
  service->RequestregisterStream(&ctx_, stream_.get(), completionQueue,
                                 completionQueue,
                                 getOpTag(RequestHandlerTag::Op::CONNECT));
}

void RequestHandler::process(RequestHandlerTag::Op op, bool ok) {
  switch (op) {
    case RequestHandlerTag::Op::CONNECT:
      std::cout << "Client connected" << std::endl;

      // Wait for read
      stream_->Read(&message_, getOpTag(RequestHandlerTag::Op::READ));
      break;
    case RequestHandlerTag::Op::READ: {
      // Process read
      std::cout << "Read message from stream" << std::endl;

      if (!hasAuthenticated) {
        /* authenticate */
      }

      // Check if should wait for next read
      if (!ok) {
        break;
      }
      // Wait for next read
      stream_->Read(&message_, getOpTag(RequestHandlerTag::Op::READ));
      break;
    }
    case RequestHandlerTag::Op::WRITE:
      // Check if write finished or channel is dropped
      if (!ok) {
        break;
      }
      opStatusMap_[RequestHandlerTag::Op::WRITE] = OpStatus::INACTIVE;
      writeQueue_.pop();

      if (status_ == HandlerStatus::STOPPING && writeQueue_.empty()) {
        stream_->Finish(grpc::Status::OK,
                        getOpTag(RequestHandlerTag::Op::FINISH));
        break;
      }

      // Process finished write
      std::cout << "Wrote message to stream" << std::endl;

      // Check next write in queue
      attemptSendMessage();
      break;
    case RequestHandlerTag::Op::FINISH:
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
  stream_->Finish(grpc::Status::OK, getOpTag(RequestHandlerTag::Op::FINISH));
}

void* RequestHandler::getOpTag(RequestHandlerTag::Op op) const {
  auto& ptr = requestHandlerTags_[static_cast<int>(op)];
  return reinterpret_cast<void*>(ptr.get());
}

void RequestHandler::attemptSendMessage() {
  if (status_ == HandlerStatus::STOPPED) {
    return;
  }
  if (opStatusMap_[RequestHandlerTag::Op::WRITE] != OpStatus::INACTIVE) {
    return;
  }
  if (writeQueue_.empty()) {
    return;
  }
  auto message = writeQueue_.front();
  stream_->Write(message, getOpTag(RequestHandlerTag::Op::WRITE));
}

bool RequestHandler::hasStopped() const {
  return status_ == HandlerStatus::STOPPED;
}
}  // namespace psychopomp