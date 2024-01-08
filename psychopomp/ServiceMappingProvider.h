#pragma once

#include <string>
#include <unordered_map>
#include <vector>

#include "psychopomp/Types.h"

namespace psychopomp {

class ServiceMappingProvider {
 public:
  virtual std::unordered_map<
      ServiceName, std::unordered_map<BinName, std::vector<ShardInfo>>>
  getServiceMappings() = 0;
};
}  // namespace psychopomp