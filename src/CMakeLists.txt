#
# VLMC build system
# Authors: Rohit Yadav <rohityadav89@gmail.com>
#          Ludovic Fauvet <etix@vlmc.org>
#

SET(VLMC_SRCS
    Commands/Commands.cpp
    Settings/SettingsManager.cpp
    Settings/SettingValue.cpp
    Library/Library.cpp
    Library/MediaContainer.cpp
    LibVLCpp/VLCInstance.cpp
    LibVLCpp/VLCMedia.cpp
    LibVLCpp/VLCMediaPlayer.cpp
    LibVLCpp/VLCpp.hpp
    Media/Clip.cpp
    Media/Media.cpp
    Metadata/MetaDataManager.cpp
    Metadata/MetaDataWorker.cpp
    Project/ProjectManager.cpp
    Project/Workspace.cpp
    Project/WorkspaceWorker.cpp
    Renderer/ClipRenderer.cpp
    Renderer/GenericRenderer.cpp
    Renderer/WorkflowFileRenderer.cpp
    Renderer/WorkflowRenderer.cpp
    Tools/ErrorHandler.cpp
    Tools/Pool.hpp
    Tools/QSingleton.hpp
    Tools/Singleton.hpp
    Tools/Toggleable.hpp
    Tools/VlmcDebug.cpp
    Tools/WaitCondition.hpp
    Workflow/AudioClipWorkflow.cpp 
    Workflow/ClipWorkflow.cpp
    Workflow/ClipHelper.cpp
    Workflow/ImageClipWorkflow.cpp
    Workflow/MainWorkflow.cpp
    Workflow/StackedBuffer.hpp
    Workflow/TrackHandler.cpp
    Workflow/TrackWorkflow.cpp
    Workflow/Types.cpp
    Workflow/VideoClipWorkflow.cpp
  )

IF (WIN32)
    LIST ( APPEND VLMC_SRCS Main/winvlmc.cpp )
ELSE (WIN32)
    LIST( APPEND VLMC_SRCS Main/vlmc.cpp )
ENDIF(WIN32)

SET (VLMC_HDRS
    Settings/SettingsManager.h
    Settings/SettingValue.h
    Library/Library.h
    Library/MediaContainer.h
    LibVLCpp/VLCInstance.h
    LibVLCpp/VLCMediaPlayer.h
    Media/Clip.h
    Media/Media.h
    Metadata/MetaDataManager.h
    Metadata/MetaDataWorker.h
    Project/ProjectManager.h
    Project/Workspace.h
    Project/WorkspaceWorker.h
    Renderer/ClipRenderer.h
    Renderer/GenericRenderer.h
    Renderer/WorkflowFileRenderer.h
    Renderer/WorkflowRenderer.h
    Tools/ErrorHandler.h
    Tools/VlmcDebug.h
    Workflow/AudioClipWorkflow.h 
    Workflow/ClipWorkflow.h
    Workflow/ClipHelper.h
    Workflow/ImageClipWorkflow.h
    Workflow/MainWorkflow.h
    Workflow/TrackHandler.h
    Workflow/TrackWorkflow.h
    Workflow/Types.h
    Workflow/VideoClipWorkflow.h
  )

SET(VLMC_RCC
    ../ressources.qrc
    ../ts/ressources-ts.qrc
  )

IF (WIN32)
    LIST( APPEND VLMC_RCC ../resources/styles.qrc )
ENDIF (WIN32)

#include paths for vlmc
INCLUDE_DIRECTORIES(
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}/src
  .
  Commands
  Settings 
  Gui
  Gui/import
  Gui/library
  Gui/media
  Gui/preview
  Gui/settings
  Gui/timeline
  Gui/widgets
  Gui/wizard
  Library
  LibVLCpp
  Media
  Metadata
  Project
  Renderer
  Tools
  Workflow
)

CONFIGURE_FILE(${CMAKE_SOURCE_DIR}/cmake/config.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config.h)

IF (NOT WITH_GUI)
    LIST (APPEND VLMC_SRCS Main/main.cpp Renderer/ConsoleRenderer.cpp )
    LIST (APPEND VLMC_HDRS Renderer/ConsoleRenderer.h)
    QT4_ADD_RESOURCES(VLMC_RCC_SRCS ${VLMC_RCC})
    QT4_WRAP_CPP(VLMC_MOC_SRCS ${VLMC_HDRS})
    ADD_EXECUTABLE(vlmc ${GUI_TYPE} ${VLMC_SRCS} ${VLMC_MOC_SRCS} ${VLMC_RCC_SRCS})
    TARGET_LINK_LIBRARIES(vlmc
      ${QT_QTCORE_LIBRARY} 
      ${QT_QTXML_LIBRARY} 
      ${LIBVLC_LIBRARY}
      ${LIBVLCCORE_LIBRARY}
      )

#Add GUI stuff if required
ELSE(NOT WITH_GUI)
     LIST( APPEND VLMC_SRCS
        Commands/KeyboardShortcutHelper.cpp
        Main/guimain.cpp
        Media/Transcoder.cpp #Won't be needed without the import, so let's put it in GUI list
        Gui/About.cpp
        Gui/AudioSpectrumDrawer.cpp
        Gui/ClickableLabel.cpp
        Gui/ClipProperty.cpp
        Gui/DockWidgetManager.cpp
        Gui/IntroDialog.cpp
        Gui/LanguageHelper.cpp
        Gui/preview/LCDTimecode.cpp
        Gui/MainWindow.cpp
        Gui/import/TagWidget.cpp
        Gui/UndoStack.cpp
        Gui/WorkflowFileRendererDialog.cpp
        Gui/export/RendererSettings.cpp
        Gui/export/ShareOnYoutube.cpp
        Gui/import/ImportController.cpp
        Gui/library/ListViewController.cpp
        Gui/library/MediaCellView.cpp
        Gui/library/MediaLibrary.cpp
        Gui/library/MediaListView.cpp
        Gui/library/StackViewController.cpp
        Gui/library/StackViewNavController.cpp
        Gui/media/GuiMedia.cpp
        Gui/media/ClipMetadataDisplayer.cpp
        Gui/project/GuiProjectManager.cpp
        Gui/preview/PreviewRuler.cpp
        Gui/preview/PreviewWidget.cpp
        Gui/settings/BoolWidget.cpp
        Gui/settings/DoubleWidget.cpp
        Gui/settings/IntWidget.cpp
        Gui/settings/KeyboardShortcut.cpp
        Gui/settings/KeyboardShortcutInput.cpp
        Gui/settings/LanguageWidget.cpp
        Gui/settings/Panel.cpp
        Gui/settings/PathWidget.cpp
        Gui/settings/PreferenceWidget.cpp
        Gui/settings/Settings.cpp
        Gui/settings/StringWidget.cpp
        Gui/timeline/AbstractGraphicsMediaItem.cpp
        Gui/timeline/GraphicsAudioItem.cpp
        Gui/timeline/GraphicsCursorItem.cpp
        Gui/timeline/GraphicsMovieItem.cpp
        Gui/timeline/GraphicsTrack.cpp
        Gui/timeline/Timeline.cpp
        Gui/timeline/TracksControls.cpp
        Gui/timeline/TracksRuler.cpp
        Gui/timeline/TracksScene.cpp
        Gui/timeline/TracksView.cpp
        Gui/widgets/ElidableLabel.cpp
        Gui/widgets/FramelessButton.cpp
        Gui/widgets/TrackControls.cpp
        Gui/wizard/GeneralPage.cpp
        Gui/wizard/OpenPage.cpp
        Gui/wizard/ProjectWizard.cpp
        Gui/wizard/VideoPage.cpp
        Gui/wizard/WelcomePage.cpp
        )

    LIST( APPEND VLMC_HDRS
        Commands/KeyboardShortcutHelper.h
        Media/Transcoder.h #Won't be needed without the import, so let's put it in GUI list
        Gui/About.h
        Gui/ClickableLabel.h
        Gui/ClipProperty.h
        Gui/DockWidgetManager.h
        Gui/export/RendererSettings.h
        Gui/export/ShareOnYoutube.h
        Gui/import/ImportController.h
        Gui/IntroDialog.h
        Gui/LanguageHelper.h
        Gui/import/TagWidget.h
        Gui/library/ListViewController.h
        Gui/library/MediaCellView.h
        Gui/library/MediaLibrary.h
        Gui/library/MediaListView.h
        Gui/library/StackViewController.h
        Gui/library/StackViewNavController.h
        Gui/library/ViewController.h
        Gui/MainWindow.h
        Gui/media/ClipMetadataDisplayer.h
        Gui/media/GuiMedia.h
        Gui/preview/LCDTimecode.h
        Gui/preview/PreviewRuler.h
        Gui/preview/PreviewWidget.h
        Gui/project/GuiProjectManager.h
        Gui/settings/KeyboardShortcut.h
        Gui/settings/KeyboardShortcutInput.h
        Gui/settings/BoolWidget.h
        Gui/settings/DoubleWidget.h
        Gui/settings/IntWidget.h
        Gui/settings/ISettingsCategoryWidget.h
        Gui/settings/LanguageWidget.h
        Gui/settings/PathWidget.h
        Gui/settings/PreferenceWidget.h
        Gui/settings/StringWidget.h
        Gui/settings/Panel.h
        Gui/settings/PreferenceWidget.h
        Gui/settings/Settings.h
        Gui/timeline/AbstractGraphicsMediaItem.h
        Gui/timeline/GraphicsAudioItem.h 
        Gui/timeline/GraphicsCursorItem.h
        Gui/timeline/GraphicsMovieItem.h 
        Gui/timeline/GraphicsTrack.h
        Gui/timeline/Timeline.h
        Gui/timeline/TracksControls.h
        Gui/timeline/TracksRuler.h
        Gui/timeline/TracksScene.h
        Gui/timeline/TracksView.h
        Gui/UndoStack.h
        Gui/widgets/ElidableLabel.h
        Gui/widgets/FramelessButton.h
        Gui/widgets/TrackControls.h
        Gui/wizard/GeneralPage.h
        Gui/wizard/OpenPage.h
        Gui/wizard/ProjectWizard.h
        Gui/wizard/VideoPage.h
        Gui/wizard/WelcomePage.h
        Gui/WorkflowFileRendererDialog.h
        )
     
    SET(VLMC_UIS
        Gui/export/RendererSettings.ui
        Gui/export/ShareOnYoutube.ui
        Gui/import/ui/ImportController.ui
        Gui/library/StackViewNavController.ui
        Gui/library/ui/MediaCellView.ui
        Gui/library/ui/MediaLibrary.ui
        Gui/library/ui/StackViewNavController.ui
        Gui/ui/About.ui
        Gui/ui/ClipProperty.ui
        Gui/ui/IntroDialog.ui
        Gui/ui/MainWindow.ui
        Gui/media/ui/ClipMetadataDisplayer.ui
        Gui/preview/ui/PreviewWidget.ui
        Gui/import/ui/TagWidget.ui
        Gui/ui/Timeline.ui
        Gui/ui/WorkflowFileRendererDialog.ui
        Gui/widgets/TrackControls.ui
        Gui/wizard/ui/GeneralPage.ui
        Gui/wizard/ui/OpenPage.ui
        Gui/wizard/ui/VideoPage.ui
        Gui/wizard/ui/WelcomePage.ui
        )

    IF (WITH_CRASHHANDLER_GUI)
        LIST( APPEND VLMC_SRCS Gui/widgets/CrashHandler.cpp )
        IF ( CMAKE_SYSTEM_NAME STREQUAL "Linux" )
            LIST( APPEND VLMC_SRCS Tools/UnixBacktraceGenerator.cpp )
        ELSE ( CMAKE_SYSTEM_NAME STREQUAL "Linux" )
            LIST( APPEND VLMC_SRCS Tools/Win32BacktraceGenerator.cpp )
        ENDIF( CMAKE_SYSTEM_NAME STREQUAL "Linux" )
        LIST( APPEND VLMC_HDRS Gui/widgets/CrashHandler.h )
        LIST( APPEND VLMC_UIS Gui/widgets/CrashHandler.ui )
    ENDIF(WITH_CRASHHANDLER_GUI)

    QT4_WRAP_UI(VLMC_UIS_H ${VLMC_UIS})
    QT4_ADD_RESOURCES(VLMC_RCC_SRCS ${VLMC_RCC})
    QT4_WRAP_CPP(VLMC_MOC_SRCS ${VLMC_HDRS})

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
    ENDIF( APPLE )

    ADD_EXECUTABLE( vlmc ${GUI_TYPE} ${VLMC_SRCS} ${VLMC_MOC_SRCS} ${VLMC_UIS_H} ${VLMC_RCC_SRCS} )
    TARGET_LINK_LIBRARIES(vlmc
      ${QT_QTCORE_LIBRARY} 
      ${QT_QTGUI_LIBRARY} 
      ${QT_QTXML_LIBRARY} 
      ${QT_QTSVG_LIBRARY} 
      ${LIBVLC_LIBRARY}
      ${LIBVLCCORE_LIBRARY}
      )

ENDIF( NOT WITH_GUI )

ADD_DEPENDENCIES( vlmc translations )

INSTALL(TARGETS vlmc 
        BUNDLE  DESTINATION ${VLMC_BIN_DIR}
        RUNTIME DESTINATION ${VLMC_BIN_DIR})

ADD_CUSTOM_COMMAND(
    OUTPUT ${CMAKE_SOURCE_DIR}/bin/vlmc
    COMMAND ${CMAKE_COMMAND} copy ${CMAKE_CURRENT_SOURCE_DIR}/vlmc ${CMAKE_SOURCE_DIR}/bin/vlmc
    DEPENDS ${CMAKE_BINARY_DIR}/src/vlmc
)

ADD_DEFINITIONS( -W -Wall )

IF(UNIX)
    IF (WITH_PROFILING)
        ADD_DEFINITIONS( -pg )
        SET_TARGET_PROPERTIES( vlmc PROPERTIES LINK_FLAGS -pg )
    ENDIF(WITH_PROFILING)
ENDIF(UNIX)

IF( APPLE )
    ADD_CUSTOM_COMMAND(
        TARGET vlmc
        POST_BUILD
        COMMAND cp -r ${CMAKE_SOURCE_DIR}/contribs/* ${CMAKE_CURRENT_BINARY_DIR}/../bin/vlmc.app/Contents/MacOS
    )

    ADD_CUSTOM_COMMAND(
        TARGET vlmc
        POST_BUILD
        COMMAND macdeployqt ${CMAKE_CURRENT_BINARY_DIR}/../bin/vlmc.app -no-plugins #-dmg
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Deploying the Qt Framework onto the bundle"
    )
ENDIF( APPLE )
