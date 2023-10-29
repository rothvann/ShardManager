#pragma once

#include <grpcpp/grpcpp.h>

#include "ServiceDiscovery.grpc.pb.h"

namespace psychopomp {
class RequestHandler {
 public:
  RequestHandler(Psychopomp::AsyncService* service,
                 grpc::ServerCompletionQueue* completionQueue);

  void sendMessage(const ServerMessage& message);

 private:
  grpc::ServerAsyncResponseWriter<ServerMessage> responder_;
  grpc::ServerContext ctx_;
};
}  // namespace psychopomp