#include <iostream>

#include "server_utils/AsyncServer.h"
#include "svc_discovery/BinManager.h"
#include "svc_discovery/HandlerManager.h"

int main() {
  auto binManager = std::make_shared<psychopomp::BinManager>();
  auto handlerManager =
      std::make_shared<psychopomp::HandlerManager>(binManager);
  {
    auto server = std::make_shared<
        server_utils::AsyncServer<psychopomp::Psychopomp::AsyncService>>(
        1 /*numThreads*/, "0.0.0.0:50051", handlerManager);
    server->run();

    std::cout << "0: close server, 1: print services and bins" << std::endl;
    while (true) {
      char i;
      std::cin >> i;
      if (i == '0') {
        break;
      } else if (i == '1') {
        auto services = binManager->getServices();
        for (auto [service, binPair] : services) {
          std::cout << "Service: " << service << std::endl;
          for (auto [bin, requestHandler] : binPair) {
            std::cout << bin << ", ";
          }
          std::cout << std::endl;
        }
      }
    }
  }
  return 0;
}