#
# VLMC build system
# Authors: Rohit Yadav <rohityadav89@gmail.com>
#          Ludovic Fauvet <etix@vlmc.org>
#          Hugo Beauzée-Luyssen <hugo@beauzee.fr>
#

include(FindPkgConfig)

if (APPLE)
    # Manually set Qt5 path for OSX installed using brew
    SET(CMAKE_PREFIX_PATH "/usr/local/opt/qt5")
endif()

# search for Qt5
find_package(Qt5Core QUIET)
if (Qt5Core_FOUND)
    # go on with other packages
    if(WITH_GUI)
        find_package(Qt5 COMPONENTS Widgets Gui Network)
    else()
        find_package(Qt5 COMPONENTS Network)
    endif()
endif()

# We need to use pkg-config, as the medialibrary will use the pkg-config provided
# libvlc
pkg_check_modules(LIBVLC REQUIRED libvlc>=3.0)

# There's a special place in hell for people who use libvlccore in their apps.
list(APPEND VLMC_LIBS ${LIBVLC_LIBRARIES} vlccore)
link_directories(${LIBVLC_LIBRARY_DIRS})
INCLUDE_DIRECTORIES(${LIBVLC_INCLUDE_DIRS})

# Find MLT Framework
pkg_check_modules(MLT REQUIRED mlt-framework>=6.3.0)
list(APPEND VLMC_LIBS ${MLT_LIBRARIES})
link_directories(${MLT_LIBRARY_DIRS})
INCLUDE_DIRECTORIES(${MLT_INCLUDE_DIRS})

pkg_check_modules(MLTPP REQUIRED mlt++>=6.3.0)
list(APPEND VLMC_LIBS ${MLTPP_LIBRARIES})
link_directories(${MLTPP_LIBRARY_DIRS})
INCLUDE_DIRECTORIES(${MLTPP_INCLUDE_DIRS})

FIND_PACKAGE(frei0r REQUIRED)

if(APPLE)
    find_library(FOUNDATION_FRAMEWORK Foundation)
    find_library(APPKIT_FRAMEWORK AppKit)
endif()

INCLUDE_DIRECTORIES(${FREI0R_INCLUDE_DIR})

# Instruct CMake to run moc automatically when needed.
SET(CMAKE_AUTOMOC ON)

SET(VLMC_SRCS
    Commands/Commands.cpp
    Backend/IBackend.h
    Backend/IService.h
    Backend/IOutput.h
    Backend/IProducer.h
    Backend/ITrack.h
    Backend/IProfile.h
    Backend/ITractor.h
    Backend/ITransition.h
    Backend/IFilter.h
    Backend/MLT/MLTBackend.cpp
    Backend/MLT/MLTOutput.cpp
    Backend/MLT/MLTProducer.cpp
    Backend/MLT/MLTTrack.cpp
    Backend/MLT/MLTService.cpp
    Backend/MLT/MLTProfile.cpp
    Backend/MLT/MLTFilter.cpp
    Backend/MLT/MLTTransition.cpp
    Backend/MLT/MLTTractor.cpp
    EffectsEngine/EffectHelper.cpp
    Library/Library.cpp
    Library/MediaContainer.cpp
    Main/Core.cpp
    Main/main.cpp
    Media/Clip.cpp
    Media/Media.cpp
    Project/Project.cpp
    Project/Workspace.cpp
    Project/WorkspaceWorker.cpp
    Project/RecentProjects.cpp
    Renderer/AbstractRenderer.cpp
    Services/AbstractSharingService.h
    Services/UploaderIODevice.cpp
    Settings/Settings.cpp
    Settings/SettingValue.cpp
    Tools/ErrorHandler.cpp
    Tools/RendererEventWatcher.cpp
    Tools/OutputEventWatcher.cpp
    Tools/Singleton.hpp
    Tools/Toggleable.hpp
    Tools/VlmcDebug.h
    Tools/VlmcLogger.cpp
    Workflow/Helper.cpp
    Workflow/MainWorkflow.cpp
    Workflow/TrackWorkflow.cpp
    Workflow/Types.h
  )

IF (WIN32)
    LIST ( APPEND VLMC_SRCS Main/winvlmc.cpp )
ELSE (WIN32)
    LIST( APPEND VLMC_SRCS Main/vlmc.cpp )
ENDIF(WIN32)

SET(VLMC_RCC ../resources.qrc)

IF (WIN32)
    LIST( APPEND VLMC_RCC ../resources/styles.qrc )
ENDIF (WIN32)

#include paths for vlmc
INCLUDE_DIRECTORIES(
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}/src
  .
  Gui
)

CONFIGURE_FILE(${CMAKE_SOURCE_DIR}/cmake/config.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config.h)

IF (NOT WITH_GUI)
    LIST (APPEND VLMC_SRCS Main/main.cpp Renderer/ConsoleRenderer.cpp Commands/AbstractUndoStack.cpp)
    qt5_add_resources(VLMC_RCC_SRCS ${VLMC_RCC})
    ADD_EXECUTABLE(vlmc ${GUI_TYPE} ${VLMC_SRCS} ${VLMC_MOC_SRCS} ${VLMC_RCC_SRCS})
    qt5_use_modules(vlmc Core Network)
    TARGET_LINK_LIBRARIES(vlmc ${VLMC_LIBS})

#Add GUI stuff if required
ELSE(NOT WITH_GUI)
     LIST( APPEND VLMC_SRCS
        Commands/KeyboardShortcutHelper.cpp
        Renderer/ClipRenderer.cpp
        Services/YouTube/YouTubeAuthenticator.cpp
        Services/YouTube/YouTubeFeedParser.cpp
        Services/YouTube/YouTubeService.cpp
        Services/YouTube/YouTubeUploader.cpp
        Gui/About.cpp
        Gui/ClipProperty.cpp
        Gui/IntroDialog.cpp
        Gui/LanguageHelper.cpp
        Gui/MainWindow.cpp
        Gui/WorkflowFileRendererDialog.cpp
        Gui/effectsengine/EffectInstanceWidget.cpp
        Gui/effectsengine/EffectInstanceListModel.cpp
        Gui/effectsengine/EffectsListView.cpp
        Gui/effectsengine/EffectStack.cpp
        Gui/effectsengine/EffectWidget.cpp
        Gui/export/RendererSettings.cpp
        Gui/export/ShareOnInternet.cpp
        Gui/import/ImportController.cpp
        Gui/import/TagWidget.cpp
        Gui/library/ListViewController.cpp
        Gui/library/MediaCellView.cpp
        Gui/library/MediaLibrary.cpp
        Gui/library/MediaListView.cpp
        Gui/library/StackViewController.cpp
        Gui/library/StackViewNavController.cpp
        Gui/library/ViewController.h
        Gui/media/ClipMetadataDisplayer.cpp
        Gui/preview/LCDTimecode.cpp
        Gui/preview/PreviewRuler.cpp
        Gui/preview/PreviewWidget.cpp
        Gui/settings/BoolWidget.cpp
        Gui/settings/ColorWidget.cpp
        Gui/settings/DoubleWidget.cpp
        Gui/settings/DoubleSliderWidget.cpp
        Gui/settings/IntWidget.cpp
        Gui/settings/ISettingsCategoryWidget.h
        Gui/settings/KeyboardShortcut.cpp
        Gui/settings/KeyboardShortcutInput.cpp
        Gui/settings/LanguageWidget.cpp
        Gui/settings/Panel.cpp
        Gui/settings/PathWidget.cpp
        Gui/settings/PreferenceWidget.cpp
        Gui/settings/SettingsDialog.cpp
        Gui/settings/StringWidget.cpp
        Gui/timeline/AbstractGraphicsItem.cpp
        Gui/timeline/AbstractGraphicsMediaItem.cpp
        Gui/timeline/GraphicsAudioItem.cpp
        Gui/timeline/GraphicsCursorItem.cpp
        Gui/timeline/GraphicsEffectItem.cpp
        Gui/timeline/GraphicsMovieItem.cpp
        Gui/timeline/GraphicsTrack.cpp
        Gui/timeline/Timeline.cpp
        Gui/timeline/TracksControls.cpp
        Gui/timeline/TracksRuler.cpp
        Gui/timeline/TracksScene.cpp
        Gui/timeline/TracksView.cpp
        Gui/widgets/ExtendedLabel.cpp
        Gui/widgets/FramelessButton.cpp
        Gui/widgets/NotificationZone.cpp
        Gui/widgets/SearchLineEdit.cpp
        Gui/widgets/TrackControls.cpp
        Gui/wizard/GeneralPage.cpp
        Gui/wizard/OpenPage.cpp
        Gui/wizard/ProjectWizard.cpp
        Gui/wizard/VideoPage.cpp
        Gui/wizard/WelcomePage.cpp
        Gui/wizard/firstlaunch/FirstLaunchWizard.cpp
        Gui/wizard/firstlaunch/FirstLaunchPage.cpp
        Gui/wizard/firstlaunch/WorkspaceLocation.cpp
        Gui/wizard/firstlaunch/Done.cpp
        )

    SET(VLMC_UIS
        Gui/effectsengine/ui/EffectInstanceWidget.ui
        Gui/effectsengine/ui/EffectStack.ui
        Gui/effectsengine/ui/EffectWidget.ui
        Gui/export/RendererSettings.ui
        Gui/export/ShareOnInternet.ui
        Gui/import/ui/ImportController.ui
        Gui/import/ui/TagWidget.ui
        Gui/library/ui/MediaCellView.ui
        Gui/library/ui/MediaLibrary.ui
        Gui/library/ui/StackViewNavController.ui
        Gui/media/ui/ClipMetadataDisplayer.ui
        Gui/preview/ui/PreviewWidget.ui
        Gui/ui/About.ui
        Gui/ui/ClipProperty.ui
        Gui/ui/IntroDialog.ui
        Gui/ui/MainWindow.ui
        Gui/ui/Timeline.ui
        Gui/ui/WorkflowFileRendererDialog.ui
        Gui/widgets/ui/TrackControls.ui
        Gui/widgets/ui/NotificationZone.ui
        Gui/wizard/ui/GeneralPage.ui
        Gui/wizard/ui/OpenPage.ui
        Gui/wizard/ui/VideoPage.ui
        Gui/wizard/ui/WelcomePage.ui
        Gui/wizard/ui/firstlaunch/FirstLaunchPage.ui
        Gui/wizard/ui/firstlaunch/WorkspaceLocation.ui
        Gui/wizard/ui/firstlaunch/Done.ui
        )

    IF (WITH_CRASHHANDLER_GUI)
        LIST( APPEND VLMC_SRCS Gui/widgets/CrashHandler.cpp )
        IF ( CMAKE_SYSTEM_NAME STREQUAL "Linux" )
            LIST( APPEND VLMC_SRCS Tools/UnixBacktraceGenerator.cpp )
        ELSE ( CMAKE_SYSTEM_NAME STREQUAL "Linux" )
            LIST( APPEND VLMC_SRCS Tools/Win32BacktraceGenerator.cpp )
        ENDIF( CMAKE_SYSTEM_NAME STREQUAL "Linux" )
        LIST( APPEND Gui/widgets/CrashHandler.h )
        LIST( APPEND VLMC_UIS Gui/widgets/ui/CrashHandler.ui )
    ENDIF(WITH_CRASHHANDLER_GUI)

    qt5_wrap_ui(VLMC_UIS_H ${VLMC_UIS})

    vlmc_get_ts(VLMC_QMS)
    vlmc_create_ts_resources("${VLMC_QMS}" "${CMAKE_CURRENT_BINARY_DIR}/ts-resource.qrc")
    list(APPEND VLMC_RCC ${CMAKE_CURRENT_BINARY_DIR}/ts-resource.qrc)

    qt5_add_resources(VLMC_RCC_SRCS ${VLMC_RCC})

    IF( MINGW )
        # resource compilation for MinGW
        ADD_CUSTOM_COMMAND( OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/icon.o
            COMMAND ${CMAKE_WINDRES} -I${CMAKE_SOURCE_DIR} -i${CMAKE_SOURCE_DIR}/cmake/icon.rc
            -o ${CMAKE_CURRENT_BINARY_DIR}/icon.o )
        SET(VLMC_SRCS ${VLMC_SRCS} ${CMAKE_CURRENT_BINARY_DIR}/icon.o)
        SET(LINK_FLAGS -Wl,-subsystem,windows)
    ENDIF( MINGW )

    IF( WIN32 )
        SET( GUI_TYPE WIN32 )
    ENDIF( WIN32 )

    IF( APPLE )
        SET( GUI_TYPE MACOSX_BUNDLE )
        SET(VLMC_SRCS ${VLMC_SRCS} ../share/VLMC.icns)
        SET_SOURCE_FILES_PROPERTIES(../share/VLMC.icns PROPERTIES
            MACOSX_PACKAGE_LOCATION Resources)
        SET(MACOSX_BUNDLE_ICON_FILE VLMC.icns)
        LIST( APPEND VLMC_SRCS Gui/preview/RenderWidget.mm )
        list( APPEND VLMC_LIBS ${FOUNDATION_FRAMEWORK} ${APPKIT_FRAMEWORK} )
    ELSE( APPLE )
        ## Since there's no implementation on non-apple platforms, we need to process
        ## the header to generate MOC
        LIST( APPEND VLMC_SRCS Gui/preview/RenderWidget.h )
    ENDIF( APPLE )

    ADD_EXECUTABLE( vlmc ${GUI_TYPE} ${VLMC_SRCS} ${VLMC_MOC_SRCS} ${VLMC_UIS_H} ${VLMC_RCC_SRCS} )
    TARGET_LINK_LIBRARIES( vlmc ${VLMC_LIBS} )
    if (NOT Qt4_FOUND)
        qt5_use_modules(vlmc Core Gui Widgets Network)
    else()
        # We need to know the target for this one, which is not the case when doing most of the QT4/5 specific stuff.
        # We could hide this command in qt_use_modules macro, but this would be some kind of black magic uglyness
        TARGET_LINK_LIBRARIES(vlmc ${QT_LIBRARIES})
    endif()
ENDIF( NOT WITH_GUI )

add_dependencies(vlmc libvlcpp)
message(STATUS "Contribs prefix: ${CONTRIBS_DIR}")
include_directories( ${CONTRIBS_DIR}/include )

INSTALL(TARGETS vlmc
        BUNDLE  DESTINATION ${VLMC_BIN_DIR}
        RUNTIME DESTINATION ${VLMC_BIN_DIR})

ADD_CUSTOM_COMMAND(
    OUTPUT ${CMAKE_SOURCE_DIR}/bin/vlmc
    COMMAND ${CMAKE_COMMAND} copy ${CMAKE_CURRENT_SOURCE_DIR}/vlmc ${CMAKE_SOURCE_DIR}/bin/vlmc
    DEPENDS ${CMAKE_BINARY_DIR}/src/vlmc
)

ADD_DEFINITIONS( -W -Wall -Wextra )

# Ensure we can use some C++11 features
target_compile_features(vlmc PUBLIC cxx_nullptr cxx_range_for)

IF(UNIX)
    IF (WITH_PROFILING)
        ADD_DEFINITIONS( -pg )
        SET_TARGET_PROPERTIES( vlmc PROPERTIES LINK_FLAGS -pg )
    ENDIF(WITH_PROFILING)
ENDIF(UNIX)

if( APPLE )
    set(APPLICATION_BUNDLE ${CMAKE_BINARY_DIR}/bin/vlmc.app)
    set(APPLICATION_LIB_DIR ${LIBVLC_LIB_DIR})

    # re-enable this when we start addressing redistribution
    # message(STATUS "Looking for bundle ${APPLICATION_BUNDLE} with library path ${APPLICATION_LIB_DIR}")
    # install(CODE "include(BundleUtilities)
    add_custom_command(TARGET vlmc
        POST_BUILD
        COMMAND ${CMAKE_SOURCE_DIR}/cmake/FixBundle.sh ${APPLICATION_BUNDLE}/Contents/MacOS ${LIBVLC_LIB_DIR}
        COMMENT "Fixing application bundle for local run")
    # fixup_bundle(\"${APPLICATION_BUNDLE}\" \"\" \"${APPLICATION_LIB_DIR}\")" COMPONENT Runtime)
endif()
