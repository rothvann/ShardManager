#pragma once

#include <unordered_map>

#include "psychopomp/State.h"

namespace psychopomp {

typedef std::unordered_map<Domain, std::unordered_map<DomainId, int32_t>>
    NodeToMetricsMap;
class MetricsMap {
 public:
  void setMetric(Domain domain, DomainId domainId, int32_t metric,
                 bool isCanary) {
    if (isCanary) {
      domainMetricsChangesMap_[domain][domainId] = metric;
    } else {
      domainMetricsMap_[domain][domainId] = metric;
    }
  }

  std::optional<int32_t> getMetric(Domain domain, DomainId domainId) const {
    if (auto metricMap = domainMetricsChangesMap_.find(domain);
        metricMap != domainMetricsChangesMap_.end()) {
      if (auto it = metricMap->second.find(domainId);
          it != metricMap->second.end()) {
        return it->second;
      }
    }

    if (auto metricMap = domainMetricsMap_.find(domain);
        metricMap != domainMetricsMap_.end()) {
      if (auto it = metricMap->second.find(domainId);
          it != metricMap->second.end()) {
        return it->second;
      }
    }

    return std::nullopt;
  }

  const NodeToMetricsMap& getChanges() const {
    return domainMetricsChangesMap_;
  }

  void clearChanges() { domainMetricsChangesMap_.clear(); }

  void commitMoves() {
    for (auto& [domain, metricsMap] : domainMetricsChangesMap_) {
      for (auto [domainId, metric] : metricsMap) {
        domainMetricsMap_[domain][domainId] = metric;
      }
    }
    domainMetricsChangesMap_.clear();
  }

 private:
  NodeToMetricsMap domainMetricsMap_;
  NodeToMetricsMap domainMetricsChangesMap_;
};
}  // namespace psychopomp