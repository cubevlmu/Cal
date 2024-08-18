# Common module

project(CalCommon)
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
add_library(CalCommon STATIC
    ${src}
)

IF(${dependenciesList})
add_dependencies(CalCommon 
    ${dependenciesList}
)
ENDIF()

target_include_directories(CalCommon PRIVATE 
    ${includeList}
)
target_link_libraries(CalCommon PRIVATE 
    ${linkList}
    
    "-framework Foundation"
    "-framework Cocoa"
    "-framework IOKit"
    "-ObjC"
)

target_compile_definitions(CalCommon PRIVATE 
    STATIC_PLUGINS
    
    CAL_DEBUG
)