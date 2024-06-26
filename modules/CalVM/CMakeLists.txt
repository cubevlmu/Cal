# CalVM module

project(CalVM)
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

InstallBase(includeList linkList dependenciesList)

file(GLOB_RECURSE src
    "src/*.m"
    "src/*.cpp"
    "src/*.mm"
    "src/*.c"
)
add_library(CalVM SHARED
    ${src}
)

IF(${dependenciesList})
add_dependencies(CalVM 
    ${dependenciesList}
)
ENDIF()

target_include_directories(CalVM PRIVATE 
    ${includeList}
)
target_link_libraries(CalVM PRIVATE 
    ${linkList}
    
    "-framework Foundation"
    "-framework Cocoa"
    "-framework IOKit"
    "-ObjC"
)

target_compile_definitions(CalVM PRIVATE 
    STATIC_PLUGINS
    
    CAL_DEBUG
)