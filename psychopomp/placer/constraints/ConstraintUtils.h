#pragma once

#include "psychopomp/placer/ExpressionTree.h"
#include "psychopomp/placer/State.h"

namespace psychopomp {

bool checkIsFutureChild(
    const std::vector<std::shared_ptr<psychopomp::MovementMap>> &movementMaps,
    DomainId child, Domain domain, DomainId currentBin) {
  for (auto movementMap : movementMaps) {
    auto nextBin = movementMap->getNextBin(child);
    if (nextBin.has_value() && nextBin.value() != currentBin) {
      return false;
    }
  }
  return true;
}

}  // namespace psychopomp