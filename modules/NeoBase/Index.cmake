# Project neo
# Index file of NeoBase module

function(AddNeoBase incVar lnkVar depVar)

    list(APPEND ${depVar} "NeoBase")
    list(APPEND ${incVar}
        "${CMAKE_SOURCE_DIR}/NeoBase/src"
    )

    IF(WIN32)
        list(APPEND ${lnkVar} "NeoBase")
    ELSEIF(APPLE)
        list(APPEND ${lnkVar}  "NeoBase")
    ELSEIF(LINUX)
        list(APPEND ${lnkVar} "NeoBase")
    ENDIF()

    set(${incVar} ${${incVar}} PARENT_SCOPE)
    set(${lnkVar} ${${lnkVar}} PARENT_SCOPE)
    set(${depVar} ${${depVar}} PARENT_SCOPE)
endfunction()