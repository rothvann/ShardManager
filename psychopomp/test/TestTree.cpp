#include "gtest/gtest.h"
//#include "psychopomp/placer/IterativeLocalSearch.h"
//#include "psychopomp/placer/utils/Committable.h"

#include <fmt/core.h>

namespace psychopomp {

TEST(TestTree, First) {
  std::vector<std::string> names{
    "bad_any_cast_impl",
    "bad_optional_access",
    "bad_variant_access",
    "base",
    "city",
    "civil_time",
    "cord_internal",
    "cord",
    "cordz_functions",
    "cordz_info",
    "cordz_sample_token",
    "crc_cord_state",
    "crc_cpu_state",
    "crc_cpu_detect",
    "crc_internal",
    "crc32c",
    "debugging_internal",
    "demangle_internal",
    "die_if_null",
    "examine_stack",
    "exponential_biased",
    "failure_signal_handler",
    "flags_commandlineflag_internal",
    "flags_commandlineflag",
    "flags_config",
    "flags_internal",
    "flags_marshalling",
    "flags_parse",
    "flags_private_handle_accessor",
    "flags_program_name",
    "flags_reflection",
    "flags_usage_internal",
    "flags_usage",
    "flags",
    "graphcycles_internal",
    "hash",
    "hashtablez_sampler",
    "int128",
    "kernel_timeout_internal",
    "leak_check",
    "log_entry",
    "log_flags",
    "log_globals",
    "log_initialize",
    "log_internal_check_op",
    "log_internal_conditions",
    "log_internal_fnmatch",
    "log_internal_format",
    "log_internal_globals",
    "log_internal_log_sink_set",
    "log_internal_message",
    "log_internal_nullguard",
    "log_internal_proto",
    "log_severity",
    "log_sink",
    "low_level_hash",
    "malloc_internal",
    "periodic_sampler",
    "random_distributions",
    "random_internal_distribution_test_util",
    "random_internal_platform",
    "random_internal_pool_urbg",
    "random_internal_randen_hwaes_impl",
    "random_internal_randen_hwaes",
    "random_internal_randen_slow",
    "random_internal_randen",
    "random_internal_seed_material",
    "random_seed_gen_exception",
    "random_seed_sequences",
    "raw_hash_set",
    "raw_logging_internal",
    "scoped_set_env",
    "spinlock_wait",
    "stacktrace",
    "status",
    "statusor",
    "str_format_internal",
    "strerror",
    "string_view",
    "strings_internal",
    "strings",
    "symbolize",
    "synchronization",
    "throw_delegate",
    "time_zone",
    "time",
  };

  for(auto name : names) {
    std::cout << fmt::format(R""""(
      prebuilt_cxx_library(
        name = '{}',
        static_lib = '//external_deps:lib-abseil-{}',
        header_namespace = '',
        exported_headers = ['//external_deps:headers-abseil'],
        preferred_linkage = 'static',
        visibility = [
          'PUBLIC',
        ],
      )
    )"""", name, name) << std::endl;
  }
  /*
  std::shared_ptr<State> state = std::make_shared<State>(1, 2);
  size_t numShards = 600000;
  size_t numBins = 5000;
  std::vector<DomainId> shards;
  std::vector<std::vector<DomainId>> binShardMap(numBins + 1,
                                                 std::vector<DomainId>());
  for (size_t i = 0; i < numShards; i++) {
    shards.emplace_back(i);
  }

  binShardMap[0] = shards;

  state->setShards(numShards);
  state->addDomain(2, 1, binShardMap);

  std::vector<int32_t> metric;
  for (size_t i = 0; i < numShards; i++) {
    metric.emplace_back(5);
  }

  state->addMetric(0, metric);
  std::vector<std::shared_ptr<Constraint>> constraints;
  std::shared_ptr<MetricConstraint> freeConstraint =
      std::make_shared<MetricConstraint>(state, 2, std::vector<DomainId>{0}, 0,
                                         0, 1);

  std::vector<DomainId> capacity;
  for (size_t i = 1; i <= numBins; i++) {
    capacity.emplace_back(i);
  }
  std::shared_ptr<MetricConstraint> capacityConstraint =
      std::make_shared<MetricConstraint>(state, 2, capacity, 0, 1000, 1);

  IterativeLocalSearch search(500000);
  auto movementMap = search.solve(state, {freeConstraint, capacityConstraint});
  */
}
}  // namespace psychopomp

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}