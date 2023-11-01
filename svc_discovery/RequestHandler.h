#pragma once

#include <grpcpp/grpcpp.h>

#include <queue>

#include "ServiceDiscovery.grpc.pb.h"
#include "ServiceDiscovery.pb.h"

namespace psychopomp {

struct RequestHandlerTag {
  void* tag;
  enum class Op { CONNECT = 0, READ = 1, WRITE = 2, FINISH = 3 };
  Op op;
};

class RequestHandler {
 public:
  enum class OpStatus { INACTIVE, WAITING };

  RequestHandler(Psychopomp::AsyncService* service,
                 grpc::ServerCompletionQueue* completionQueue);

  void process(RequestHandlerTag::Op op, bool ok);

  void sendMessage(const ServerMessage& message);

 private:
  void* getOpTag(RequestHandlerTag::Op op) const;

  void attemptSendMessage();

  grpc::ServerContext ctx_;
  std::unique_ptr<grpc::ServerAsyncReaderWriter<ServerMessage, ClientMessage>>
      stream_;

  std::queue<ServerMessage> msgQueue_;
  std::unordered_map<RequestHandlerTag::Op, OpStatus> opStatusMap_;
  std::vector<std::unique_ptr<RequestHandlerTag>> requestHandlerTags_;

  std::queue<ServerMessage> writeQueue_;
};
}  // namespace psychopomp