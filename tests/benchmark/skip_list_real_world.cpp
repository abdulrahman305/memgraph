#include "skip_list_common.hpp"

#include "utils/skip_list.hpp"

DEFINE_int32(max_element, 20000, "Maximum element in the intial list");
DEFINE_int32(max_range, 2000000, "Maximum range used for the test");

int main(int argc, char **argv) {
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  google::InitGoogleLogging(argv[0]);

  utils::SkipList<uint64_t> list;

  {
    auto acc = list.access();
    for (uint64_t i = 0; i <= FLAGS_max_element; ++i) {
      CHECK(acc.insert(i).second);
    }
    uint64_t val = 0;
    for (auto item : acc) {
      CHECK(item == val);
      ++val;
    }
    CHECK(val == FLAGS_max_element + 1);
  }

  RunConcurrentTest([&list](auto *run, auto *stats) {
    std::mt19937 generator(std::random_device{}());
    std::uniform_int_distribution<uint64_t> distribution(0, 9);
    std::mt19937 i_generator(std::random_device{}());
    std::uniform_int_distribution<uint64_t> i_distribution(0, FLAGS_max_range);
    while (run->load(std::memory_order_relaxed)) {
      auto value = distribution(generator);
      auto accessor = list.access();
      auto item = i_distribution(i_generator);
      switch (value) {
        case 0:
          stats->succ[OP_CONTAINS] +=
              static_cast<uint64_t>(accessor.contains(item));
          break;
        case 1:
        case 2:
        case 3:
        case 4:
        case 5:
          stats->succ[OP_FIND] +=
              static_cast<uint64_t>(accessor.find(item) != accessor.end());
          break;
        case 6:
        case 7:
        case 8:
          stats->succ[OP_INSERT] +=
              static_cast<uint64_t>(accessor.insert(item).second);
          break;
        case 9:
          stats->succ[OP_REMOVE] +=
              static_cast<uint64_t>(accessor.remove(item));
          break;
        default:
          std::terminate();
      }
      ++stats->total;
    }
  });

  return 0;
}