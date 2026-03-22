add_library(search-lsm STATIC
    lsm.cpp
    memtable.cpp
    sstable.cpp
)

target_include_directories(search-lsm PUBLIC ${CMAKE_SOURCE_DIR} absl::log)

add_subdirectory(bench)
add_subdirectory(test)
