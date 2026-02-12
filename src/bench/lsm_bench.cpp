#include <cstdint>
#include <vector>

#include "benchmark/benchmark.h"
#include "src/bytes.h"
#include "src/lsm.h"
#include "src/memory_filesystem.h"

namespace search {

namespace {

Lsm MakeLsm() {
  auto filesystem = std::make_shared<MemoryFileSystem>();

  return Lsm(filesystem);
}

std::vector<Key> BuildKeys(size_t count) {
  std::vector<Key> keys;
  keys.reserve(count);

  for (size_t i = 0; i < count; ++i) {
    keys.emplace_back(ToBytes(static_cast<uint64_t>(i)));
  }

  return keys;
}

std::vector<Value> BuildValues(size_t count) {
  std::vector<Value> values;
  values.reserve(count);

  for (size_t i = 0; i < count; ++i) {
    values.emplace_back(ToBytes(static_cast<uint64_t>(i + 1000)));
  }

  return values;
}

void FillLsm(Lsm* lsm, const std::vector<Key>& keys, const std::vector<Value>& values) {
  for (size_t i = 0; i < keys.size(); ++i) {
    lsm->Insert(keys[i], values[i]);
  }
}

void LsmInsert(benchmark::State& state) {
  const size_t count = static_cast<size_t>(state.range(0));
  const std::vector<Key> keys = BuildKeys(count);
  const std::vector<Value> values = BuildValues(count);

  for (auto _ : state) {
    Lsm lsm = MakeLsm();
    for (size_t i = 0; i < count; ++i) {
      lsm.Insert(keys[i], values[i]);
    }
  }

  state.SetItemsProcessed(state.iterations() * static_cast<int64_t>(count));
}

void LsmPointLookupOldest(benchmark::State& state) {
  const size_t count = static_cast<size_t>(state.range(0));
  const std::vector<Key> keys = BuildKeys(count);
  const std::vector<Value> values = BuildValues(count);

  Lsm lsm = MakeLsm();
  FillLsm(&lsm, keys, values);
  const Key target = keys.front();

  for (auto _ : state) {
    std::optional<Value> result = lsm.Get(target);
    benchmark::DoNotOptimize(result);
  }

  state.SetItemsProcessed(state.iterations());
}

void LsmPointLookupMiddle(benchmark::State& state) {
  const size_t count = static_cast<size_t>(state.range(0));
  const std::vector<Key> keys = BuildKeys(count);
  const std::vector<Value> values = BuildValues(count);

  Lsm lsm = MakeLsm();
  FillLsm(&lsm, keys, values);
  const Key target = keys[count / 2];

  for (auto _ : state) {
    std::optional<Value> result = lsm.Get(target);
    benchmark::DoNotOptimize(result);
  }

  state.SetItemsProcessed(state.iterations());
}

void LsmPointLookupNewest(benchmark::State& state) {
  const size_t count = static_cast<size_t>(state.range(0));
  const std::vector<Key> keys = BuildKeys(count);
  const std::vector<Value> values = BuildValues(count);

  Lsm lsm = MakeLsm();
  FillLsm(&lsm, keys, values);
  const Key target = keys.back();

  for (auto _ : state) {
    std::optional<Value> result = lsm.Get(target);
    benchmark::DoNotOptimize(result);
  }

  state.SetItemsProcessed(state.iterations());
}

void LsmReadRangeSmall(benchmark::State& state) {
  const size_t count = static_cast<size_t>(state.range(0));
  const size_t window = static_cast<size_t>(state.range(1));
  const std::vector<Key> keys = BuildKeys(count);
  const std::vector<Value> values = BuildValues(count);

  Lsm lsm = MakeLsm();
  FillLsm(&lsm, keys, values);

  size_t offset = 0;
  for (auto _ : state) {
    const size_t left = offset % (count - window + 1);
    std::vector<std::pair<Key, Value>> result = lsm.ReadRange(keys.at(left), keys.at(left + window - 1));
    benchmark::DoNotOptimize(result);
    offset += 9973;
  }

  state.SetItemsProcessed(state.iterations() * static_cast<int64_t>(window));
}

BENCHMARK(LsmInsert)->Arg(1 << 10);
BENCHMARK(LsmPointLookupOldest)->Arg(1 << 10);
BENCHMARK(LsmPointLookupMiddle)->Arg(1 << 10);
BENCHMARK(LsmPointLookupNewest)->Arg(1 << 10);
BENCHMARK(LsmReadRangeSmall)->Args({1 << 10, 16});

}  // namespace

}  // namespace search
