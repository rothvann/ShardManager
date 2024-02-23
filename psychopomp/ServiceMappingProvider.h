#pragma once

#include <string>
#include <unordered_map>
#include <vector>

#include "psychopomp/Types.h"

namespace psychopomp {

class ServiceMappingProvider {
 public:
  virtual std::unordered_map<
      ServiceId, std::unordered_map<BinId, std::vector<ShardInfo>>>
  getServiceMappings() = 0;
};
}  // namespace psychopomp