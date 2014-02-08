#
# VLMC build system
# Authors: Rohit Yadav <rohityadav89@gmail.com>
#          Ludovic Fauvet <etix@vlmc.org>
#

SET(VLMC_SRCS
    Commands/Commands.cpp
    EffectsEngine/EffectsEngine.cpp
    EffectsEngine/Effect.cpp
    EffectsEngine/EffectUser.cpp
    EffectsEngine/EffectHelper.cpp
    EffectsEngine/EffectInstance.cpp
    EffectsEngine/EffectSettingValue.cpp
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
    Services/AbstractSharingService.h
    Services/UploaderIODevice.cpp
    Services/YouTube/YouTubeAuthenticator.cpp
    Services/YouTube/YouTubeFeedParser.cpp
    Services/YouTube/YouTubeService.cpp
    Services/YouTube/YouTubeUploader.cpp
    Settings/SettingsManager.cpp
    Settings/SettingValue.cpp
    Tools/ErrorHandler.cpp
    Tools/QSingleton.hpp
    Tools/Singleton.hpp
    Tools/Toggleable.hpp
	Tools/VlmcDebug.h
	Tools/VlmcLogger.cpp
    Workflow/AudioClipWorkflow.cpp
    Workflow/ClipWorkflow.cpp
    Workflow/ClipHelper.cpp
    Workflow/Helper.cpp
    Workflow/ImageClipWorkflow.cpp
    Workflow/MainWorkflow.cpp
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

SET(VLMC_RCC
    ../resources.qrc
    ../ts/resources-ts.qrc
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
  EffectsEngine
  Settings
  Gui
  Gui/effectsengine
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
  Services
  Services/YouTube
  Tools
  Workflow
)

CONFIGURE_FILE(${CMAKE_SOURCE_DIR}/cmake/config.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config.h)

IF (NOT WITH_GUI)
    LIST (APPEND VLMC_SRCS Main/main.cpp Renderer/ConsoleRenderer.cpp )
    QT_ADD_RESOURCES(VLMC_RCC_SRCS ${VLMC_RCC})
    ADD_EXECUTABLE(vlmc ${GUI_TYPE} ${VLMC_SRCS} ${VLMC_MOC_SRCS} ${VLMC_RCC_SRCS})
    qt_use_modules(vlmc Core Xml Network)
    TARGET_LINK_LIBRARIES(vlmc ${VLMC_LIBS})

#Add GUI stuff if required
ELSE(NOT WITH_GUI)
     LIST( APPEND VLMC_SRCS
        Commands/KeyboardShortcutHelper.cpp
        Gui/About.cpp
        Gui/AudioSpectrumDrawer.cpp
        Gui/ClipProperty.cpp
        Gui/DockWidgetManager.cpp
        Gui/IntroDialog.cpp
        Gui/LanguageHelper.cpp
        Gui/MainWindow.cpp
        Gui/UndoStack.cpp
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
        Gui/media/GuiMedia.cpp
        Gui/preview/LCDTimecode.cpp
        Gui/preview/PreviewRuler.cpp
        Gui/preview/PreviewWidget.cpp
        Gui/project/GuiProjectManager.cpp
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
        Gui/settings/Settings.cpp
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
        Main/guimain.cpp
        Media/Transcoder.cpp #Won't be needed without the import, so let's put it in GUI list
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

    QT_WRAP_UI(VLMC_UIS_H ${VLMC_UIS})
    QT_ADD_RESOURCES(VLMC_RCC_SRCS ${VLMC_RCC})

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
    ELSE( APPLE )
        ## Since there's no implementation on non-apple platforms, we need to process
        ## the header to generate MOC
        LIST( APPEND VLMC_SRCS Gui/preview/RenderWidget.h )
    ENDIF( APPLE )

    ADD_EXECUTABLE( vlmc ${GUI_TYPE} ${VLMC_SRCS} ${VLMC_MOC_SRCS} ${VLMC_UIS_H} ${VLMC_RCC_SRCS} )
    TARGET_LINK_LIBRARIES( vlmc ${VLMC_LIBS} )
	if (NOT Qt4_FOUND)
	    qt_use_modules(vlmc Core Gui Widgets Xml Network)
	else()
		# We need to know the target for this one, which is not the case when doing most of the QT4/5 specific stuff.
		# We could hide this command in qt_use_modules macro, but this would be some kind of black magic uglyness
		TARGET_LINK_LIBRARIES(vlmc ${QT_LIBRARIES})
	endif()

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
