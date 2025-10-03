project(NeoBase)
###################################################################
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_OBJC_STANDARD 17)
###################################################################
set(INCS
    "${CMAKE_SOURCE_DIR}/modules/NeoBase/src"
)
set(LNKS
    ""
)
set(DEPS
    ""
)
set(HEAD
    "NE_DEBUG"
)
set(PROJ_NAME NeoBase)
###################################################################
SetPlatformDeps(INCS LNKS)

AddSpdlog(INCS LNKS)
AddRpmalloc(INCS LNKS)
###################################################################
IF(APPLE)
    file(GLOB_RECURSE src
        "src/*.m"
        "src/*.cpp"
        "src/*.mm"
        "src/*.c"
    )
ELSE()
    file(GLOB_RECURSE src
        "src/*.cpp"
        "src/*.c"
    )
ENDIF()
add_library(${PROJ_NAME} STATIC
    ${src}
)
target_include_directories(${PROJ_NAME} PUBLIC
    ${INCS}
)
target_link_libraries(${PROJ_NAME} PUBLIC
    ${LNKS}
)
target_compile_definitions(${PROJ_NAME} PUBLIC
    ${HEAD}
)