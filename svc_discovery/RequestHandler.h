#pragma once

#include <folly/Synchronized.h>
#include <grpcpp/grpcpp.h>

#include <queue>

#include "ServiceDiscovery.grpc.pb.h"
#include "ServiceDiscovery.pb.h"

namespace psychopomp {
class HandlerManager;

struct HandlerTag {
  void* tag;
  enum class Op {
    CONNECT = 0,
    READ = 1,
    WRITE = 2,
    FINISH = 3,
    CANCELLED = 4,
  };
  Op op;
};

class RequestHandler {
 public:
  enum class OpStatus { INACTIVE, WAITING };
  enum class HandlerStatus { ACTIVE, STOPPING, STOPPED };

  RequestHandler(Psychopomp::AsyncService* service,
                 grpc::ServerCompletionQueue* completionQueue,
                 HandlerManager* handlerManager);

  void process(HandlerTag::Op op, bool ok);

  bool sendMessage(const ServerMessage& message);

  void stop();

  bool hasStopped() const;

 private:
  void* getOpTag(HandlerTag::Op op) const;

  void attemptSendMessage();

  HandlerManager* handlerManager_;
  grpc::ServerContext ctx_;
  std::unique_ptr<grpc::ServerAsyncReaderWriter<ServerMessage, ClientMessage>>
      stream_;

  std::queue<ServerMessage> msgQueue_;
  std::unordered_map<HandlerTag::Op, OpStatus> opStatusMap_;
  std::vector<std::unique_ptr<HandlerTag>> handlerTags_;

  bool hasAuthenticated;

  std::queue<ServerMessage> writeQueue_;
  HandlerStatus status_;
  ClientMessage message_;
};

typedef folly::Synchronized<RequestHandler> SyncedRequestHandler;
}  // namespace psychopomp