add_executable(search-test
    bloom_filter_test.cpp
    lsm_test.cpp
    memtable_test.cpp
    sstable_test.cpp
)

target_link_libraries(search-test PUBLIC search-lsm gtest gtest_main)
target_include_directories(search-test PUBLIC ${CMAKE_SOURCE_DIR})
