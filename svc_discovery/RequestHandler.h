#pragma once

#include <folly/Synchronized.h>
#include <grpcpp/grpcpp.h>

#include <queue>
#include <utility>

#include "ServiceDiscovery.grpc.pb.h"
#include "ServiceDiscovery.pb.h"
#include "server_utils/RequestHandler.h"

namespace psychopomp {
class HandlerManager;
struct HandlerTag;

class RequestHandler
    : public server_utils::RequestHandler<ServerMessage, ClientMessage> {
 public:
  enum class OpStatus { INACTIVE, WAITING };
  enum class HandlerStatus { ACTIVE, STOPPED };

  RequestHandler(Psychopomp::AsyncService* service,
                 grpc::ServerCompletionQueue* completionQueue,
                 HandlerManager* handlerManager);

  bool sendMessage(const ServerMessage& message);

  void stop();
  bool hasStopped() const;

 private:
  virtual void handleConnect(bool ok) override;
  virtual void handleRead(bool ok, bool& shouldAttemptNext) override;
  virtual void handleWrite(bool ok, bool& shouldAttemptNext) override;
  virtual void handleFinish(bool ok) override;
  virtual void readFromStream() override;
  virtual void writeToStream(const ServerMessage& msg) override;
  virtual void* getOpTag(server_utils::Operation op) const override;

  bool hasConnected() const;

  // GRPC
  HandlerManager* handlerManager_;
  grpc::ServerContext ctx_;
  std::unique_ptr<grpc::ServerAsyncReaderWriter<ServerMessage, ClientMessage>>
      stream_;
  std::vector<std::unique_ptr<HandlerTag>> handlerTags_;

  // IO
  ClientMessage message_;
  std::queue<ServerMessage> writeQueue_;
  bool isWriting_;

  // Handling Logic
  folly::Synchronized<HandlerStatus> status_;
  folly::Synchronized<bool> hasAuthenticated;

  // State
  folly::Synchronized<std::map<std::pair<int64_t, int64_t>, ShardInfo>>
      shardRangeInfo_;
};
}  // namespace psychopomp