#pragma once

#include <functional>
#include <memory>
#include <unordered_map>

#include "psychopomp/placer/SparseMappingTree.h"
#include "psychopomp/placer/SolvingState.h"
#include "psychopomp/placer/utils/Committable.h"

namespace psychopomp {

typedef std::unordered_map<Domain, std::unordered_map<DomainId, int32_t>>
    NodeToMetricsMap;
typedef CommittableMap<NodeToMetricsMap, int32_t> MetricsMap;

/*
Used to create an expression tree. Values are propagated upwards using the
function passed in.
*/
class ExpressionTree {
 public:
  ExpressionTree(
      std::shared_ptr<SolvingState> state, Domain domain,
      const std::vector<DomainId>& treeParents,
      std::function<int32_t(const SparseMappingTree&,
                            const std::vector<std::shared_ptr<MovementMap>>&,
                            const MetricsMap&, const std::vector<DomainId>&,
                            std::pair<Domain, DomainId>)>
          calcMetricFunc);

  std::shared_ptr<SparseMappingTree> getAssignmentTree() const;

  /*
  Canary a set of moves. Changes are outputted to metrics map.
  */
  void canaryMoves(std::shared_ptr<MovementMap> committedMoves,
                   std::shared_ptr<MovementMap> canaryMoves);

  /*
  Commit previous movements previously canaried.
  */
  void commitMoves();

  const MetricsMap& getMetricsMap() const;

 private:
  void initializeAssignmentTree(Domain domain,
                                const std::vector<DomainId>& treeParents);
  void initializeMetricState();

  void propagateChanges(
      const std::unordered_map<
          Domain, std::unordered_map<DomainId, std::vector<DomainId>>>&
          toUpdate,
      const std::vector<std::shared_ptr<MovementMap>>& movementMaps);

  std::shared_ptr<SolvingState> state_;

  std::shared_ptr<SparseMappingTree> assignmentTree_;
  std::function<int32_t(const SparseMappingTree&,
                        const std::vector<std::shared_ptr<MovementMap>>&,
                        const MetricsMap&, const std::vector<DomainId>&,
                        std::pair<Domain, DomainId>)>
      calcMetricFunc_;
  MetricsMap metricsMap_;
};

}  // namespace psychopomp