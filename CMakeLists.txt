cmake_minimum_required(VERSION 3.15)

project(CAL LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(libs/index.cmake)

set(includeList 
    "src"
)
set(linkList
    ""
)
set(dependenciesList 
    ""
)
set(headerList 
    ""
)

InstallSpdlog(includeList linkList)

file(GLOB_RECURSE src
    "src/*.m"
    "src/*.cpp"
    "src/*.mm"
    "src/*.c"
)
add_executable(Test
    ${src}
)

target_include_directories(Test PRIVATE 
    ${includeList}
)
target_link_libraries(Test PRIVATE 
    ${linkList}
)

target_compile_definitions(Test PRIVATE 
    STATIC_PLUGINS
)