# Compilier module

project(CalCompilier)
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
InstallLLVM(includeList linkList)
InstallCommon(includeList linkList dependenciesList)

file(GLOB_RECURSE src
    "src/*.m"
    "src/*.cpp"
    "src/*.mm"
    "src/*.c"
)
add_executable(CalCompilier
    ${src}
)

IF(${dependenciesList})
add_dependencies(CalCompilier 
    ${dependenciesList}
)
ENDIF()

target_include_directories(CalCompilier PRIVATE 
    ${includeList}
)
target_link_libraries(CalCompilier PRIVATE 
    ${linkList}
    
    "-framework Foundation"
    "-framework Cocoa"
    "-framework IOKit"
    "-ObjC"
)

target_compile_definitions(CalCompilier PRIVATE 
    STATIC_PLUGINS
    
    CAL_DEBUG
)