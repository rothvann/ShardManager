#include <iostream>

#include "server_utils/AsyncServer.h"
#include "svc_discovery/HandlerManager.h"
#include "svc_discovery/BinManager.h"

int main() {
  auto binManager = std::make_shared<psychopomp::BinManager>();
  auto handlerManager =
      std::make_shared<psychopomp::HandlerManager>(binManager);
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