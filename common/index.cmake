# CalCommon Module

function(InstallCommon includeListVar linkListVar dependenciesVar)

    list(APPEND ${dependenciesVar} "CalCommon")
    list(APPEND ${includeListVar} "${CMAKE_SOURCE_DIR}/common/src" ) # "${CMAKE_SOURCE_DIR}/libs/spdlog/include"
    

    IF(WIN32)
        # 处理 Windows 平台
        list(APPEND ${linkListVar} "CalCommon")
    ELSEIF(APPLE)
        # 处理 macOS 平台
        list(APPEND ${linkListVar}  "CalCommon")
    ELSEIF(LINUX)
        # 处理 Linux 平台
        list(APPEND ${linkListVar} "CalCommon")
    ENDIF()

    set(${includeListVar} ${${includeListVar}} PARENT_SCOPE)
    set(${linkListVar} ${${linkListVar}} PARENT_SCOPE)
    set(${dependenciesVar} ${${dependenciesVar}} PARENT_SCOPE)
endfunction()