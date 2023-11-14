#include <iostream>

#include "server_utils/AsyncServer.h"
#include "svc_discovery/HandlerManager.h"

int main() {
  auto svcConnections = std::make_shared<psychopomp::ServiceConnections>();
  auto handlerManager =
      std::make_shared<psychopomp::HandlerManager>(svcConnections);
  {
    auto server = std::make_shared<
        server_utils::AsyncServer<psychopomp::Psychopomp::AsyncService>>(
        1 /*numThreads*/, "0.0.0.0:50051", handlerManager);
    server->run();
    std::cout << "Enter input to close server" << std::endl;
    char i;
    std::cin >> i;
  }
  return 0;
}