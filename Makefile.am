ACLOCAL_AMFLAGS = -I m4

bin_PROGRAMS = vlmc

SUFFIXES = .ui .h .moc.cpp

vlmc_SOURCES = \
	src/Commands/Commands.cpp \
	src/Backend/MLT/MLTBackend.cpp \
	src/Backend/MLT/MLTOutput.cpp \
	src/Backend/MLT/MLTInput.cpp \
	src/Backend/MLT/MLTTrack.cpp \
	src/Backend/MLT/MLTService.cpp \
	src/Backend/MLT/MLTProfile.cpp \
	src/Backend/MLT/MLTFilter.cpp \
	src/Backend/MLT/MLTTransition.cpp \
	src/Backend/MLT/MLTMultiTrack.cpp \
	src/EffectsEngine/EffectHelper.cpp \
	src/Library/Library.cpp \
	src/Library/MediaContainer.cpp \
	src/Main/Core.cpp \
	src/Main/main.cpp \
	src/Media/Clip.cpp \
	src/Media/Media.cpp \
	src/Project/Project.cpp \
	src/Project/Workspace.cpp \
	src/Project/WorkspaceWorker.cpp \
	src/Project/RecentProjects.cpp \
	src/Renderer/AbstractRenderer.cpp \
	src/Services/UploaderIODevice.cpp \
	src/Settings/Settings.cpp \
	src/Settings/SettingValue.cpp \
	src/Tools/ErrorHandler.cpp \
	src/Tools/RendererEventWatcher.cpp \
	src/Tools/OutputEventWatcher.cpp \
	src/Tools/VlmcLogger.cpp \
	src/Workflow/Helper.cpp \
	src/Workflow/MainWorkflow.cpp \
	src/Workflow/SequenceWorkflow.cpp \
	src/Workflow/ThumbnailWorker.cpp \
	$(NULL)

vlmc_SOURCES += \
	src/Project/Workspace.h \
	src/Project/WorkspaceWorker.h \
	src/Project/Project.h \
	src/Project/RecentProjects.h \
	src/Commands/Commands.h \
	src/Commands/AbstractUndoStack.h \
	src/Commands/KeyboardShortcutHelper.h \
	src/Tools/RendererEventWatcher.h \
	src/Tools/VlmcDebug.h \
	src/Tools/ErrorHandler.h \
	src/Tools/BacktraceGenerator.h \
	src/Tools/mdate.h \
	src/Tools/VlmcLogger.h \
	src/Tools/OutputEventWatcher.h \
	src/Tools/Singleton.hpp \
	src/Renderer/ClipRenderer.h \
	src/Renderer/ConsoleRenderer.h \
	src/Renderer/AbstractRenderer.h \
	src/Services/UploaderIODevice.h \
	src/Services/AbstractSharingService.h \
	src/Services/YouTube/YouTubeUploader.h \
	src/Services/YouTube/YouTubeCommon.h \
	src/Services/YouTube/YouTubeAuthenticator.h \
	src/Services/YouTube/YouTubeService.h \
	src/Services/YouTube/YouTubeFeedParser.h \
	src/EffectsEngine/EffectHelper.h \
	src/Media/Transcoder.h \
	src/Media/Media.h \
	src/Media/Clip.h \
	src/Settings/Settings.h \
	src/Settings/SettingValue.h \
	src/vlmc.h \
	src/Backend/ITrack.h \
	src/Backend/ITransition.h \
	src/Backend/IFilter.h \
	src/Backend/IInput.h \
	src/Backend/IOutput.h \
	src/Backend/MLT/MLTTransition.h \
	src/Backend/MLT/MLTFilter.h \
	src/Backend/MLT/MLTProfile.h \
	src/Backend/MLT/MLTTrack.h \
	src/Backend/MLT/MLTBackend.h \
	src/Backend/MLT/MLTService.h \
	src/Backend/MLT/MLTInput.h \
	src/Backend/MLT/MLTMultiTrack.h \
	src/Backend/MLT/MLTOutput.h \
	src/Backend/IBackend.h \
	src/Backend/IProfile.h \
	src/Backend/IMultiTrack.h \
	src/Main/Core.h \
	src/Library/Library.h \
	src/Library/MediaContainer.h \
	src/Workflow/Helper.h \
	src/Workflow/Types.h \
	src/Workflow/MainWorkflow.h \
	$(NULL)

nodist_vlmc_SOURCES = \
	src/Media/Clip.moc.cpp \
	src/Workflow/SequenceWorkflow.moc.cpp \
	src/Workflow/ThumbnailWorker.moc.cpp \
	src/Services/YouTube/YouTubeService.moc.cpp \
	src/EffectsEngine/EffectHelper.moc.cpp \
	src/Workflow/Helper.moc.cpp \
	src/Tools/RendererEventWatcher.moc.cpp \
	src/Project/Workspace.moc.cpp \
	src/Services/YouTube/YouTubeUploader.moc.cpp \
	src/Tools/VlmcLogger.moc.cpp \
	src/Settings/Settings.moc.cpp \
	src/Media/Media.moc.cpp \
	src/Renderer/AbstractRenderer.moc.cpp \
	src/Project/WorkspaceWorker.moc.cpp \
	src/Commands/KeyboardShortcutHelper.moc.cpp \
	src/Services/AbstractSharingService.moc.cpp \
	src/Workflow/MainWorkflow.moc.cpp \
	src/Project/RecentProjects.moc.cpp \
	src/Library/MediaContainer.moc.cpp \
	src/Commands/Commands.moc.cpp \
	src/Renderer/ClipRenderer.moc.cpp \
	src/Project/Project.moc.cpp \
	src/Services/YouTube/YouTubeAuthenticator.moc.cpp \
	src/Settings/SettingValue.moc.cpp \
	src/Tools/OutputEventWatcher.moc.cpp \
	src/Services/UploaderIODevice.moc.cpp \
	src/Library/Library.moc.cpp \
	$(NULL)

vlmc_RC = \
	$(top_srcdir)/resources.qrc \
	$(top_srcdir)/src/Gui/timeline/resources-timeline.qrc \
	$(NULL)

EXTRA_DIST = $(vlmc_RC)

if HAVE_WIN32
vlmc_SOURCES += src/Main/winvlmc.cpp
vlmc_RC += $(top_srcdir)/resources/styles.qrc
else
vlmc_SOURCES += src/Main/vlmc.cpp
endif

vlmc_CPPFLAGS = \
	$(AM_CPPFLAGS) \
	$(QT_CFLAGS) \
	$(MLT_CFLAGS) \
	$(LIBVLCPP_CFLAGS) \
	$(MEDIALIBRARY_CFLAGS) \
	-I$(top_srcdir)/src \
	$(NULL)

vlmc_CXXFLAGS = \
	$(AM_CXXFLAGS) \
	$(NULL)

vlmc_LDADD = \
	$(QT_LIBS) \
	$(MLT_LIBS) \
	$(MLTPP_LIBS) \
	$(MEDIALIBRARY_LIBS) \
	$(NULL)

vlmc_LDFLAGS=

if HAVE_GUI
vlmc_SOURCES += \
	src/Commands/KeyboardShortcutHelper.cpp \
	src/Renderer/ClipRenderer.cpp \
	src/Services/YouTube/YouTubeAuthenticator.cpp \
	src/Services/YouTube/YouTubeFeedParser.cpp \
	src/Services/YouTube/YouTubeService.cpp \
	src/Services/YouTube/YouTubeUploader.cpp \
	src/Gui/About.cpp \
	src/Gui/ClipProperty.cpp \
	src/Gui/IntroDialog.cpp \
	src/Gui/LanguageHelper.cpp \
	src/Gui/MainWindow.cpp \
	src/Gui/WorkflowFileRendererDialog.cpp \
	src/Gui/effectsengine/EffectInstanceWidget.cpp \
	src/Gui/effectsengine/EffectInstanceListModel.cpp \
	src/Gui/effectsengine/EffectsListView.cpp \
	src/Gui/effectsengine/EffectStack.cpp \
	src/Gui/effectsengine/EffectWidget.cpp \
	src/Gui/export/RendererSettings.cpp \
	src/Gui/export/ShareOnInternet.cpp \
	src/Gui/import/ImportController.cpp \
	src/Gui/import/TagWidget.cpp \
	src/Gui/library/ListViewController.cpp \
	src/Gui/library/MediaCellView.cpp \
	src/Gui/library/MediaLibraryView.cpp \
	src/Gui/library/MediaListView.cpp \
	src/Gui/library/StackViewController.cpp \
	src/Gui/library/StackViewNavController.cpp \
	src/Gui/media/ClipMetadataDisplayer.cpp \
	src/Gui/preview/LCDTimecode.cpp \
	src/Gui/preview/PreviewRuler.cpp \
	src/Gui/preview/PreviewWidget.cpp \
	src/Gui/settings/BoolWidget.cpp \
	src/Gui/settings/ColorWidget.cpp \
	src/Gui/settings/DoubleWidget.cpp \
	src/Gui/settings/DoubleSliderWidget.cpp \
	src/Gui/settings/IntWidget.cpp \
	src/Gui/settings/KeyboardShortcut.cpp \
	src/Gui/settings/KeyboardShortcutInput.cpp \
	src/Gui/settings/LanguageWidget.cpp \
	src/Gui/settings/Panel.cpp \
	src/Gui/settings/PathWidget.cpp \
	src/Gui/settings/PreferenceWidget.cpp \
	src/Gui/settings/SettingsDialog.cpp \
	src/Gui/settings/StringWidget.cpp \
	src/Gui/timeline/Timeline.cpp \
	src/Gui/timeline/ThumbnailImageProvider.cpp \
	src/Gui/widgets/ExtendedLabel.cpp \
	src/Gui/widgets/FramelessButton.cpp \
	src/Gui/widgets/NotificationZone.cpp \
	src/Gui/widgets/SearchLineEdit.cpp \
	src/Gui/wizard/GeneralPage.cpp \
	src/Gui/wizard/OpenPage.cpp \
	src/Gui/wizard/ProjectWizard.cpp \
	src/Gui/wizard/VideoPage.cpp \
	src/Gui/wizard/WelcomePage.cpp \
	src/Gui/wizard/firstlaunch/FirstLaunchWizard.cpp \
	src/Gui/wizard/firstlaunch/FirstLaunchPage.cpp \
	src/Gui/wizard/firstlaunch/WorkspaceLocation.cpp \
	src/Gui/wizard/firstlaunch/Done.cpp \
	$(NULL)

if HAVE_DARWIN
vlmc_SOURCES += src/Gui/preview/RenderWidget.mm
vlmc_LDFLAGS += -Wl,-framework,Cocoa
endif

vlmc_SOURCES += \
	src/Gui/ClipProperty.h \
	src/Gui/WorkflowFileRendererDialog.h \
	src/Gui/widgets/FramelessButton.h \
	src/Gui/widgets/ExtendedLabel.h \
	src/Gui/widgets/SearchLineEdit.h \
	src/Gui/widgets/CrashHandler.h \
	src/Gui/widgets/NotificationZone.h \
	src/Gui/MainWindow.h \
	src/Gui/wizard/ProjectWizard.h \
	src/Gui/wizard/GeneralPage.h \
	src/Gui/wizard/PageFactory.h \
	src/Gui/wizard/WelcomePage.h \
	src/Gui/wizard/VideoPage.h \
	src/Gui/wizard/firstlaunch/FirstLaunchWizard.h \
	src/Gui/wizard/firstlaunch/FirstLaunchPage.h \
	src/Gui/wizard/firstlaunch/WorkspaceLocation.h \
	src/Gui/wizard/firstlaunch/Done.h \
	src/Gui/wizard/OpenPage.h \
	src/Gui/timeline/Timeline.h \
	src/Gui/timeline/ThumbnailImageProvider.h \
	src/Gui/About.h \
	src/Gui/LanguageHelper.h \
	src/Gui/library/ListViewController.h \
	src/Gui/library/StackViewController.h \
	src/Gui/library/MediaListView.h \
	src/Gui/library/MediaLibraryView.h \
	src/Gui/library/MediaCellView.h \
	src/Gui/library/StackViewNavController.h \
	src/Gui/library/ViewController.h \
	src/Gui/media/ClipMetadataDisplayer.h \
	src/Gui/preview/RenderWidget.h \
	src/Gui/preview/PreviewRuler.h \
	src/Gui/preview/PreviewWidget.h \
	src/Gui/preview/LCDTimecode.h \
	src/Gui/settings/DoubleWidget.h \
	src/Gui/settings/KeyboardShortcut.h \
	src/Gui/settings/StringWidget.h \
	src/Gui/settings/KeyboardShortcutInput.h \
	src/Gui/settings/SettingsDialog.h \
	src/Gui/settings/DoubleSliderWidget.h \
	src/Gui/settings/ColorWidget.h \
	src/Gui/settings/IntWidget.h \
	src/Gui/settings/ISettingsCategoryWidget.h \
	src/Gui/settings/PathWidget.h \
	src/Gui/settings/BoolWidget.h \
	src/Gui/settings/LanguageWidget.h \
	src/Gui/settings/Panel.h \
	src/Gui/settings/PreferenceWidget.h \
	src/Gui/import/ImportController.h \
	src/Gui/import/TagWidget.h \
	src/Gui/effectsengine/EffectStack.h \
	src/Gui/effectsengine/EffectWidget.h \
	src/Gui/effectsengine/EffectsListView.h \
	src/Gui/effectsengine/EffectInstanceWidget.h \
	src/Gui/effectsengine/EffectInstanceListModel.h \
	src/Gui/export/RendererSettings.h \
	src/Gui/export/ShareOnInternet.h \
	src/Gui/IntroDialog.h \
	$(NULL)

nodist_vlmc_SOURCES += \
	src/Gui/wizard/WelcomePage.moc.cpp \
	src/Gui/library/ListViewController.moc.cpp \
	src/Gui/settings/SettingsDialog.moc.cpp \
	src/Gui/export/ShareOnInternet.moc.cpp \
	src/Gui/widgets/FramelessButton.moc.cpp \
	src/Gui/wizard/OpenPage.moc.cpp \
	src/Gui/settings/ColorWidget.moc.cpp \
	src/Gui/import/ImportController.moc.cpp \
	src/Gui/library/StackViewController.moc.cpp \
	src/Gui/library/MediaLibraryView.moc.cpp \
	src/Gui/wizard/firstlaunch/FirstLaunchPage.moc.cpp \
	src/Gui/settings/Panel.moc.cpp \
	src/Gui/preview/RenderWidget.moc.cpp \
	src/Gui/settings/BoolWidget.moc.cpp \
	src/Gui/WorkflowFileRendererDialog.moc.cpp \
	src/Gui/library/MediaListView.moc.cpp \
	src/Gui/wizard/firstlaunch/Done.moc.cpp \
	src/Gui/wizard/VideoPage.moc.cpp \
	src/Gui/settings/ISettingsCategoryWidget.moc.cpp \
	src/Gui/wizard/GeneralPage.moc.cpp \
	src/Gui/library/MediaCellView.moc.cpp \
	src/Gui/MainWindow.moc.cpp \
	src/Gui/settings/KeyboardShortcut.moc.cpp \
	src/Gui/preview/PreviewWidget.moc.cpp \
	src/Gui/preview/PreviewRuler.moc.cpp \
	src/Gui/settings/PreferenceWidget.moc.cpp \
	src/Gui/timeline/Timeline.moc.cpp \
	src/Gui/timeline/ThumbnailImageProvider.moc.cpp \
	src/Gui/settings/LanguageWidget.moc.cpp \
	src/Gui/import/TagWidget.moc.cpp \
	src/Gui/widgets/NotificationZone.moc.cpp \
	src/Gui/settings/DoubleSliderWidget.moc.cpp \
	src/Gui/widgets/ExtendedLabel.moc.cpp \
	src/Gui/About.moc.cpp \
	src/Gui/LanguageHelper.moc.cpp \
	src/Gui/ClipProperty.moc.cpp \
	src/Gui/library/ViewController.moc.cpp \
	src/Gui/export/RendererSettings.moc.cpp \
	src/Gui/media/ClipMetadataDisplayer.moc.cpp \
	src/Gui/effectsengine/EffectWidget.moc.cpp \
	src/Gui/effectsengine/EffectInstanceWidget.moc.cpp \
	src/Gui/preview/LCDTimecode.moc.cpp \
	src/Gui/effectsengine/EffectStack.moc.cpp \
	src/Gui/wizard/firstlaunch/WorkspaceLocation.moc.cpp \
	src/Gui/settings/DoubleWidget.moc.cpp \
	src/Gui/wizard/ProjectWizard.moc.cpp \
	src/Gui/IntroDialog.moc.cpp \
	src/Gui/settings/KeyboardShortcutInput.moc.cpp \
	src/Gui/widgets/SearchLineEdit.moc.cpp \
	src/Gui/library/StackViewNavController.moc.cpp \
	src/Gui/settings/PathWidget.moc.cpp \
	src/Gui/settings/IntWidget.moc.cpp \
	src/Gui/settings/StringWidget.moc.cpp \
	src/Gui/effectsengine/EffectsListView.moc.cpp \
	$(NULL)

vlmc_UI = \
	src/Gui/ui/EffectInstanceWidget.ui \
	src/Gui/ui/EffectStack.ui \
	src/Gui/ui/EffectWidget.ui \
	src/Gui/ui/RendererSettings.ui \
	src/Gui/ui/ShareOnInternet.ui \
	src/Gui/ui/ImportController.ui \
	src/Gui/ui/TagWidget.ui \
	src/Gui/ui/MediaCellView.ui \
	src/Gui/ui/MediaLibraryView.ui \
	src/Gui/ui/StackViewNavController.ui \
	src/Gui/ui/ClipMetadataDisplayer.ui \
	src/Gui/ui/PreviewWidget.ui \
	src/Gui/ui/About.ui \
	src/Gui/ui/ClipProperty.ui \
	src/Gui/ui/IntroDialog.ui \
	src/Gui/ui/MainWindow.ui \
	src/Gui/ui/Timeline.ui \
	src/Gui/ui/WorkflowFileRendererDialog.ui \
	src/Gui/ui/NotificationZone.ui \
	src/Gui/ui/GeneralPage.ui \
	src/Gui/ui/OpenPage.ui \
	src/Gui/ui/VideoPage.ui \
	src/Gui/ui/WelcomePage.ui \
	src/Gui/ui/FirstLaunchPage.ui \
	src/Gui/ui/WorkspaceLocation.ui \
	src/Gui/ui/Done.ui \
	$(NULL)

if HAVE_CRASHHANDLER
vlmc_UI += src/Gui/ui/CrashHandler.ui
vlmc_SOURCES += src/Gui/widgets/CrashHandler.cpp
if HAVE_WIN32
vlmc_SOURCES += src/Tools/Win32BacktraceGenerator.cpp
else
vlmc_SOURCES += src/Tools/UnixBacktraceGenerator.cpp
endif
endif

EXTRA_DIST += $(vlmc_UI)
vlmc_CPPFLAGS += -Isrc/Gui/ -I$(top_srcdir)/src/Gui

nodist_vlmc_SOURCES += $(vlmc_UI:.ui=.h)

endif

resources.cpp:
	$(RCC) -o $@ $(vlmc_RC)

nodist_vlmc_SOURCES += resources.cpp

moc_verbose = $(moc_verbose_$(V))
moc_verbose_ = $(moc_verbose__$(AM_DEFAULT_VERBOSITY))
moc_verbose_0 = @echo "  MOC   " $@;
moc_verbose__0 = $(moc_verbose_0)

.h.moc.cpp:
	$(moc_verbose)$(MOC) -o $@ $<

uic_verbose = $(uic_verbose_$(V))
uic_verbose_ = $(uic_verbose__$(AM_DEFAULT_VERBOSITY))
uic_verbose_0 = @echo "  UIC   " $@;
uic_verbose__0 = $(uic_verbose_0)

%.h: %.ui
	$(AM_V_at)mkdir -p $(@D)
	$(uic_verbose)$(UIC) -p $< -o $@

BUILT_SOURCES = $(nodist_vlmc_SOURCES)
CLEANFILES = $(BUILT_SOURCES)

