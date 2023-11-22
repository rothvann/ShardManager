#pragma once

#include <grpcpp/grpcpp.h>

#include <queue>
#include <folly/Synchronized.h>

#include "ServiceDiscovery.grpc.pb.h"
#include "ServiceDiscovery.pb.h"

namespace psychopomp {
class HandlerManager;

struct RequestHandlerTag {
  void* tag;
  enum class Op { CONNECT = 0, READ = 1, WRITE = 2, FINISH = 3 };
  Op op;
};

class RequestHandler {
 public:
  enum class OpStatus { INACTIVE, WAITING };
  enum class HandlerStatus { ACTIVE, STOPPING, STOPPED };

  RequestHandler(Psychopomp::AsyncService* service,
                 grpc::ServerCompletionQueue* completionQueue,
                 HandlerManager* handlerManager);

  void process(RequestHandlerTag::Op op, bool ok);

  bool sendMessage(const ServerMessage& message);

  void stop();

  bool hasStopped() const;

 private:
  void* getOpTag(RequestHandlerTag::Op op) const;

  void attemptSendMessage();

  HandlerManager* handlerManager_;
  grpc::ServerContext ctx_;
  std::unique_ptr<grpc::ServerAsyncReaderWriter<ServerMessage, ClientMessage>>
      stream_;

  std::queue<ServerMessage> msgQueue_;
  std::unordered_map<RequestHandlerTag::Op, OpStatus> opStatusMap_;
  std::vector<std::unique_ptr<RequestHandlerTag>> requestHandlerTags_;

  bool hasAuthenticated;

  std::queue<ServerMessage> writeQueue_;
  HandlerStatus status_;
  ClientMessage message_;
};

typedef folly::Synchronized<RequestHandler> SyncedRequestHandler;
}  // namespace psychopomp