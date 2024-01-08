#include "psychopomp/SolvingManager.h"

namespace psychopomp {
void SolvingManager::update() {
  auto services = mappingProvider_->getServiceMappings();
  for (auto& [service, bins] : services) {
    for (auto& [binName, shardInfos] : bins) {
        
    }
  }
}


void solve();
void output();
}  // namespace psychopomp