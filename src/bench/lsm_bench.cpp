#include <cstdint>
#include <random>
#include <vector>

#include "benchmark/benchmark.h"
#include "src/bytes.h"
#include "src/local_filesystem.h"
#include "src/lsm.h"
#include "src/memory_filesystem.h"

namespace search {

namespace {

Lsm MakeLsm() {
  auto filesystem = std::make_shared<LocalFileSystem>("/tmp/aaa");

  return Lsm(filesystem, Lsm::Parameters{.memtable_bytes_limit = 1 << 20});
}

void FillLsm(Lsm& lsm, uint64_t count) {
  for (size_t i = 0; i < count; ++i) {
    auto key = ToBytes(i);
    auto value = key;
    lsm.Insert(key, value);
  }
}

void LsmInsert(benchmark::State& state) {
  Lsm lsm = MakeLsm();
  uint64_t i = 0;
  for (auto _ : state) {
    auto key = ToBytes(i++);
    auto value = key;
    lsm.Insert(key, value);
  }

  state.SetItemsProcessed(state.iterations());
}

void LsmPointLookup(benchmark::State& state) {
  const uint64_t count = static_cast<uint64_t>(state.range(0));

  Lsm lsm = MakeLsm();
  FillLsm(lsm, count);

  uint64_t sum = 0;

  uint64_t query = 0;
  for (auto _ : state) {
    std::optional<Value> result = lsm.Get(ToBytes(query++));
    if (query == count) {
      query = 0;
    }
    sum += result.value().size();
  }

  benchmark::DoNotOptimize(sum);

  state.SetItemsProcessed(state.iterations());
}

void LsmReadSmallRange(benchmark::State& state) {
  const size_t count = static_cast<size_t>(state.range(0));
  const size_t window = static_cast<size_t>(state.range(1));

  Lsm lsm = MakeLsm();
  FillLsm(lsm, count);

  size_t iters = 0;
  uint64_t elems = 0;
  for (auto _ : state) {
    const size_t left = iters % (count - window + 1);
    std::vector<std::pair<Key, Value>> result = lsm.ReadRange(ToBytes(left), ToBytes(left + window - 1));
    elems += result.size();
    iters += 1;
  }

  benchmark::DoNotOptimize(elems);
  state.SetItemsProcessed(iters);
}

BENCHMARK(LsmInsert)->MinTime(3);
BENCHMARK(LsmPointLookup)->Arg(1 << 17);
BENCHMARK(LsmReadSmallRange)->Args({1 << 17, 16});

}  // namespace

}  // namespace search
