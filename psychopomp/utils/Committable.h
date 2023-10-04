#pragma once

#include <experimental/optional>

#include "folly/MapUtil.h"

namespace psychopomp {

template <typename Map, typename Val>
class Committable {
 public:
  Committable() = default;

  template <typename... Keys>
  std::optional<Val> get(const Keys&... keys) const {
    auto* ptr = folly::get_ptr(canaryMap_, keys...);
    if (!ptr) {
      ptr = folly::get_ptr(committedMap_, keys...);
    }
    if (ptr) {
      return *ptr;
    }
    return std::nullopt;
  }

  template <typename... Keys>
  void set(bool isCanary, Val val, const Keys&... keys) {
    auto& map = isCanary ? canaryMap_ : committedMap_;
    setMap(map, val, keys...);
  }

  void commit() {
    copyChanges(canaryMap_, committedMap_);
    canaryMap_.clear();
  }

  void clear() { canaryMap_.clear(); }

  Map& getCanaryMap() { return canaryMap_; }

  Map& getCommittedMap() { return committedMap_; }

 private:
  template <typename NestedMap, typename FirstKey>
  void setMap(NestedMap& map, Val val, const FirstKey& firstKey) {
    map[firstKey] = val;
  }

  template <typename NestedMap, typename FirstKey, typename... Keys>
  void setMap(NestedMap& map, Val val, const FirstKey& firstKey,
              const Keys&... keys) {
    setMap(map[firstKey], val, keys...);
  }

  template <typename NestedMap>
  void copyChanges(NestedMap& canaryMap, NestedMap& committedMap) {
    for (auto& [key, val] : canaryMap) {
      copyChanges(val, committedMap[key]);
    }
  }

  template <template <typename...> typename NestedMap, typename Key>
  void copyChanges(NestedMap<Key, Val>& canaryMap,
                   NestedMap<Key, Val>& committedMap) {
    for (auto& [key, val] : canaryMap) {
      committedMap[key] = val;
    }
  }

  Map committedMap_;
  Map canaryMap_;
};
}  // namespace psychopomp