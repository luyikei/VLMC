
# Output file name
SET(NSIS_OUTPUT_FILE "${PROJECT_NAME}.exe")

# NSIS is only available for Win32 builds (native or cross compilation)
IF(WIN32)
    
    # Check contribs existence
    IF(EXISTS "${CMAKE_SOURCE_DIR}/contribs/")

        MESSAGE(STATUS "Contribs found!")

        # Copy necessary libraries required at runtime
        FILE(COPY "${CMAKE_SOURCE_DIR}/contribs/bin/"
             DESTINATION "${CMAKE_BINARY_DIR}/bin/"
             FILES_MATCHING PATTERN "*.dll")

        MESSAGE(STATUS "Configuring NSIS")
        
        # Copy and configure the nsis script file
        CONFIGURE_FILE(installer.nsi "${CMAKE_BINARY_DIR}/bin/installer.nsi" @ONLY)

        ADD_CUSTOM_TARGET(installer
            ${CMAKE_COMMAND} -E remove "${CMAKE_BINARY_DIR}/bin/${NSIS_OUTPUT_FILE}"
            COMMAND makensis "${CMAKE_BINARY_DIR}/bin/installer.nsi"
            WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/bin/"
            )

        ADD_DEPENDENCIES(installer vlmc)

    ELSE(EXISTS "${CMAKE_SOURCE_DIR}/contribs/")
        
        MESSAGE(WARNING "Contribs not available, you won't be able to create an installer. Read INSTALL.win32 for details.")
    
    ENDIF(EXISTS "${CMAKE_SOURCE_DIR}/contribs/")

ENDIF(WIN32)
