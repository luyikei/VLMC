#CMakeQt by Rohit Yadav<rohityadav89@gmail.com>
#CMake Build System for Qt

SUBDIRS(EffectsEngine/Plugins/src)

SET(VLMC_SRCS
    main.cpp
    Commands/Commands.cpp
    Commands/KeyboardShortcutHelper.cpp
    Settings/SettingsManager.cpp
    Settings/SettingValue.cpp
    EffectsEngine/EffectNode.cpp
    EffectsEngine/EffectNodeFactory.cpp
    EffectsEngine/EffectPluginTypeLoader.cpp
    EffectsEngine/EffectPluginTypeManager.cpp
    EffectsEngine/EffectsEngine.cpp
    EffectsEngine/SemanticObjectManager.hpp
    EffectsEngine/SimpleObjectsReferencer.hpp
    EffectsEngine/PluginsAPI/InSlot.hpp
    EffectsEngine/PluginsAPI/LightVideoFrame.cpp
    EffectsEngine/PluginsAPI/OutSlot.hpp
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
    Renderer/ClipRenderer.cpp
    Renderer/GenericRenderer.cpp
    Renderer/WorkflowFileRenderer.cpp
    Renderer/WorkflowRenderer.cpp
    Tools/Pool.hpp
    Tools/QSingleton.hpp
    Tools/Singleton.hpp
    Tools/Toggleable.hpp
    Tools/VlmcDebug.cpp
    Tools/WaitCondition.hpp
    Workflow/AudioClipWorkflow.cpp 
    Workflow/ClipWorkflow.cpp
    Workflow/ImageClipWorkflow.cpp
    Workflow/MainWorkflow.cpp
    Workflow/StackedBuffer.hpp
    Workflow/TrackHandler.cpp
    Workflow/TrackWorkflow.cpp
    Workflow/VideoClipWorkflow.cpp
  )

IF (WIN32)
    LIST ( APPEND VLMC_SRCS winvlmc.cpp )
ELSE (WIN32)
    LIST( APPEND VLMC_SRCS vlmc.cpp )
ENDIF(WIN32)

SET (VLMC_HDRS
    Commands/KeyboardShortcutHelper.h
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
    Renderer/ClipRenderer.h
    Renderer/GenericRenderer.h
    Renderer/WorkflowFileRenderer.h
    Renderer/WorkflowRenderer.h
    Tools/VlmcDebug.h
    Workflow/AudioClipWorkflow.h 
    Workflow/ClipWorkflow.h
    Workflow/ImageClipWorkflow.h
    Workflow/MainWorkflow.h
    Workflow/TrackHandler.h
    Workflow/TrackWorkflow.h
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
  EffectsEngine
  EffectsEngine/PluginsAPI
  Gui
  Gui/import
  Gui/library
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

#Add GUI stuff if required
IF (NOT WITH_GUI)
    ADD_EXECUTABLE(vlmc ${GUI_TYPE} ${VLMC_SRCS} ${VLMC_MOC_SRCS} ${VLMC_RCC_SRCS})
    TARGET_LINK_LIBRARIES(vlmc
      ${QT_QTCORE_LIBRARY} 
      ${QT_QTXML_LIBRARY} 
      ${LIBVLC_LIBRARY}
      ${LIBVLCCORE_LIBRARY}
      )
    QT4_ADD_RESOURCES(VLMC_RCC_SRCS ${VLMC_RCC})
    QT4_WRAP_CPP(VLMC_MOC_SRCS ${VLMC_HDRS})

ELSE(NOT WITH_GUI)
     LIST( APPEND VLMC_SRCS
        Gui/About.cpp
        Gui/AudioSpectrumDrawer.cpp
        Gui/ClickableLabel.cpp
        Gui/ClipProperty.cpp
        Gui/DockWidgetManager.cpp
        Gui/LanguageHelper.cpp
        Gui/preview/LCDTimecode.cpp
        Gui/MainWindow.cpp
        Gui/preview/PreviewRuler.cpp
        Gui/preview/PreviewWidget.cpp
        Gui/import/TagWidget.cpp
        Gui/UndoStack.cpp
        Gui/WorkflowFileRendererDialog.cpp
        Gui/export/RendererSettings.cpp
        Gui/import/ImportController.cpp
        Gui/library/ListViewController.cpp
        Gui/library/MediaCellView.cpp
        Gui/library/MediaListView.cpp
        Gui/library/StackViewController.cpp
        Gui/library/StackViewNavController.cpp
        Gui/settings/BoolWidget.cpp
        Gui/settings/DoubleWidget.cpp
        Gui/settings/IntWidget.cpp
        Gui/settings/KeyboardShortcut.cpp
        Gui/settings/KeyboardShortcutInput.cpp
        Gui/settings/LanguageWidget.cpp
        Gui/settings/Panel.cpp
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
        Gui/widgets/TrackControls.cpp
        Gui/wizard/GeneralPage.cpp
        Gui/wizard/OpenPage.cpp
        Gui/wizard/ProjectWizard.cpp
        Gui/wizard/VideoPage.cpp
        Gui/wizard/WelcomePage.cpp
        )

    LIST( APPEND VLMC_HDRS
        Gui/About.h
        Gui/ClickableLabel.h
        Gui/ClipProperty.h
        Gui/DockWidgetManager.h
        Gui/export/RendererSettings.h
        Gui/import/ImportController.h
        Gui/LanguageHelper.h
        Gui/preview/LCDTimecode.h
        Gui/library/ListViewController.h
        Gui/library/MediaCellView.h
        Gui/library/MediaListView.h
        Gui/library/StackViewController.h
        Gui/library/StackViewNavController.h
        Gui/library/ViewController.h
        Gui/MainWindow.h
        Gui/preview/PreviewRuler.h
        Gui/preview/PreviewWidget.h
        Gui/settings/KeyboardShortcutInput.h
        Gui/settings/Panel.h
        Gui/settings/PreferenceWidget.h
        Gui/settings/Settings.h
        Gui/import/TagWidget.h
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
        Gui/import/ui/ImportController.ui
        Gui/library/StackViewNavController.ui
        Gui/library/ui/MediaCellView.ui
        Gui/library/ui/StackViewNavController.ui
        Gui/ui/About.ui
        Gui/ui/ClipProperty.ui
        Gui/ui/MainWindow.ui
        Gui/preview/ui/PreviewWidget.ui
        Gui/import/ui/TagWidget.ui
        Gui/ui/Timeline.ui
        Gui/ui/transcode.ui
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

    IF(WIN32)
        SET( GUI_TYPE WIN32 )
    ENDIF( WIN32 )

    ADD_EXECUTABLE(vlmc ${GUI_TYPE} ${VLMC_SRCS} ${VLMC_MOC_SRCS} ${VLMC_UIS_H} ${VLMC_RCC_SRCS})
    TARGET_LINK_LIBRARIES(vlmc
      ${QT_QTCORE_LIBRARY} 
      ${QT_QTGUI_LIBRARY} 
      ${QT_QTXML_LIBRARY} 
      ${QT_QTSVG_LIBRARY} 
      ${QT_QTNETWORK_LIBRARY}
      ${LIBVLC_LIBRARY}
      ${LIBVLCCORE_LIBRARY}
      )

ENDIF(NOT WITH_GUI)

ADD_DEPENDENCIES(vlmc translations)

INSTALL(TARGETS vlmc RUNTIME DESTINATION ${VLMC_BIN_DIR})

ADD_CUSTOM_COMMAND(
    OUTPUT ${CMAKE_SOURCE_DIR}/bin/vlmc
    COMMAND ${CMAKE_COMMAND} copy ${CMAKE_CURRENT_SOURCE_DIR}/vlmc ${CMAKE_SOURCE_DIR}/bin/vlmc
    DEPENDS ${CMAKE_BINARY_DIR}/src/vlmc
)

IF(UNIX)
    IF (WITH_PROFILING)
        ADD_DEFINITIONS( -pg )
        SET_TARGET_PROPERTIES( vlmc PROPERTIES LINK_FLAGS -pg )
    ENDIF(WITH_PROFILING)
ENDIF(UNIX)

