# Base module

project(CalBase)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

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

InstallJsoncpp(includeList linkList)
InstallXXHash(includeList linkList)
InstallSpdlog(includeList linkList)

file(GLOB_RECURSE src
    "src/*.m"
    "src/*.cpp"
    "src/*.mm"
    "src/*.c"
)
add_library(CalBase STATIC
    ${src}
)

IF(${dependenciesList})
add_dependencies(CalBase 
    ${dependenciesList}
)
ENDIF()

target_include_directories(CalBase PRIVATE 
    ${includeList}
)
target_link_libraries(CalBase PRIVATE 
    ${linkList}
    
    "-framework Foundation"
    "-framework Cocoa"
    "-framework IOKit"
    "-ObjC"
)

target_compile_definitions(CalBase PRIVATE 
    STATIC_PLUGINS
    
    CAL_DEBUG
)