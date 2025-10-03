# Project neo
# Platform dependencies script

function(SetPlatformDeps incVar lnkVar)
    IF(WIN32)
        # libs of system
        list(APPEND ${lnkVar}
            "user32"
            "shell32"
        )
    ELSEIF(APPLE)
        # libs of system
        list(APPEND ${lnkVar}
            "-framework Foundation"
            "-framework Cocoa"
            "-framework IOKit"
            "-framework QuartzCore"
            "-framework AppKit"
            "-framework SystemConfiguration"
            "-ObjC"
        )
    ELSEIF(LINUX)
        # TODO add linux platform support
    ENDIF()

    set(${incVar} ${${incVar}} PARENT_SCOPE)
    set(${lnkVar} ${${lnkVar}} PARENT_SCOPE)
endfunction()