#pragma once

#include <functional>
#include <memory>
#include <unordered_map>

#include "psychopomp/AssignmentTree.h"
#include "psychopomp/MetricsMap.h"
#include "psychopomp/State.h"

namespace psychopomp {
class ExpressionTree {
 public:
  ExpressionTree(
      std::shared_ptr<State> state, Metric metric, Domain domain,
      const std::vector<DomainId>& treeParents,
      std::function<int32_t(const AssignmentTree&,
                            const std::vector<std::shared_ptr<MovementMap>>&,
                            const MetricsMap&, const std::vector<DomainId>&,
                            std::pair<Domain, DomainId>)>
          calcMetricFunc);

  std::shared_ptr<AssignmentTree> getAssignmentTree() const;

  void canaryMoves(std::shared_ptr<MovementMap> committedMoves,
                   std::shared_ptr<MovementMap> canaryMoves);

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
      const std::vector<std::shared_ptr<MovementMap>>& movementMaps,
      bool isCanary);

  std::shared_ptr<State> state_;
  Metric metric_;

  std::shared_ptr<AssignmentTree> assignmentTree_;
  std::function<int32_t(const AssignmentTree&,
                        const std::vector<std::shared_ptr<MovementMap>>&,
                        const MetricsMap&, const std::vector<DomainId>&,
                        std::pair<Domain, DomainId>)>
      calcMetricFunc_;
  MetricsMap metricsMap_;
};

}  // namespace psychopomp