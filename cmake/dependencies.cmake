include(FetchContent)

set(BUILD_TESTING OFF)

set(BENCHMARK_ENABLE_TESTING OFF)
set(BENCHMARK_ENABLE_GTEST_TESTS OFF)

FetchContent_Declare(
  googletest
  SYSTEM
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG        v1.17.0
)

FetchContent_MakeAvailable(googletest)

FetchContent_Declare(
  benchmark
  SYSTEM
  GIT_REPOSITORY https://github.com/google/benchmark.git
  GIT_TAG        v1.9.4
)

FetchContent_MakeAvailable(benchmark)

set(ROARING_BUILD_LTO OFF)

FetchContent_Declare(
  roaring
  SYSTEM
  GIT_REPOSITORY https://github.com/RoaringBitmap/CRoaring.git
  GIT_TAG        v4.6.1
)

FetchContent_MakeAvailable(roaring)

FetchContent_Declare(
  snowball
  URL      https://snowballstem.org/dist/libstemmer_c-3.0.1.tar.gz
  URL_HASH SHA256=419db89961cf2e30e6417265a4f3c903632d47d6917e7f8c6ae0e4d998743aad
)

FetchContent_MakeAvailable(snowball)
include(cmake/snowball.cmake)
