#include <iostream>

#include "user_libs/Client.h"

int main() {
  Camfer::Client client("test service", "test key");
  client.start();
  
  std::cout << "Running" << std::endl;
  int a = 10;
  while(a != 0) {
    std::cin >> a;
  }
  return 0;
}