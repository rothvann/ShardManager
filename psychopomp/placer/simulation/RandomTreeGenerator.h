#pragma once

#include <vector>

#include "psychopomp/Types.h"
#include "psychopomp/placer/utils/RandomGenerator.h"

namespace psychopomp {

std::vector<std::vector<DomainId>> mapBinsToDomainsRandom(
    std::vector<size_t> domainSizes, size_t numBins) {
  std::vector<std::vector<DomainId>> randomBinMapping(numBins + 1);
  auto randomGen = random::SeededRandomGenerator();

  std::vector<std::uniform_int_distribution<int>> uniformDists;
  for (auto size : domainSizes) {
    // Map from [1, size - 1] since domain 0 is excluded
    uniformDists.emplace_back(1, size - 1);
  }

  // Map null bin 0 to null domainIds 0
  randomBinMapping[0] = std::vector(0, domainSizes.size());

  for (DomainId binId = 1; binId <= numBins; binId++) {
    std::vector<DomainId> mapping;
    mapping.reserve(domainSizes.size());
    for (auto& dist : uniformDists) {
      mapping.emplace_back(dist(randomGen));
    }
    randomBinMapping[binId] = std::move(mapping);
  }

  return randomBinMapping;
}

std::vector<std::vector<DomainId>> mapShardsToBinsRandom(size_t numBins,
                                                         size_t numShards,
                                                         bool mapEmptyBin) {
  size_t startingBin = mapEmptyBin ? 0 : 1;
  std::vector<std::vector<DomainId>> binShardMapping(numBins + 1);
  auto randomGen = random::SeededRandomGenerator();
  std::uniform_int_distribution<int> dist(startingBin, numBins);
  for (DomainId shardId = 0; shardId < numShards; shardId++) {
    auto binId = dist(randomGen);
    binShardMapping[binId].emplace_back(shardId);
  }
  return binShardMapping;
}

std::vector<std::vector<DomainId>> mapShardsToBinsEmpty(size_t numBins,
                                                        size_t numShards) {
  std::vector<std::vector<DomainId>> binShardMapping(numBins + 1);
  for (DomainId shardId = 0; shardId < numShards; shardId++) {
    binShardMapping[0].emplace_back(shardId);
  }
  return binShardMapping;
}
}  // namespace psychopomp
