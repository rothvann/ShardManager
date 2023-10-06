#pragma once

#include <experimental/optional>

#include "folly/MapUtil.h"

namespace psychopomp {

template <typename Map, typename Val>
class CommittableMap {
 public:
  CommittableMap() = default;

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
    clear();
  }

  void clear() { canaryMap_.clear(); }

  Map& getCanaryMap() { return canaryMap_; }

  Map& getCommittedMap() { return committedMap_; }

  template <typename... Keys>
  std::optional<Val> getFromCanaryMap(const Keys&... keys) const {
    auto ptr = folly::get_ptr(canaryMap_, keys...);
    if (ptr) {
      return *ptr;
    }
    return std::nullopt;
  }

  template <typename... Keys>
  std::optional<Val> getFromCommittedMap(const Keys&... keys) const {
    auto ptr = folly::get_ptr(committedMap_, keys...);
    if (ptr) {
      return *ptr;
    }
    return std::nullopt;
  }

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

  template <typename NestedMap,
            typename std::enable_if<
                !std::is_same<typename NestedMap::mapped_type, Val>::value,
                bool>::type = false>
  void copyChanges(NestedMap& canaryMap, NestedMap& committedMap) {
    for (auto& [key, val] : canaryMap) {
      copyChanges(val, committedMap[key]);
    }
  }

  template <typename NestedMap,
            typename std::enable_if<
                std::is_same<typename NestedMap::mapped_type, Val>::value,
                bool>::type = false>
  void copyChanges(NestedMap& canaryMap, NestedMap& committedMap) {
    for (auto& [key, val] : canaryMap) {
      committedMap[key] = val;
    }
  }

  Map committedMap_;
  Map canaryMap_;
};

template <typename Val>
class CommittableKey {
 public:
  CommittableKey() = default;

  std::optional<Val> get() const {
    if (canaryVal_) {
      return canaryVal_;
    }
    return committedVal_;
  }

  void set(bool isCanary, Val val) {
    if (isCanary) {
      canaryVal_ = val;
    } else {
      committedVal_ = val;
    }
  }

  void commit() {
    if (canaryVal_) {
      committedVal_ = canaryVal_;
    }
    clear();
  }

  void clear() { canaryVal_.reset(); }

  std::optional<Val>& getCanaryVal() { return canaryVal_; }

  std::optional<Val>& getCommittedVal() { return committedVal_; }

 private:
  std::optional<Val> canaryVal_;
  std::optional<Val> committedVal_;
};

}  // namespace psychopomp