#include <chrono>
#include <limits>
#include <unordered_map>
#include <utility>
#include <vector>

using namespace std::chrono_literals;
using namespace std::chrono;

namespace psychopomp {

class MockShardBin {
 public:
  explicit MockShardBin(int64_t dropTime, int64_t addTime)
      : dropTime_(dropTime), addTime_(addTime) {}

  void addShard(uint32_t shardId) {
    shardSchedule[shardId] = std::make_pair(
        system_clock::now() + addTime_,
        system_clock::from_time_t(std::numeric_limits<time_t>::max()));
  }

  void dropShard(uint32_t shardId) {
    shardSchedule[shardId].second = system_clock::now() + dropTime_;
  }

  std::vector<uint32_t> getActiveShards() {
    std::vector<uint32_t> activeShards;
    auto now = system_clock::now();

    for (auto it = shardSchedule.begin(); it != shardSchedule.end();) {
      if (it->second.second < now) {
        it = shardSchedule.erase(it);
      } else {
        if (it->second.first >= now) {
          activeShards.push_back(it->first);
        }
        it++;
      }
    }
    return activeShards;
  }

 private:
  std::chrono::seconds dropTime_;
  std::chrono::seconds addTime_;
  std::unordered_map<
      uint32_t, std::pair<time_point<system_clock>, time_point<system_clock>>>
      shardSchedule;
};
}  // namespace psychopomp
