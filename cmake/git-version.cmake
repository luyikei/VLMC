function (append_git_version)
    find_program(GIT git)
    if(GIT)
        execute_process(
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
            COMMAND ${GIT} rev-parse --short HEAD
            OUTPUT_VARIABLE GIT_OUT OUTPUT_STRIP_TRAILING_WHITESPACE
        )
        set(PROJECT_VERSION "${PROJECT_VERSION}-git-${GIT_OUT}" PARENT_SCOPE)
    endif()
endfunction()
