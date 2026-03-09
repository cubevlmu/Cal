cmake_minimum_required(VERSION 3.20)

project(ProjectNeon)

if (MSVC)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
endif()

# scripts
include(${CMAKE_SOURCE_DIR}/cmakes/Index.cmake)

# libs
include(${CMAKE_SOURCE_DIR}/libs/index.cmake)

# projects
add_subdirectory(${CMAKE_SOURCE_DIR}/modules/NeoBase)
add_subdirectory(${CMAKE_SOURCE_DIR}/modules/NeoCompiler)
