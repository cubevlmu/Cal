cmake_minimum_required(VERSION 3.15)

project(ProjectCAL)
# set(CMAKE_CXX_STANDARD 17)
# set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(${CMAKE_SOURCE_DIR}/libs/index.cmake)
include(${CMAKE_SOURCE_DIR}/cmakes/Modules.cmake)

# set(includeList 
#     "src"
# )
# set(linkList
#     ""
# )
# set(dependenciesList 
#     ""
# )
# set(headerList 
#     ""
# )

# InstallSpdlog(includeList linkList)
# InstallXXHash(includeList linkList)
# InstallJsoncpp(includeList linkList)
# # InstallLLVM(includeList linkList)

# file(GLOB_RECURSE src
#     "src/*.m"
#     "src/*.cpp"
#     "src/*.mm"
#     "src/*.c"
# )
# add_executable(Test
#     ${src}
# )

# target_include_directories(Test PRIVATE 
#     ${includeList}
# )
# target_link_libraries(Test PRIVATE 
#     ${linkList}

#     "-framework Foundation"
#     "-framework Cocoa"
#     "-framework IOKit"
#     "-ObjC"
# )

# target_compile_definitions(Test PRIVATE 
#     STATIC_PLUGINS
# )

add_subdirectory(modules)