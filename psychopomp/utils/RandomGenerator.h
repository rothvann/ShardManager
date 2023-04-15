#include <algorithm>
#include <array>
#include <random>
#include <vector>

namespace psychopomp {
namespace random {

template<class T = std::mt19937, std::size_t N = T::state_size * sizeof(typename T::result_type)>
auto SeededRandomGenerator () -> typename std::enable_if<N, T>::type {
    std::random_device source;
    std::array<std::random_device::result_type, N> random_data;
    std::generate(std::begin(random_data), std::end(random_data), std::ref(source));
    std::seed_seq seeds(std::begin(random_data), std::end(random_data));
    return T(seeds);
}

template<typename F>
std::vector<int> generateMetric(F& generator, size_t size) {
  std::vector<int> ret(size);
  for(size_t i = 0; i < size; i++) {
    ret[i].emplace(generator() % 101));
  }
  return ret;
}


}
}
