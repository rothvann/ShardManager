#include "psychopomp/placer/MovementMap.h"

namespace psychopomp {

void MovementMap::addMovement(DomainId shardId, DomainId binId) {
  binToIncomingShardMap_[binId].emplace(shardId);
  shardToNextBinMap_[shardId] = (binId);
}

void MovementMap::removeMovement(DomainId shardId) {
  const auto nextBin = getNextBin(shardId);
  if (nextBin) {
    binToIncomingShardMap_[nextBin.value()].erase(shardId);
    shardToNextBinMap_.erase(shardId);
  }
}

void MovementMap::clearMovements() {
  binToIncomingShardMap_.clear();
  shardToNextBinMap_.clear();
}

const std::unordered_set<DomainId>& MovementMap::getIncomingShards(
    DomainId binId) {
  return binToIncomingShardMap_[binId];
}

folly::Optional<DomainId> MovementMap::getNextBin(DomainId shardId) {
  if (auto parentBin = shardToNextBinMap_.find(shardId);
      parentBin != shardToNextBinMap_.end()) {
    return parentBin->second;
  }
  return folly::none;
}

const std::unordered_map<DomainId, DomainId>& MovementMap::getAllMovements() {
  return shardToNextBinMap_;
}

void MovementMap::addMovements(std::shared_ptr<MovementMap> newMovements) {
  for (auto [shardId, binId] : newMovements->getAllMovements()) {
    addMovement(shardId, binId);
  }
}
}  // namespace psychopomp