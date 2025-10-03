project(NeoCompiler)
###################################################################
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_OBJC_STANDARD 17)
###################################################################
set(INCS 
    "${CMAKE_SOURCE_DIR}/modules/NeoCompiler/src"
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
set(PROJ_NAME NeoCompiler)
###################################################################
SetPlatformDeps(INCS LNKS)

AddSpdlog(INCS LNKS)
AddNeoBase(INCS LNKS DEPS)
###################################################################
file(GLOB_RECURSE src
    "src/*.m"
    "src/*.cpp"
    "src/*.mm"
    "src/*.c"
)
add_executable(${PROJ_NAME}
    ${src}
)
target_include_directories(${PROJ_NAME} PRIVATE
    ${INCS}
)
target_link_libraries(${PROJ_NAME} PRIVATE
    ${LNKS}
)
target_compile_definitions(${PROJ_NAME} PRIVATE
    ${HEAD}
)