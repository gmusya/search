# Build libstemmer from the pre-generated libstemmer_c distribution.
# This file is included after FetchContent_MakeAvailable(snowball).

file(GLOB STEMMER_ALGO_SOURCES "${snowball_SOURCE_DIR}/src_c/*.c")

add_library(stemmer STATIC
    ${snowball_SOURCE_DIR}/runtime/api.c
    ${snowball_SOURCE_DIR}/runtime/utilities.c
    ${snowball_SOURCE_DIR}/libstemmer/libstemmer.c
    ${STEMMER_ALGO_SOURCES}
)

target_include_directories(stemmer SYSTEM PUBLIC
    ${snowball_SOURCE_DIR}/include
)

target_include_directories(stemmer PRIVATE
    ${snowball_SOURCE_DIR}
)
