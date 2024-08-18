cmake_minimum_required(VERSION 3.20)

project(CalProject)

include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules.cmake)
include(${CMAKE_CURRENT_SOURCE_DIR}/libs/index.cmake)

add_subdirectory(common)
add_subdirectory(compilier)