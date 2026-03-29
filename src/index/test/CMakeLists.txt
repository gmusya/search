add_executable(search-index-test
    bitmap_test.cpp
    value_index_test.cpp
    index_test.cpp
    lsm_storage_test.cpp
    stemmer_test.cpp
)

target_link_libraries(search-index-test PUBLIC search-index gtest gtest_main)
target_include_directories(search-index-test PUBLIC ${CMAKE_SOURCE_DIR})
