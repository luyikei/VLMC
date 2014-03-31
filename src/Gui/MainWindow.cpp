/*****************************************************************************
 * MainWindow.cpp: VLMC MainWindow
 *****************************************************************************
 * Copyright (C) 2008-2014 VideoLAN
 *
 * Authors: Ludovic Fauvet <etix@l0cal.com>
 *          Hugo Beauzée-Luyssen <hugo@beauzee.fr>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston MA 02110-1301, USA.
 *****************************************************************************/

#include "config.h"

#include <QSizePolicy>
#include <QDockWidget>
#include <QFileDialog>
#include <QSlider>
#include <QMessageBox>
#include <QDesktopServices>
#include <QUndoView>
#include <QUndoStack>
#include <QUrl>
#include <QNetworkProxy>
#include "Main/Core.h"
#include "Project/Project.h"
#include "Library/Library.h"
#include "Tools/VlmcDebug.h"
#include "Tools/VlmcLogger.h"
#include "EffectsEngine/EffectsEngine.h"
#include "Backend/IBackend.h"
#include "Workflow/MainWorkflow.h"
#include "Renderer/WorkflowFileRenderer.h"
#include "Renderer/WorkflowRenderer.h"
#include "Renderer/ClipRenderer.h"

/* Gui */
#include "MainWindow.h"
#include "About.h"
#include "WorkflowFileRendererDialog.h"
#include "export/RendererSettings.h"
#include "export/ShareOnInternet.h"
#include "settings/SettingsDialog.h"

/* Widgets */
#include "DockWidgetManager.h"
#include "effectsengine/EffectsListView.h"
#include "import/ImportController.h"
#include "library/MediaLibrary.h"
#include "widgets/NotificationZone.h"
#include "preview/PreviewWidget.h"
#include "timeline/Timeline.h"
#include "timeline/TracksView.h"

/* Settings / Preferences */
#include "project/GuiProjectManager.h"
#include "Project/RecentProjects.h"
#include "wizard/ProjectWizard.h"
#include "Settings/Settings.h"
#include "LanguageHelper.h"
#include "Commands/KeyboardShortcutHelper.h"

MainWindow::MainWindow( Backend::IBackend* backend, QWidget *parent )
    : QMainWindow( parent )
    , m_backend( backend )
    , m_fileRenderer( NULL )
    , m_projectPreferences( NULL )
    , m_wizard( NULL )
{
    m_ui.setupUi( this );

    Core::getInstance()->logger()->setup();
    //Preferences
    initVlmcPreferences();
    //All preferences have been created: restore them:
    Core::getInstance()->settings()->load();

    // GUI
    DockWidgetManager::getInstance( this )->setMainWindow( this );
    createGlobalPreferences();
    initializeDockWidgets();
    initToolbar();
    createStatusBar();
    checkFolders();
    loadGlobalProxySettings();
#ifdef WITH_CRASHBUTTON
    setupCrashTester();
#endif

    // Zoom
    connect( m_zoomSlider, SIGNAL( valueChanged( int ) ),
             m_timeline, SLOT( changeZoom( int ) ) );
    connect( m_timeline->tracksView(), SIGNAL( zoomIn() ),
             this, SLOT( zoomIn() ) );
    connect( m_timeline->tracksView(), SIGNAL( zoomOut() ),
             this, SLOT( zoomOut() ) );
    connect( this, SIGNAL( toolChanged( ToolButtons ) ),
             m_timeline, SLOT( setTool( ToolButtons ) ) );

    //Connecting Library stuff:
    const ClipRenderer* clipRenderer = qobject_cast<const ClipRenderer*>( m_clipPreview->getGenericRenderer() );
    Q_ASSERT( clipRenderer != NULL );
    connect( m_mediaLibrary, SIGNAL( clipSelected( Clip* ) ),
             clipRenderer, SLOT( setClip( Clip* ) ) );
    connect( m_mediaLibrary, SIGNAL( importRequired() ),
             this, SLOT( on_actionImport_triggered() ) );


#ifdef WITH_CRASHHANDLER
    if ( restoreSession() == true )
        return ;
#endif
    // Restore the geometry
    restoreGeometry( VLMC_GET_BYTEARRAY( "private/MainWindowGeometry" ) );
    // Restore the layout
    restoreState( VLMC_GET_BYTEARRAY( "private/MainWindowState" ) );
}

MainWindow::~MainWindow()
{
    if ( m_fileRenderer )
        delete m_fileRenderer;
    delete m_importController;
}

void
MainWindow::showWizard()
{
    if ( m_wizard == NULL )
    {
        m_wizard = new ProjectWizard( this );
        m_wizard->setModal( true );
    }
    m_wizard->show();
}

void
MainWindow::changeEvent( QEvent *e )
{
    switch ( e->type() )
    {
    case QEvent::LanguageChange:
        m_ui.retranslateUi( this );
        DockWidgetManager::getInstance()->retranslateUi();
        break;
    default:
        break;
    }
}

//use this helper when the shortcut is binded to a menu action
#define CREATE_MENU_SHORTCUT( key, defaultValue, name, desc, actionInstance  )      \
        VLMC_CREATE_PREFERENCE_KEYBOARD( key, defaultValue, name, desc );           \
        new KeyboardShortcutHelper( key, m_ui.actionInstance, this );

void
MainWindow::initVlmcPreferences()
{
    //Setup VLMC Keyboard Preference...
    VLMC_CREATE_PREFERENCE_KEYBOARD( "keyboard/mediapreview", "Ctrl+Return",
                                     QT_TRANSLATE_NOOP( "PreferenceWidget", "Media preview" ),
                                     QT_TRANSLATE_NOOP( "PreferenceWidget", "Preview the selected media, or pause the current preview" ) );

    VLMC_CREATE_PREFERENCE_KEYBOARD( "keyboard/renderpreview", "Space",
                                     QT_TRANSLATE_NOOP( "PreferenceWidget", "Render preview" ),
                                     QT_TRANSLATE_NOOP( "PreferenceWidget", "Preview the project, or pause the current preview" ) );

    //A bit nasty, but we better use what Qt's providing as default shortcut
    CREATE_MENU_SHORTCUT( "keyboard/undo",
                          QKeySequence( QKeySequence::Undo ).toString().toLocal8Bit(),
                          QT_TRANSLATE_NOOP( "PreferenceWidget", "Undo" ),
                          QT_TRANSLATE_NOOP( "PreferenceWidget", "Undo the last action" ), actionUndo );

    CREATE_MENU_SHORTCUT( "keyboard/redo",
                          QKeySequence( QKeySequence::Redo ).toString().toLocal8Bit(),
                          QT_TRANSLATE_NOOP( "PreferenceWidget", "Redo" ),
                          QT_TRANSLATE_NOOP( "PreferenceWidget", "Redo the last action" ), actionRedo );

    CREATE_MENU_SHORTCUT( "keyboard/help",
                          QKeySequence( QKeySequence::HelpContents ).toString().toLocal8Bit(),
                          QT_TRANSLATE_NOOP( "PreferenceWidget", "Help" ),
                          QT_TRANSLATE_NOOP( "PreferenceWidget", "Toggle the help page" ), actionHelp );

    CREATE_MENU_SHORTCUT( "keyboard/quit", "Ctrl+Q",
                          QT_TRANSLATE_NOOP( "PreferenceWidget", "Quit" ),
                          QT_TRANSLATE_NOOP( "PreferenceWidget", "Quit VLMC" ), actionQuit );

    CREATE_MENU_SHORTCUT( "keyboard/preferences", "Alt+P",
                          QT_TRANSLATE_NOOP( "PreferenceWidget", "Preferences" ),
                          QT_TRANSLATE_NOOP( "PreferenceWidget", "Open VLMC preferences" ), actionPreferences );

    CREATE_MENU_SHORTCUT( "keyboard/projectpreferences", "Ctrl+P",
                          QT_TRANSLATE_NOOP( "PreferenceWidget", "Project preferences" ),
                          QT_TRANSLATE_NOOP( "PreferenceWidget", "Open the project preferences"), actionProject_Preferences );

    CREATE_MENU_SHORTCUT( "keyboard/fullscreen", "F",
                          QT_TRANSLATE_NOOP( "PreferenceWidget", "Fullscreen" ),
                          QT_TRANSLATE_NOOP( "PreferenceWidget", "Switch to fullscreen mode" ), actionFullscreen );

    CREATE_MENU_SHORTCUT( "keyboard/newproject",
                          QKeySequence( QKeySequence::New ).toString().toLocal8Bit(),
                          QT_TRANSLATE_NOOP( "PreferenceWidget", "New project" ),
                          QT_TRANSLATE_NOOP( "PreferenceWidget", "Open the new project wizard" ), actionNew_Project );

    CREATE_MENU_SHORTCUT( "keyboard/openproject",
                          QKeySequence( QKeySequence::Open ).toString().toLocal8Bit(),
                          QT_TRANSLATE_NOOP( "PreferenceWidget", "Open a project" ),
                          QT_TRANSLATE_NOOP( "PreferenceWidget", "Open an existing project" ), actionLoad_Project );

    CREATE_MENU_SHORTCUT( "keyboard/save",
                          QKeySequence( QKeySequence::Save ).toString().toLocal8Bit(),
                          QT_TRANSLATE_NOOP( "PreferenceWidget", "Save" ),
                          QT_TRANSLATE_NOOP( "PreferenceWidget", "Save the current project" ), actionSave );

    CREATE_MENU_SHORTCUT( "keyboard/saveas", "Ctrl+Shift+S",
                          QT_TRANSLATE_NOOP( "PreferenceWidget", "Save as" ),
                          QT_TRANSLATE_NOOP( "PreferenceWidget", "Save the current project to a new file" ), actionSave_As );

    CREATE_MENU_SHORTCUT( "keyboard/closeproject",
                          QKeySequence( QKeySequence::Close ).toString().toLocal8Bit(),
                          QT_TRANSLATE_NOOP( "PreferenceWidget", "Close the project" ),
                          QT_TRANSLATE_NOOP( "PreferenceWidget", "Close the current project" ), actionClose_Project );

    CREATE_MENU_SHORTCUT( "keyboard/importmedia", "Ctrl+I",
                          QT_TRANSLATE_NOOP( "PreferenceWidget", "Import media" ),
                          QT_TRANSLATE_NOOP( "PreferenceWidget", "Open the import window" ), actionImport );

    CREATE_MENU_SHORTCUT( "keyboard/renderproject", "Ctrl+R",
                          QT_TRANSLATE_NOOP( "PreferenceWidget", "Render the project" ),
                          QT_TRANSLATE_NOOP( "PreferenceWidget", "Render the project to a file" ), actionRender );

    CREATE_MENU_SHORTCUT( "keyboard/defaultmode", "n",
                          QT_TRANSLATE_NOOP( "PreferenceWidget", "Selection mode" ),
                          QT_TRANSLATE_NOOP( "PreferenceWidget", "Select the selection tool in the timeline" ), actionSelection_mode );

    CREATE_MENU_SHORTCUT( "keyboard/cutmode", "x",
                          QT_TRANSLATE_NOOP( "PreferenceWidget", "Cut mode" ),
                          QT_TRANSLATE_NOOP( "PreferenceWidget", "Select the cut/razor tool in the timeline" ), actionCut_mode );

    //Setup VLMC Lang. Preferences...
    VLMC_CREATE_PREFERENCE_LANGUAGE( "vlmc/VLMCLang", "default",
                                     QT_TRANSLATE_NOOP( "PreferenceWidget", "Language" ),
                                     QT_TRANSLATE_NOOP( "PreferenceWidget", "The VLMC's UI language" ) );

    Core::getInstance()->settings()->watchValue( "vlmc/VLMCLang",
                                                LanguageHelper::getInstance(),
                                                SLOT( languageChanged( const QVariant& ) ) );
    //Setup VLMC General Preferences...
    VLMC_CREATE_PREFERENCE_BOOL( "vlmc/ConfirmDeletion", true,
                                 QT_TRANSLATE_NOOP( "PreferenceWidget", "Confirm clip deletion"),
                                 QT_TRANSLATE_NOOP( "PreferenceWidget", "Ask for confirmation before deleting a clip from the timeline" ) );

    VLMC_CREATE_PREFERENCE_PATH( "vlmc/DefaultProjectLocation", QDir::homePath(),
                                    QT_TRANSLATE_NOOP( "PreferenceWidget", "Project default location" ),
                                    QT_TRANSLATE_NOOP( "PreferenceWidget", "The default location where to store projects folders" ) );

    VLMC_CREATE_PREFERENCE_PATH( "vlmc/TempFolderLocation", QDir::tempPath() + "/VLMC/",
                                    QT_TRANSLATE_NOOP( "PreferenceWidget", "Temporary folder" ),
                                    QT_TRANSLATE_NOOP( "PreferenceWidget", "The temporary folder used by VLMC to process videos." ) );

    //Setup VLMC Youtube Preference...
    VLMC_CREATE_PREFERENCE_STRING( "youtube/DeveloperKey", "",
                                     QT_TRANSLATE_NOOP( "PreferenceWidget", "Youtube Developer Key" ),
                                     QT_TRANSLATE_NOOP( "PreferenceWidget", "VLMC's Youtube Developer Key" ) );

    VLMC_CREATE_PREFERENCE_STRING( "youtube/Username", "username",
                                     QT_TRANSLATE_NOOP( "PreferenceWidget", "Youtube Username" ),
                                     QT_TRANSLATE_NOOP( "PreferenceWidget", "Valid YouTube username" ) );

    VLMC_CREATE_PREFERENCE_PASSWORD( "youtube/Password", "",
                                     QT_TRANSLATE_NOOP( "PreferenceWidget", "Youtube Password" ),
                                     QT_TRANSLATE_NOOP( "PreferenceWidget", "Leave this field empty, password will be stored in unencrypted form." ) );

    //Setup VLMC Proxy Settings
    VLMC_CREATE_PREFERENCE_BOOL( "network/UseProxy", false,
                                     QT_TRANSLATE_NOOP( "PreferenceWidget", "Enable Proxy for VLMC"),
                                     QT_TRANSLATE_NOOP( "PreferenceWidget", "Enables Global Network Proxy for VLMC." ) );

    VLMC_CREATE_PREFERENCE_STRING( "network/ProxyHost", "",
                                     QT_TRANSLATE_NOOP( "PreferenceWidget", "Proxy Hostname" ),
                                     QT_TRANSLATE_NOOP( "PreferenceWidget", "Set Proxy Hostname." ) );

    VLMC_CREATE_PREFERENCE_STRING( "network/ProxyPort", "",
                                     QT_TRANSLATE_NOOP( "PreferenceWidget", "Proxy Port" ),
                                     QT_TRANSLATE_NOOP( "PreferenceWidget", "Set Proxy Port." ) );

    VLMC_CREATE_PREFERENCE_STRING( "network/ProxyUsername", "",
                                     QT_TRANSLATE_NOOP( "PreferenceWidget", "Proxy Username" ),
                                     QT_TRANSLATE_NOOP( "PreferenceWidget", "Set Proxy Username, if any." ) );

    VLMC_CREATE_PREFERENCE_PASSWORD( "network/ProxyPassword", "",
                                     QT_TRANSLATE_NOOP( "PreferenceWidget", "Proxy Password" ),
                                     QT_TRANSLATE_NOOP( "PreferenceWidget", "Set Proxy Password, if any." ) );

    // Setup private variables
    VLMC_CREATE_PRIVATE_PREFERENCE_BOOL( "private/CleanQuit", true );
    VLMC_CREATE_PRIVATE_PREFERENCE_STRING( "private/EmergencyBackup", "" );
    VLMC_CREATE_PRIVATE_PREFERENCE_STRING( "private/ImportPreviouslySelectPath", QDir::homePath() );
    VLMC_CREATE_PRIVATE_PREFERENCE_BYTEARRAY( "private/MainWindowGeometry", "" );
    VLMC_CREATE_PRIVATE_PREFERENCE_BYTEARRAY( "private/MainWindowState", "" );
}

#undef CREATE_MENU_SHORTCUT

void
MainWindow::on_actionSave_triggered()
{
    Project::getInstance()->save();
}

void
MainWindow::on_actionSave_As_triggered()
{
    Project::getInstance()->saveAs();
}

void
MainWindow::on_actionLoad_Project_triggered()
{
    // FIXME: We probably could use a default folder.
    QString fileName = QFileDialog::getOpenFileName( NULL, tr( "Please choose a project file" ),
                                    "", tr( "VLMC project file(*.vlmc)" ) );
    if ( fileName.isEmpty() == true )
        return ;
    Project::load( fileName );
}

void
MainWindow::createNotificationZone()
{
    QWidget *notifSpacer = new QWidget( this );
    notifSpacer->setFixedWidth( 75 );
    m_ui.statusbar->addPermanentWidget( notifSpacer );

    m_ui.statusbar->addPermanentWidget( NotificationZone::getInstance() );
}

void
MainWindow::createStatusBar()
{
    //Notifications:
    createNotificationZone();

    // Spacer
    QWidget* spacer = new QWidget( this );
    spacer->setFixedWidth( 20 );
    m_ui.statusbar->addPermanentWidget( spacer );

    // Zoom Out
    QToolButton* zoomOutButton = new QToolButton( this );
    zoomOutButton->setIcon( QIcon( ":/images/zoomout" ) );
    zoomOutButton->setStatusTip( tr( "Zoom out" ) );
    m_ui.statusbar->addPermanentWidget( zoomOutButton );
    connect( zoomOutButton, SIGNAL( clicked() ),
             this, SLOT( zoomOut() ) );

    // Zoom slider
    m_zoomSlider = new QSlider( this );
    m_zoomSlider->setOrientation( Qt::Horizontal );
    m_zoomSlider->setTickInterval( 1 );
    m_zoomSlider->setSingleStep( 1 );
    m_zoomSlider->setPageStep( 1 );
    m_zoomSlider->setMinimum( 0 );
    m_zoomSlider->setMaximum( 13 );
    m_zoomSlider->setValue( 10 );
    m_zoomSlider->setFixedWidth( 80 );
    m_zoomSlider->setInvertedAppearance( true );
    m_ui.statusbar->addPermanentWidget( m_zoomSlider );

    // Zoom IN
    QToolButton* zoomInButton = new QToolButton( this );
    zoomInButton->setIcon( QIcon( ":/images/zoomin" ) );
    zoomInButton->setStatusTip( tr( "Zoom in" ) );
    m_ui.statusbar->addPermanentWidget( zoomInButton );
    connect( zoomInButton, SIGNAL( clicked() ),
             this, SLOT( zoomIn() ) );
}

void
MainWindow::initializeDockWidgets()
{
    m_renderer = new WorkflowRenderer( m_backend );
    m_renderer->initializeRenderer();
    m_timeline = new Timeline( m_renderer, this );
    m_timeline->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding );
    m_timeline->show();
    setCentralWidget( m_timeline );

    m_importController = new ImportController();

    setupLibrary();
    setupEffectsList();
    setupClipPreview();
    setupProjectPreview();
    setupUndoRedoWidget();
}

void
MainWindow::setupUndoRedoWidget()
{
    QDockWidget     *dockedWidget = DockWidgetManager::getInstance()->createDockedWidget(
                                      QT_TRANSLATE_NOOP( "DockWidgetManager", "History" ),
                                      Qt::AllDockWidgetAreas,
                                      QDockWidget::AllDockWidgetFeatures );
    QWidget         *undoRedoWidget = new QUndoView( Project::getInstance()->undoStack(), dockedWidget );

    DockWidgetManager::getInstance()->addDockedWidget( dockedWidget, undoRedoWidget, Qt::TopDockWidgetArea );
}

void
MainWindow::setupEffectsList()
{
    QDockWidget     *dockedWidget = DockWidgetManager::getInstance()->createDockedWidget(
                                        QT_TRANSLATE_NOOP( "DockWidgetManager", "Effects List" ),
                                        Qt::AllDockWidgetAreas,
                                        QDockWidget::AllDockWidgetFeatures );
    m_effectsList = new EffectsListView( dockedWidget );
    m_effectsList->setType( Effect::Filter );
    Core::getInstance()->effectsEngine()->loadEffects();
    DockWidgetManager::getInstance()->addDockedWidget( dockedWidget, m_effectsList, Qt::TopDockWidgetArea );
}

void
MainWindow::setupLibrary()
{
    QDockWidget     *dockedLibrary = DockWidgetManager::getInstance()->createDockedWidget(
                                                    QT_TRANSLATE_NOOP( "DockWidgetManager", "Media Library" ),
                                                    Qt::AllDockWidgetAreas,
                                                    QDockWidget::AllDockWidgetFeatures );
    m_mediaLibrary = new MediaLibrary( dockedLibrary );
    DockWidgetManager::getInstance()->addDockedWidget( dockedLibrary, m_mediaLibrary, Qt::TopDockWidgetArea );
}

void
MainWindow::setupClipPreview()
{
    QDockWidget *dockedWidget = DockWidgetManager::getInstance()->createDockedWidget(
                                                QT_TRANSLATE_NOOP( "DockWidgetManager", "Clip Preview" ),
                                                Qt::AllDockWidgetAreas,
                                                QDockWidget::AllDockWidgetFeatures );
    m_clipPreview = new PreviewWidget( dockedWidget );
    m_clipPreview->setRenderer( new ClipRenderer );

    KeyboardShortcutHelper* clipShortcut = new KeyboardShortcutHelper( "keyboard/mediapreview", this );
    connect( clipShortcut, SIGNAL( activated() ), m_clipPreview, SLOT( on_pushButtonPlay_clicked() ) );
    DockWidgetManager::getInstance()->addDockedWidget( dockedWidget, m_clipPreview, Qt::TopDockWidgetArea );
}

void
MainWindow::setupProjectPreview()
{
    QDockWidget     *dockedWidget = DockWidgetManager::getInstance()->createDockedWidget(
                                        QT_TRANSLATE_NOOP( "DockWidgetManager", "Project Preview" ),
                                        Qt::AllDockWidgetAreas,
                                        QDockWidget::AllDockWidgetFeatures );

    m_projectPreview = new PreviewWidget( dockedWidget );
    m_projectPreview->setRenderer( m_renderer );
    m_projectPreview->setClipEdition( false );
    KeyboardShortcutHelper* renderShortcut = new KeyboardShortcutHelper( "keyboard/renderpreview", this );
    connect( renderShortcut, SIGNAL( activated() ), m_projectPreview, SLOT( on_pushButtonPlay_clicked() ) );
    DockWidgetManager::getInstance()->addDockedWidget( dockedWidget, m_projectPreview, Qt::TopDockWidgetArea );
}

void
MainWindow::initToolbar()
{
    QActionGroup    *mouseActions = new QActionGroup( m_ui.toolBar );
    mouseActions->addAction( m_ui.actionSelection_mode );
    mouseActions->addAction( m_ui.actionCut_mode );
    m_ui.actionSelection_mode->setChecked( true );
    m_ui.toolBar->addActions( mouseActions->actions() );
    connect( mouseActions, SIGNAL( triggered(QAction*) ),
             this, SLOT( toolButtonClicked( QAction* ) ) );
    m_ui.menuTools->addActions( mouseActions->actions() );
}

void
MainWindow::createGlobalPreferences()
{
    m_globalPreferences = new SettingsDialog( Core::getInstance()->settings(), tr( "VLMC Preferences" ), this );
    m_globalPreferences->addCategory( "vlmc", QT_TRANSLATE_NOOP( "Settings", "General" ), QIcon( ":/images/vlmc" ) );
    m_globalPreferences->addCategory( "keyboard", QT_TRANSLATE_NOOP( "Settings", "Keyboard" ), QIcon( ":/images/keyboard" ) );
    m_globalPreferences->addCategory( "youtube", QT_TRANSLATE_NOOP( "Settings", "YouTube" ), QIcon( ":/images/youtube" ) );
    m_globalPreferences->addCategory( "network", QT_TRANSLATE_NOOP( "Settings", "Network" ), QIcon( ":/images/network" ) );
}

void
MainWindow::loadGlobalProxySettings()
{
    if( VLMC_GET_BOOL( "network/UseProxy" ) )
    {
        /* Updates Global Proxy for VLMC */
        QNetworkProxy proxy;
        proxy.setType( QNetworkProxy::HttpProxy );
        proxy.setHostName( VLMC_GET_STRING( "network/ProxyHost" ) );
        proxy.setPort( VLMC_GET_STRING( "network/ProxyPort" ).toInt() );
        proxy.setUser( VLMC_GET_STRING( "network/ProxyUsername" ) );
        proxy.setPassword( VLMC_GET_STRING( "network/ProxyPassword" ) );
        QNetworkProxy::setApplicationProxy( proxy );
        return;
    }

    QNetworkProxy::setApplicationProxy( QNetworkProxy::NoProxy );
}

void
MainWindow::createProjectPreferences()
{
    delete m_projectPreferences;
    m_projectPreferences = new SettingsDialog( Project::getInstance()->settings(), tr( "Project preferences" ), this );
    m_projectPreferences->addCategory( "general", QT_TRANSLATE_NOOP( "Settings", "General" ), QIcon( ":/images/vlmc" ) );
    m_projectPreferences->addCategory( "video", QT_TRANSLATE_NOOP( "Settings", "Video" ), QIcon( ":/images/video" ) );
    m_projectPreferences->addCategory( "audio", QT_TRANSLATE_NOOP( "Settings", "Audio" ), QIcon( ":/images/audio" ) );
}

void
MainWindow::checkFolders()
{
    QDir dirUtil;
    if ( !dirUtil.exists( VLMC_GET_STRING( "vlmc/DefaultProjectLocation" ) ) )
        dirUtil.mkdir( VLMC_GET_STRING( "vlmc/DefaultProjectLocation" ) );

    if ( !dirUtil.exists( VLMC_GET_STRING( "vlmc/TempFolderLocation" ) ) )
        dirUtil.mkdir( VLMC_GET_STRING( "vlmc/TempFolderLocation" ) );
}

void
MainWindow::clearTemporaryFiles()
{
    QDir dirUtil;
    if( dirUtil.cd( VLMC_GET_STRING( "vlmc/TempFolderLocation" ) ) )
        foreach ( const QString &file, dirUtil.entryList( QDir::Files ) )
            dirUtil.remove( file );
}

//Private slots definition

void
MainWindow::on_actionQuit_triggered()
{
    saveSettings();
    QApplication::quit();
}

void
MainWindow::on_actionPreferences_triggered()
{
   m_globalPreferences->show();
}

void
MainWindow::on_actionAbout_triggered()
{
    About::getInstance()->exec();
}

bool
MainWindow::checkVideoLength()
{
    if ( Project::getInstance()->workflow()->getLengthFrame() <= 0 )
    {
        QMessageBox::warning( NULL, tr ( "VLMC Renderer" ), tr( "There is nothing to render." ) );
        return false;
    }
    return true;
}

bool
MainWindow::renderVideo( const QString& outputFileName, quint32 width, quint32 height, double fps, quint32 vbitrate, quint32 abitrate )
{
    if ( m_fileRenderer )
        delete m_fileRenderer;
    m_fileRenderer = new WorkflowFileRenderer( m_backend );

    WorkflowFileRendererDialog  *dialog = new WorkflowFileRendererDialog( m_fileRenderer, width, height );
    dialog->setModal( true );
    dialog->setOutputFileName( outputFileName );

    m_fileRenderer->initializeRenderer();
    m_fileRenderer->run( outputFileName, width, height, fps, vbitrate, abitrate );

    if ( dialog->exec() == QDialog::Rejected )
    {
        delete dialog;
        return false;
    }

    delete dialog;
    return true;
}

bool
MainWindow::renderVideoSettings( bool shareOnInternet )
{
    RendererSettings *settings = new RendererSettings( shareOnInternet );

    if ( settings->exec() == QDialog::Rejected )
    {
        delete settings;
        return false;
    }

    QString     outputFileName = settings->outputFileName();
    quint32     width          = settings->width();
    quint32     height         = settings->height();
    double      fps            = settings->fps();
    quint32     vbitrate       = settings->videoBitrate();
    quint32     abitrate       = settings->audioBitrate();

    delete settings;

    return renderVideo( outputFileName, width, height, fps, vbitrate, abitrate );
}

void
MainWindow::on_actionRender_triggered()
{
    if ( checkVideoLength() )
    {
        m_renderer->stop();
        //Setup dialog box for querying render parameters.
        renderVideoSettings( false );
    }
}

void
MainWindow::on_actionShare_On_Internet_triggered()
{
    if ( checkVideoLength() )
    {
        m_renderer->stop();

        if( !renderVideoSettings( true ) )
            return;

        checkFolders();
        QString fileName = VLMC_GET_STRING( "vlmc/TempFolderLocation" ) + "/" +
                           VLMC_PROJECT_GET_STRING( "vlmc/ProjectName" ) +
                           "-vlmc.mp4";

        loadGlobalProxySettings();

        ShareOnInternet *shareVideo = new ShareOnInternet();
        shareVideo->setVideoFile( fileName );

        if ( shareVideo->exec() == QDialog::Rejected )
        {
            delete shareVideo;
            return;
        }

        delete shareVideo;
    }
}

void
MainWindow::on_actionNew_Project_triggered()
{
    m_wizard->restart();
    m_wizard->show();
}

void
MainWindow::on_actionHelp_triggered()
{
    QDesktopServices::openUrl( QUrl( "http://videolan.org/vlmc" ) );
}

void
MainWindow::zoomIn()
{
    m_zoomSlider->setValue( m_zoomSlider->value() - 1 );
}

void
MainWindow::zoomOut()
{
    m_zoomSlider->setValue( m_zoomSlider->value() + 1 );
}

void
MainWindow::on_actionFullscreen_triggered( bool checked )
{
    if ( checked && !isFullScreen() )
    {
        setUnifiedTitleAndToolBarOnMac( false );
        showFullScreen();
    }
    else
    {
        setUnifiedTitleAndToolBarOnMac( true );
        showNormal();
    }
}

void
MainWindow::registerWidgetInWindowMenu( QDockWidget* widget )
{
    m_ui.menuWindow->addAction( widget->toggleViewAction() );
}

void
MainWindow::toolButtonClicked( QAction *action )
{
    if ( action == m_ui.actionSelection_mode )
        emit toolChanged( TOOL_DEFAULT );
    else if ( action == m_ui.actionCut_mode )
        emit toolChanged( TOOL_CUT );
    else
        vlmcCritical() << "Unknown tool. This should not happen !";
}

void
MainWindow::on_actionProject_Preferences_triggered()
{
  m_projectPreferences->show();
}

bool
MainWindow::saveSettings()
{
    // ??????
    clearTemporaryFiles();
    Settings* settings = Core::getInstance()->settings();
    // Save the current geometry
    settings->setValue( "private/MainWindowGeometry", saveGeometry() );
    // Save the current layout
    settings->setValue( "private/MainWindowState", saveState() );
    settings->setValue( "private/CleanQuit", true );
    settings->save();
    Project::getInstance()->save();
    return true;
}

void
MainWindow::closeEvent( QCloseEvent* e )
{
    if ( saveSettings() )
        e->accept();
    else
        e->ignore();
}

void
MainWindow::projectUpdated( const QString& projectName )
{
    QString title = tr( "VideoLAN Movie Creator" );
    title += " - ";
    title += projectName;
    setWindowTitle( title );
}

void
MainWindow::cleanStateChanged( bool isClean )
{
    QString title = windowTitle();
    if ( isClean == true )
        title.replace(" *", "");
    else
        title += " *";
    setWindowTitle( title );
}

void
MainWindow::on_actionUndo_triggered()
{
    Project::getInstance()->undoStack()->undo();
}

void
MainWindow::on_actionRedo_triggered()
{
    Project::getInstance()->undoStack()->redo();
}

void
MainWindow::on_actionCrash_triggered()
{
    //WARNING: read this part at your own risk !!
    //I'm not responsible if you puke while reading this :D
    QString str;
    int test = 1 / str.length();
    Q_UNUSED( test );
}

bool
MainWindow::restoreSession()
{
    bool    cleanQuit = VLMC_GET_BOOL( "private/CleanQuit" );
    bool    ret = false;

    if ( cleanQuit == false )
    {
        QMessageBox::StandardButton res = QMessageBox::question( this, tr( "Crash recovery" ), tr( "VLMC didn't closed nicely. Do you want to recover your project?" ),
                               QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes );
        if ( res == QMessageBox::Yes )
        {
            if ( Project::getInstance()->loadEmergencyBackup() == true )
                ret = true;
            else
                QMessageBox::warning( this, tr( "Can't restore project" ), tr( "VLMC didn't manage to restore your project. We apology for the inconvenience" ) );
        }
    }
    Core::getInstance()->settings()->setValue( "private/CleanQuit", true );
    return ret;
}

void
MainWindow::on_actionImport_triggered()
{
    m_importController->exec();
}

void
MainWindow::canUndoChanged( bool canUndo )
{
    m_ui.actionUndo->setEnabled( canUndo );
}

void
MainWindow::canRedoChanged( bool canRedo )
{
    m_ui.actionRedo->setEnabled( canRedo );
}

void
MainWindow::onProjectLoaded(Project* project)
{
    createProjectPreferences();
    connect( project, SIGNAL( projectUpdated( const QString&, bool ) ), this, SLOT( projectUpdated( const QString&, bool ) ) );

    // Undo/Redo
    connect( project->undoStack(), SIGNAL( canUndoChanged( bool ) ), this, SLOT( canUndoChanged( bool ) ) );
    connect( project->undoStack(), SIGNAL( canRedoChanged( bool ) ), this, SLOT( canRedoChanged( bool ) ) );
    canUndoChanged( project->undoStack()->canUndo() );
    canRedoChanged( project->undoStack()->canRedo() );

    const ClipRenderer* clipRenderer = qobject_cast<const ClipRenderer*>( m_clipPreview->getGenericRenderer() );
    connect( project->library(), SIGNAL( clipRemoved( const QUuid& ) ), clipRenderer, SLOT( clipUnloaded( const QUuid& ) ) );
}

#ifdef WITH_CRASHBUTTON
void
MainWindow::setupCrashTester()
{
    QAction* actionCrash = new QAction( this );
    actionCrash->setObjectName( QString::fromUtf8( "actionCrash" ) );
    m_ui.menuTools->addAction( actionCrash );
    actionCrash->setText( QApplication::translate( "MainWindow", "Crash", 0, QApplication::UnicodeUTF8 ) );
    connect( actionCrash, SIGNAL( triggered( bool ) ), this, SLOT( on_actionCrash_triggered() ) );
}
#endif

