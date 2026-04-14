add_library(search-index STATIC
    bitmap.cpp
    index.cpp
    lsm_storage.cpp
    stemmer.cpp
)

target_link_libraries(search-index PUBLIC roaring::roaring stemmer search-lsm)
target_include_directories(search-index PUBLIC ${CMAKE_SOURCE_DIR})

# add_subdirectory(bench)
add_subdirectory(test)
