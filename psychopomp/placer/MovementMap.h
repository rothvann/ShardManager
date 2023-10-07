#pragma once

#include <memory>
#include <optional>
#include <unordered_map>
#include <unordered_set>

#include "psychopomp/Types.h"

namespace psychopomp {

class MovementMap {
 public:
  void addMovement(DomainId shardId, DomainId binId) {
    binToIncomingShardMap_[binId].emplace(shardId);
    shardToNextBinMap_[shardId] = (binId);
  }

  void removeMovement(DomainId shardId) {
    const auto nextBin = getNextBin(shardId);
    if (nextBin) {
      binToIncomingShardMap_[nextBin.value()].erase(shardId);
      shardToNextBinMap_.erase(shardId);
    }
  }

  void clearMovements() {
    binToIncomingShardMap_.clear();
    shardToNextBinMap_.clear();
  }

  const std::unordered_set<DomainId>& getIncomingShards(DomainId binId) {
    return binToIncomingShardMap_[binId];
  }

  std::optional<DomainId> getNextBin(DomainId shardId) {
    if (auto parentBin = shardToNextBinMap_.find(shardId);
        parentBin != shardToNextBinMap_.end()) {
      return parentBin->second;
    }
    return std::nullopt;
  }

  const std::unordered_map<DomainId, DomainId>& getAllMovements() {
    return shardToNextBinMap_;
  }

  void addMovements(std::shared_ptr<MovementMap> newMovements) {
    for (auto [shardId, binId] : newMovements->getAllMovements()) {
      addMovement(shardId, binId);
    }
  }

 private:
  std::unordered_map<DomainId, std::unordered_set<DomainId>>
      binToIncomingShardMap_;
  std::unordered_map<DomainId, DomainId> shardToNextBinMap_;
};

}  // namespace psychopomp