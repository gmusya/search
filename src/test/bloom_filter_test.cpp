#include "src/bloom_filter.h"

#include "gtest/gtest.h"
#include "src/memory_file.h"

namespace search {

Key GenerateRandomKey(std::mt19937& rng, int min_len = 7, int max_len = 11) {
  std::uniform_int_distribution<int> len_dist(min_len, max_len);
  std::uniform_int_distribution<uint8_t> byte_dist(0, 255);

  int length = len_dist(rng);
  Key key;
  key.reserve(length);
  for (int i = 0; i < length; i++) {
    key.push_back(byte_dist(rng));
  }
  return key;
}

TEST(BloomFilter, Simple) {
  struct Configuration {
    uint32_t keys;
    uint32_t bits;
    uint32_t hash_functions;
    uint32_t num_tries;
  };

  constexpr Configuration kConfigurations[] = {
      Configuration{.keys = 10000, .bits = 40000, .hash_functions = 3, .num_tries = 100000},
      Configuration{.keys = 10000, .bits = 40000, .hash_functions = 2, .num_tries = 100000},
      Configuration{.keys = 10000, .bits = 40000, .hash_functions = 1, .num_tries = 100000},
      Configuration{.keys = 10000, .bits = 10000, .hash_functions = 1, .num_tries = 100000},
      Configuration{.keys = 10000, .bits = 10000, .hash_functions = 1, .num_tries = 100000},
      Configuration{.keys = 10000, .bits = 100000, .hash_functions = 6, .num_tries = 1000000}};

  for (const auto& [num_keys, bit_count, hash_count, num_tries] : kConfigurations) {
    std::mt19937 rng(42);

    auto file = std::make_shared<MemoryFile>();

    BloomFilterParameters params{.bytes = bit_count / 8, .functions = hash_count};
    BloomFilterWriter writer(file, params);

    std::vector<Key> added_keys;
    std::set<Key> added_keys_set;

    for (uint32_t i = 0; i < num_keys; i++) {
      Key key = GenerateRandomKey(rng);
      while (added_keys_set.count(key) > 0) {
        key = GenerateRandomKey(rng);
      }
      added_keys_set.insert(key);
      writer.Add(key);
      added_keys.push_back(key);
    }

    std::move(writer).Finish();
    ASSERT_LE(bit_count / 8, file->Size());
    ASSERT_LE(file->Size(), bit_count / 8 + 100);

    BloomFilterReader reader(file);

    for (const auto& key : added_keys) {
      ASSERT_TRUE(reader.Contains(key)) << "False negative for key";
    }

    int false_positives = 0;

    for (uint32_t i = 0; i < num_tries; i++) {
      Key test_key = GenerateRandomKey(rng);
      while (added_keys_set.count(test_key) > 0) {
        test_key = GenerateRandomKey(rng);
      }

      if (reader.Contains(test_key)) {
        false_positives++;
      }
    }

    double actual_fp_rate = static_cast<double>(false_positives) / num_tries;

    double theoretical_fp = std::pow(1 - std::exp(-static_cast<double>(hash_count) * num_keys / bit_count), hash_count);

    EXPECT_LT(actual_fp_rate, theoretical_fp * 1.05) << "FP rate too high: " << actual_fp_rate;
  }
}

}  // namespace search
