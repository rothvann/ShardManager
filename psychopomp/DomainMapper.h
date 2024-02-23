#pragma once

#include "psychopomp/Types.h"
#include "folly/MapUtil.h"
#include "ServiceDiscovery.pb.h"

namespace psychopomp {

class DomainMapper {
 public:
  DomainMapper(
      const std::unordered_map<BinId, std::vector<ShardInfo>>& binMapping,
      std::shared_ptr<std::vector<std::pair<ShardKey, ShardKey>>>
          shardKeyRangeMapping) {
    size_t binCount = 0;
    for (auto& [binId, shardInfos] : binMapping) {
      binCount++;
      binDomainIdMapping_[binCount] = binId;
    }
  }

  folly::Optional<DomainId> getDomainId(BinId binName) {
    return folly::get_optional(domainIdBinMapping_, binName);
  }

  folly::Optional<BinId> getBinId(DomainId domainId) {
    return folly::get_optional(binDomainIdMapping_, domainId);
  }

  folly::Optional<std::string> getGroup(std::vector<DomainId> domainIds) {}

  folly::Optional<std::pair<Domain, DomainId>> getDomainId(std::vector<std::string> groups) {}

 private:
  std::unordered_map<DomainId, BinId> binDomainIdMapping_;
  std::unordered_map<BinId, DomainId> domainIdBinMapping_;
};
}  // namespace psychopomp