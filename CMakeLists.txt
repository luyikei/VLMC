#
# VLMC build system
# Authors: Rohit Yadav <rohityadav89@gmail.com>
#          Ludovic Fauvet <etix@vlmc.org>
#

# Project Info
SET(PROJECT_NAME_SHORT   "VLMC")
SET(PROJECT_NAME_LONG    "VideoLAN Movie Creator")
SET(PROJECT_DESCRIPTION  "VLMC (VideoLAN Movie Creator) is a cross-platform, non-linear video editing software based on the VLC Media Player.")
SET(PROJECT_COPYRIGHT    "Copyright (C) 2008-2010 VideoLAN")
SET(PROJECT_CONTACT      "vlmc-devel@videolan.org")
SET(PROJECT_VENDOR       "VLMC Team")
SET(ORG_WEBSITE          "videolan.org")

# Configuring build type
IF(NOT CMAKE_BUILD_TYPE)
    SET(CMAKE_BUILD_TYPE "Debug" CACHE STRING
        "Choose the type of build, options are: None Debug Release RelWithDebInfo MinSizeRel."
        FORCE)
    MESSAGE("Setting build type to '${CMAKE_BUILD_TYPE}'")
ELSE(NOT CMAKE_BUILD_TYPE)
    MESSAGE("Build type set to '${CMAKE_BUILD_TYPE}'")
ENDIF(NOT CMAKE_BUILD_TYPE)

IF(WIN32)
    # Building in release for WIN32
    SET(CMAKE_BUILD_TYPE "Release")
    SET_PROPERTY(DIRECTORY APPEND PROPERTY COMPILE_DEFINITIONS_RELEASE QT_NO_DEBUG)
    # Use mingw's sprintf instead of windows's
    ADD_DEFINITIONS( -D__USE_MINGW_ANSI_STDIO=1 )
ENDIF(WIN32)

# Find Git Version Patch   
FIND_PROGRAM(GIT git)
IF(GIT)
    EXECUTE_PROCESS(
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        COMMAND ${GIT} rev-parse --short HEAD 
        OUTPUT_VARIABLE GIT_OUT OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    SET(GIT_PATCH "git${GIT_OUT}")
ENDIF(GIT)

# version: 0.2.x
SET(CPACK_PACKAGE_VERSION_MAJOR "0")
SET(CPACK_PACKAGE_VERSION_MINOR "2")
SET(CPACK_PACKAGE_VERSION_PATCH "0")
SET(CPACK_PACKAGE_VERSION ${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH})
SET(PROJECT_VERSION_MAJOR ${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR})
SET(PROJECT_VERSION ${CPACK_PACKAGE_VERSION})

MESSAGE(STATUS "VLMC Version: ${PROJECT_VERSION}")

STRING(TOLOWER ${PROJECT_NAME_SHORT} PROJECT_NAME_SHORT_LOWER)
PROJECT(${PROJECT_NAME_SHORT_LOWER}-${PROJECT_VERSION})

# Codename of the Project :-)
SET(CODENAME "alpha")

# System Information
SITE_NAME(HOSTNAME)
SET(SYSNAME ${CMAKE_SYSTEM}-${CMAKE_SYSTEM_PROCESSOR})

# CMake Settings
CMAKE_MINIMUM_REQUIRED(VERSION 2.6.0)
SET(CMAKE_COLOR_MAKEFILE ON)
SET(CMAKE_VERBOSE_MAKEFILE OFF)

# Set path to additional CMake modules
SET(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake ${CMAKE_MODULE_PATH})

# Path to contribs
SET(CONTRIBS_PATH ${CMAKE_CURRENT_SOURCE_DIR}/contribs)

# Optionally allow embedding libvlc into the binary as
# VLMC requires libVLC
# SET (WITH_LIBVLC FALSE CACHE BOOL "Compile LibVLC in VLMC")

IF (UNIX)
    SET(WITH_PROFILING FALSE CACHE BOOL "Build with profiling support" )
    SET(CMAKE_INSTALL_PREFIX /usr)
ENDIF(UNIX)

SET(WITH_GUI TRUE CACHE BOOL "Enable the VLMC's GUI")
SET(WITH_CRASHBUTTON FALSE CACHE BOOL "Enable the crash button")

IF(NOT APPLE)
    SET(WITH_CRASHHANDLER_GUI TRUE CACHE BOOL "Enable the crash handler GUI (with backtrace and restart capabilities)")
    SET(WITH_CRASHHANDLER TRUE CACHE BOOL "Enable the crash handler")
ELSE(NOT APPLE)
    #FIXME: Mac OS port does n't start if crash handler is enabled.
    SET(WITH_CRASHHANDLER_GUI FALSE CACHE BOOL "Disable crash handler gui...")
    SET(WITH_CRASHHANDLER False CACHE BOOL "Disable the crash handler")
    SET(CMAKE_BUILD_TYPE "Release")
    SET(CMAKE_OSX_ARCHITECTURES i386)
#   SET(CMAKE_INSTALL_PREFIX "${CMAKE_CURRENT_BINARY_DIR}")
ENDIF(NOT APPLE)

FIND_PACKAGE(LIBVLC)
  IF (NOT LIBVLC_FOUND)
    MESSAGE (SEND_ERROR "LibVLC dependency was not found (please get libvlc)!")
  ENDIF (NOT LIBVLC_FOUND)

INCLUDE_DIRECTORIES(${LIBVLC_INCLUDE_DIR})

FIND_PACKAGE(frei0r)
IF (NOT FREI0R_FOUND)
  MESSAGE (SEND_ERROR "frei0r not found! It is required for effects engine to work.")
ENDIF (NOT FREI0R_FOUND)

INCLUDE_DIRECTORIES(${FREI0R_INCLUDE_DIR})

# search for Qt4
IF (WITH_GUI)
    IF(WIN32)
        #FindQt4.cmake is so brain-dead when Xcompiling
        SET(QT_LIBRARY_DIR ${CONTRIBS_PATH}/lib)
        SET(QT_INCLUDE_DIR ${CONTRIBS_PATH}/include/qt4)
        ADD_DEFINITIONS( -I${CONTRIBS_PATH}/include/qt4/QtCore)
        ADD_DEFINITIONS( -I${CONTRIBS_PATH}/include/qt4/QtGui)
        ADD_DEFINITIONS( -I${CONTRIBS_PATH}/include/qt4/QtXml)
        ADD_DEFINITIONS( -I${CONTRIBS_PATH}/include/qt4/QtNetwork)
        ADD_DEFINITIONS( -I${CONTRIBS_PATH}/include/qt4/)
        ADD_DEFINITIONS( -DQT_NO_OPENSSL)
        FIND_PACKAGE(Qt4 4.5.1 COMPONENTS QtCore QtXml QtGui QtNetwork REQUIRED )
    ELSE(WIN32)
        FIND_PACKAGE(Qt4 4.5.1 COMPONENTS QtCore QtXml QtGui QtNetwork REQUIRED )
    ENDIF(WIN32)
ELSE(WITH_GUI)
    FIND_PACKAGE (Qt4 4.5.1 COMPONENTS QtCore QtNetwork QtXml REQUIRED)
ENDIF(WITH_GUI)

IF (NOT WIN32)
    #FindQt4.cmake is so brain-dead when Xcompiling
    SET (VLMC_LIBS ${QT_QTCORE_LIBRARY}
          ${QT_QTGUI_LIBRARY}
          ${QT_QTNETWORK_LIBRARY}
          ${QT_QTSVG_LIBRARY}
          ${QT_QTXML_LIBRARY}
          ${LIBVLC_LIBRARY}
          ${LIBVLCCORE_LIBRARY})

    IF (APPLE) # Fails to link to Cocoa framework automatically
        SET (VLMC_LIBS ${VLMC_LIBS}
                       ${CMAKE_OSX_SYSROOT}/System/Library/Frameworks/Cocoa.framework)
    ELSE (APPLE)
        IF (WITH_GUI)
            SET (VLMC_LIBS ${VLMC_LIBS} -lX11)
        ENDIF (WITH_GUI)
    ENDIF(APPLE)

    INCLUDE(${QT_USE_FILE})
    ADD_DEFINITIONS(${QT_DEFINITIONS})
ELSE (NOT WIN32)
    SET (VLMC_LIBS
            ${CONTRIBS_PATH}/lib/libQtNetwork.a ${CONTRIBS_PATH}/lib/libQtXml.a
            ${CONTRIBS_PATH}/lib/libqtaccessiblewidgets.a
            ${CONTRIBS_PATH}/lib/libQtGui.a #${CONTRIBS_PATH}/lib/libqjpeg.a
            ${CONTRIBS_PATH}/lib/libqtaccessiblewidgets.a
            ${CONTRIBS_PATH}/lib/libQtCore.a
            ${CONTRIBS_PATH}/lib/libvlccore.dll.a
            ${CONTRIBS_PATH}/lib/libvlc.dll.a
            ws2_32 winmm imm32)
ENDIF(NOT WIN32)

# layout of VLMC installation
# (default values are platform-specific)
SET (CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
SET (CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/effects)

# Copy the sh launcher
IF (NOT WIN32)
    CONFIGURE_FILE("${CMAKE_CURRENT_SOURCE_DIR}/cmake/vlmc.sh" "${CMAKE_CURRENT_BINARY_DIR}/vlmc" COPYONLY)
ENDIF(NOT WIN32)

# common for MAC and UNIX
SET (DEFAULT_BIN_SUBDIR     bin)
SET (DEFAULT_LIB_SUBDIR     lib)
SET (DEFAULT_DATA_SUBDIR    share)
SET (DEFAULT_SRC_SUBDIR     src)
SET (DEFAULT_INCLUDE_SUBDIR include)

SET (VLMC_BIN_SUBDIR     "${DEFAULT_BIN_SUBDIR}"        CACHE STRING "Subdirectory where executables will be installed")
SET (VLMC_LIB_SUBDIR     "${DEFAULT_LIB_SUBDIR}"        CACHE STRING "Subdirectory where libraries will be installed")
SET (VLMC_DATA_SUBDIR    "${DEFAULT_DATA_SUBDIR}"       CACHE STRING "Subdirectory where VLMC data will be installed")
SET (VLMC_SRC_SUBDIR     "${DEFAULT_SRC_SUBDIR}"        CACHE STRING "Subdirectory where VLMC source is")
SET (VLMC_INCLUDE_SUBDIR "${DEFAULT_INCLUDE_SUBDIR}"    CACHE STRING "Subdirectory where header files will be installed")

# full paths for the installation
SET (VLMC_BIN_DIR     ${CMAKE_INSTALL_PREFIX}/${VLMC_BIN_SUBDIR})
SET (VLMC_LIB_DIR     ${CMAKE_INSTALL_PREFIX}/${VLMC_LIB_SUBDIR})
SET (VLMC_INCLUDE_DIR ${CMAKE_INSTALL_PREFIX}/${VLMC_INCLUDE_SUBDIR})
SET (VLMC_DATA_DIR    ${CMAKE_INSTALL_PREFIX}/${VLMC_DATA_SUBDIR}/vlmc/)
SET (VLMC_DOC_DIR     ${CMAKE_INSTALL_PREFIX}/${VLMC_DATA_SUBDIR}/doc/vlmc/)
SET (VLMC_EFFECTS_DIR ${CMAKE_INSTALL_PREFIX}/${VLMC_LIB_SUBDIR}/vlmc/effects/)
SET (VLMC_DESKTOP_DIR ${CMAKE_INSTALL_PREFIX}/${VLMC_DATA_SUBDIR}/applications/)
SET (VLMC_PIXMAPS_DIR ${CMAKE_INSTALL_PREFIX}/${VLMC_DATA_SUBDIR}/pixmaps/)

# manual page - makes sense only on unix systems
IF (UNIX)
    SET (DEFAULT_MANUAL_SUBDIR  ${VLMC_DATA_SUBDIR}/man)
    SET (VLMC_MANUAL_SUBDIR  "${DEFAULT_MANUAL_SUBDIR}"  CACHE STRING "Subdirectory where manual files will be installed")
    MARK_AS_ADVANCED (VLMC_MANUAL_SUBDIR)
    SET (VLMC_MANUAL_DIR  ${CMAKE_INSTALL_PREFIX}/${VLMC_MANUAL_SUBDIR})
ENDIF (UNIX)

SUBDIRS(ts)
SUBDIRS(src)
SUBDIRS(nsis)
SUBDIRS(effects)

# Copy stuff to doc subdirectory
INSTALL (FILES AUTHORS COPYING INSTALL NEWS README THANKS TRANSLATORS
         DESTINATION ${VLMC_DOC_DIR})

# Copy .desktop file
INSTALL (FILES share/vlmc.desktop DESTINATION ${VLMC_DESKTOP_DIR})

# Copy the VLMC pixmap
INSTALL (FILES share/vlmc.png DESTINATION ${VLMC_PIXMAPS_DIR})

# On UNIX copy the man page
IF (UNIX)
    INSTALL (FILES ${CMAKE_SOURCE_DIR}/doc/vlmc.1
             DESTINATION ${VLMC_MANUAL_DIR}/man1)
ENDIF (UNIX)

# Copy the share directory (TODO: add checks if the dir is empty, don't install)
# INSTALL (DIRECTORY share/ DESTINATION ${VLMC_DATA_DIR}
#         PATTERN "*.desktop" EXCLUDE
#         PATTERN "*.ico" EXCLUDE
#         PATTERN "*.icns" EXCLUDE
#         PATTERN "vlmc.png" EXCLUDE)

#############################################################
# Package Settings

INCLUDE(InstallRequiredSystemLibraries)

SET(CPACK_SYSTEM_NAME ${CMAKE_SYSTEM_NAME})
SET(CPACK_PACKAGE_NAME ${PROJECT_NAME_SHORT})
SET(CPACK_PACKAGE_DESCRIPTION ${PROJECT_NAME_LONG})
SET(CPACK_PACKAGE_DESCRIPTION_SUMMARY "${PROJECT_NAME_SHORT} \n ${PROJECT_DESCRIPTION}")
SET(CPACK_PACKAGE_VENDOR ${PROJECT_VENDOR})
SET(CPACK_PACKAGE_CONTACT ${PROJECT_CONTACT})
SET(CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_SOURCE_DIR}/README")
SET(CPACK_RESOURCE_FILE_README ${CMAKE_SOURCE_DIR}/README)
SET(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/COPYING")
SET(CPACK_PACKAGE_INSTALL_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}")

# Force Package Name
SET(CPACK_PACKAGE_FILE_NAME ${PROJECT_NAME}${GIT_PATCH}-${CPACK_SYSTEM_NAME}-${CMAKE_SYSTEM_PROCESSOR})

# NSIS Windows Installer
IF(WIN32 AND NOT UNIX)
    # There is a bug in NSI that does not handle full unix paths properly. Make
    # sure there is at least one set of four (4) backslashes.
    SET(CPACK_PACKAGE_ICON "${CMAKE_SOURCE_DIR}/share\\\\vlmc.png")
    SET(CPACK_NSIS_INSTALLED_ICON_NAME "\\\\vlmc.exe")
    SET(CPACK_NSIS_DISPLAY_NAME "${CPACK_PACKAGE_INSTALL_DIRECTORY} VLMC")
    SET(CPACK_NSIS_HELP_LINK "http:\\\\\\\\vlmc.org")
    SET(CPACK_NSIS_URL_INFO_ABOUT "http:\\\\\\\\vlmc.org")
    SET(CPACK_NSIS_CONTACT ${PROJECT_CONTACT})
    SET(CPACK_NSIS_MODIFY_PATH ON)
ELSE(WIN32 AND NOT UNIX)
    #SET(CPACK_STRIP_FILES "VLMC")
    #SET(CPACK_SOURCE_STRIP_FILES "")
ENDIF(WIN32 AND NOT UNIX)

SET(PACKAGE_REQUIRES "libvlc-dev ( >= 1.1.4 ), frei0r-plugins, libqt4-gui ( >= 4.6 ), libqt4-network ( >= 4.6 ), libqt4-svg ( >= 4.6 ), libqt4-xml ( >= 4.6 )")

# RPM packages
INCLUDE ( ${CMAKE_MODULE_PATH}/RpmBuild.cmake )
IF ( RPMBUILD_FOUND AND NOT WIN32 )
    SET(CPACK_GENERATOR "RPM")
    SET(CPACK_SET_DESTDIR TRUE)
    SET(CPACK_PACKAGE_DESCRIPTION_SUMMARY "${PROJECT_DESCRIPTION}")
ENDIF ( RPMBUILD_FOUND AND NOT WIN32 )

# Debian packages
INCLUDE ( ${CMAKE_MODULE_PATH}/DpkgBuild.cmake )
IF ( DPKG_FOUND AND NOT WIN32 )
    SET(CPACK_GENERATOR "DEB")
    SET(CPACK_SET_DESTDIR TRUE)
    SET(CPACK_DEBIAN_PACKAGE_DEPENDS ${PACKAGE_REQUIRES})
ENDIF ( DPKG_FOUND AND NOT WIN32 )

# Mac Bundle
IF( APPLE )
    SET(CPACK_GENERATOR "DragNDrop")

    #Libraries are bundled directly
    SET(CPACK_COMPONENT_LIBRARIES_HIDDEN TRUE)

    # Bundle Properties
    SET(MACOSX_BUNDLE_BUNDLE_NAME vlmc)
    SET(MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION})
    SET(MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION})
    SET(MACOSX_BUNDLE_LONG_VERSION_STRING "Version ${PROJECT_VERSION}")

    #SET(CPACK_STRIP_FILES TRUE)
ENDIF( APPLE )

SET(CPACK_PACKAGE_EXECUTABLES "vlmc" "VLMC")
INCLUDE(CPack)
