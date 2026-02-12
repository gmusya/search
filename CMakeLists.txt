cmake_minimum_required(VERSION 3.25)

project(search)

include(cmake/compile.cmake)
include(cmake/dependencies.cmake)
include(cmake/sanitize.cmake)

add_subdirectory(src)
