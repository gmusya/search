add_executable(search-bench
    lsm_bench.cpp
)

target_link_libraries(search-bench PUBLIC search-lsm benchmark::benchmark benchmark::benchmark_main)
target_include_directories(search-bench PUBLIC ${CMAKE_SOURCE_DIR})
