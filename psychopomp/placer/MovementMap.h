#pragma once

#include <memory>
#include <unordered_map>
#include <unordered_set>

#include "psychopomp/Types.h"
#include "folly/Optional.h"

namespace psychopomp {

class MovementMap {
 public:
  void addMovement(DomainId shardId, DomainId binId);

  void removeMovement(DomainId shardId);

  void clearMovements();

  const std::unordered_set<DomainId>& getIncomingShards(DomainId binId);

  folly::Optional<DomainId> getNextBin(DomainId shardId);

  const std::unordered_map<DomainId, DomainId>& getAllMovements();

  void addMovements(std::shared_ptr<MovementMap> newMovements);

 private:
  std::unordered_map<DomainId, std::unordered_set<DomainId>>
      binToIncomingShardMap_;
  std::unordered_map<DomainId, DomainId> shardToNextBinMap_;
};

}  // namespace psychopomp