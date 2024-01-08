#include "psychopomp/SolvingManager.h"

namespace psychopomp {
void SolvingManager::update() {
  auto services = binManager_->getServices();
  for (auto& [service, bins] : services) {
    for (auto [bin, requestHandler] : bins) {
        
    }
  }
}


void solve();
void output();
}  // namespace psychopomp