/*****************************************************************************
 * MainWindow.cpp: VLMC MainWindow
 *****************************************************************************
 * Copyright (C) 2008-2010 VideoLAN
 *
 * Authors: Ludovic Fauvet <etix@l0cal.com>
 *          Hugo Beauzée-Luyssen <beauze.h@gmail.com>
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

#include <QSizePolicy>
#include <QDockWidget>
#include <QFileDialog>
#include <QSlider>
#include <QMessageBox>
#include <QDesktopServices>
#include <QUrl>
#include <QNetworkProxy>
#include <QSettings>
#include <QDebug>

#include "MainWindow.h"
#include "config.h"
#include "Library.h"
#include "About.h"
#include "VlmcDebug.h"

#include "EffectsEngine/EffectsEngine.h"
#include "MainWorkflow.h"
#include "export/RendererSettings.h"
#include "export/ShareOnInternet.h"
#include "WorkflowFileRenderer.h"
#include "WorkflowRenderer.h"
#include "ClipRenderer.h"

/* Widgets */
#include "DockWidgetManager.h"
#include "EffectsListView.h"
#include "ImportController.h"
#include "MediaLibrary.h"
#include "NotificationZone.h"
#include "PreviewWidget.h"
#include "timeline/Timeline.h"
#include "timeline/TracksView.h"
#include "UndoStack.h"

/* Settings / Preferences */
#include "project/GuiProjectManager.h"
#include "ProjectWizard.h"
#include "Settings.h"
#include "SettingsManager.h"
#include "LanguageHelper.h"

/* VLCpp */
#include "VLCInstance.h"

MainWindow::MainWindow( QWidget *parent ) :
    QMainWindow( parent ), m_fileRenderer( NULL )
{
    m_ui.setupUi( this );

    //We only install message handler here cause it uses configuration.
    VlmcDebug::getInstance()->setup();

    //VLC Instance:
    LibVLCpp::Instance::getInstance();

    //Creating the project manager first (so it can create all the project variables)
    GUIProjectManager::getInstance();

    //Preferences
    initVlmcPreferences();

    // GUI
    DockWidgetManager::getInstance( this )->setMainWindow( this );
    createGlobalPreferences();
    createProjectPreferences();
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

    connect( GUIProjectManager::getInstance(), SIGNAL( projectUpdated( const QString&, bool ) ),
             this, SLOT( projectUpdated( const QString&, bool ) ) );

    // Undo/Redo
    connect( UndoStack::getInstance( this ), SIGNAL( canRedoChanged( bool ) ),
             this, SLOT( canRedoChanged( bool ) ) );
    connect( UndoStack::getInstance( this ), SIGNAL( canUndoChanged( bool ) ),
             this, SLOT( canUndoChanged( bool ) ) );
    canRedoChanged( UndoStack::getInstance( this )->canRedo() );
    canUndoChanged( UndoStack::getInstance( this )->canUndo() );

    //Connecting Library stuff:
    const ClipRenderer* clipRenderer = qobject_cast<const ClipRenderer*>( m_clipPreview->getGenericRenderer() );
    Q_ASSERT( clipRenderer != NULL );
    connect( m_mediaLibrary, SIGNAL( clipSelected( Clip* ) ),
             clipRenderer, SLOT( setClip( Clip* ) ) );
    connect( m_mediaLibrary, SIGNAL( importRequired() ),
             this, SLOT( on_actionImport_triggered() ) );
    connect( Library::getInstance(), SIGNAL( clipRemoved( const QUuid& ) ),
             clipRenderer, SLOT( clipUnloaded( const QUuid& ) ) );

    // Wizard
    m_pWizard = new ProjectWizard( this );
    m_pWizard->setModal( true );


#ifdef WITH_CRASHHANDLER
    if ( restoreSession() == true )
        return ;
#endif
    QSettings s;

    // Restore the geometry
    restoreGeometry( s.value( "MainWindowGeometry" ).toByteArray() );
    // Restore the layout
    restoreState( s.value( "MainWindowState" ).toByteArray() );

    if ( s.value( "ShowWizardStartup", true ).toBool() )
    {
        //If a project was opened from the command line: don't show the wizard.
        if ( qApp->arguments().size() == 1 )
            m_pWizard->show();
    }
}

MainWindow::~MainWindow()
{
    if ( m_fileRenderer )
        delete m_fileRenderer;
    delete m_importController;
    LibVLCpp::Instance::destroyInstance();
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
    VLMC_CREATE_PREFERENCE_LANGUAGE( "general/VLMCLang", "default",
                                     QT_TRANSLATE_NOOP( "PreferenceWidget", "Language" ),
                                     QT_TRANSLATE_NOOP( "PreferenceWidget", "The VLMC's UI language" ) );

    SettingsManager::getInstance()->watchValue( "general/VLMCLang",
                                                LanguageHelper::getInstance(),
                                                SLOT( languageChanged( const QVariant& ) ),
                                                SettingsManager::Vlmc );

    //Setup VLMC General Preferences...
    VLMC_CREATE_PREFERENCE_BOOL( "general/ConfirmDeletion", true,
                                 QT_TRANSLATE_NOOP( "PreferenceWidget", "Confirm clip deletion"),
                                 QT_TRANSLATE_NOOP( "PreferenceWidget", "Ask for confirmation before deleting a clip from the timeline" ) );

    VLMC_CREATE_PREFERENCE_PATH( "general/DefaultProjectLocation", QDir::homePath(),
                                    QT_TRANSLATE_NOOP( "PreferencesWidget", "Project default location" ),
                                    QT_TRANSLATE_NOOP( "PreferenceWidget", "The default location where to store projects folders" ) );

    VLMC_CREATE_PREFERENCE_PATH( "general/TempFolderLocation", QDir::tempPath() + "/VLMC/",
                                    QT_TRANSLATE_NOOP( "PreferencesWidget", "Temporary folder" ),
                                    QT_TRANSLATE_NOOP( "PreferenceWidget", "The temporary folder used by VLMC to process videos." ) );

    //Setup VLMC Youtube Preference...
    VLMC_CREATE_PREFERENCE_STRING( "youtube/DeveloperKey", "AI39si7FOtp165Vq644xVkuka84TVQNbztQmQ1dC9stheBfh3-33RZaTu7eJkYJzvxp6XNbvlr4M6-ULjXDERFl62WIo6AQIEQ",
                                     QT_TRANSLATE_NOOP( "PreferenceWidget", "Youtube Developer Key" ),
                                     QT_TRANSLATE_NOOP( "PreferenceWidget", "VLMC's Youtube Developer Key" ) );

    VLMC_CREATE_PREFERENCE_STRING( "youtube/Username", "username",
                                     QT_TRANSLATE_NOOP( "PreferenceWidget", "Youtube Username" ),
                                     QT_TRANSLATE_NOOP( "PreferenceWidget", "Valid YouTube username" ) );

    VLMC_CREATE_PREFERENCE_PASSWORD( "youtube/Password", "",
                                     QT_TRANSLATE_NOOP( "PreferenceWidget", "Youtube Password" ),
                                     QT_TRANSLATE_NOOP( "PreferenceWidget", "Leave this field, password will be stored in unencryped form." ) );

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

    //Load saved preferences :
    QSettings       s;
    if ( s.value( "VlmcVersion" ).toString() != PROJECT_VERSION_MAJOR )
        s.clear();
    else
    {
        loadVlmcPreferences( "keyboard" );
        loadVlmcPreferences( "general" );
        loadVlmcPreferences( "youtube" );
        loadVlmcPreferences( "network" );
    }
    s.setValue( "VlmcVersion", PROJECT_VERSION_MAJOR );
}

void
MainWindow::loadVlmcPreferences( const QString &subPart )
{
    QSettings       s;
    s.beginGroup( subPart );
    foreach ( QString key, s.allKeys() )
    {
        SettingsManager::getInstance()->setValue( subPart + "/" + key, s.value( key ),
                                                  SettingsManager::Vlmc );
    }
}

#undef CREATE_MENU_SHORTCUT

void
MainWindow::on_actionSave_triggered()
{
    GUIProjectManager::getInstance()->saveProject();
}

void
MainWindow::on_actionSave_As_triggered()
{
    GUIProjectManager::getInstance()->saveProject( true );
}

void
MainWindow::on_actionLoad_Project_triggered()
{
    GUIProjectManager::getInstance()->loadProject();
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
    m_renderer = new WorkflowRenderer();
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
    QWidget         *undoRedoWidget = UndoStack::getInstance( dockedWidget );
    DockWidgetManager::getInstance()->addDockedWidget( dockedWidget, undoRedoWidget, Qt::LeftDockWidgetArea );
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
    EffectsEngine::getInstance()->loadEffects();
    DockWidgetManager::getInstance()->addDockedWidget( dockedWidget, m_effectsList, Qt::LeftDockWidgetArea );
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
    m_globalPreferences = new Settings( SettingsManager::Vlmc, this );
    m_globalPreferences->addCategory( QT_TRANSLATE_NOOP( "Settings", "general" ), SettingsManager::Vlmc,
                                     QIcon( ":/images/vlmc" ),
                                     tr ( "General" ) );
    m_globalPreferences->addCategory( QT_TRANSLATE_NOOP( "Settings", "keyboard" ), SettingsManager::Vlmc,
                                     QIcon( ":/images/keyboard" ),
                                     tr( "Keyboard" ) );
    m_globalPreferences->addCategory( QT_TRANSLATE_NOOP( "Settings", "youtube" ), SettingsManager::Vlmc,
                                     QIcon( ":/images/youtube" ),
                                     tr( "Youtube" ) );
    m_globalPreferences->addCategory( QT_TRANSLATE_NOOP( "Settings", "network" ), SettingsManager::Vlmc,
                                     QIcon( ":/images/network" ),
                                     tr( "Network" ) );
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
    m_projectPreferences = new Settings( SettingsManager::Project, this );
    m_projectPreferences->addCategory( QT_TRANSLATE_NOOP( "Settings", "general" ), SettingsManager::Project,
                                   QIcon( ":/images/vlmc" ),
                                   tr ( "General" ) );
    m_projectPreferences->addCategory( QT_TRANSLATE_NOOP( "Settings", "video" ), SettingsManager::Project,
                                   QIcon( ":/images/video" ),
                                   tr ( "Video" ) );
    m_projectPreferences->addCategory( QT_TRANSLATE_NOOP( "Settings", "audio" ), SettingsManager::Project,
                                   QIcon( ":/images/audio" ),
                                   tr ( "Audio" ) );
}

void
MainWindow::checkFolders()
{
    QDir dirUtil;
    if ( !dirUtil.exists( VLMC_GET_STRING( "general/DefaultProjectLocation" ) ) )
        dirUtil.mkdir( VLMC_GET_STRING( "general/DefaultProjectLocation" ) );

    if ( !dirUtil.exists( VLMC_GET_STRING( "general/TempFolderLocation" ) ) )
        dirUtil.mkdir( VLMC_GET_STRING( "general/TempFolderLocation" ) );
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
    if ( MainWorkflow::getInstance()->getLengthFrame() <= 0 )
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
    m_fileRenderer = new WorkflowFileRenderer();

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
        m_renderer->killRenderer();
        //Setup dialog box for querying render parameters.
        renderVideoSettings( false );
    }
}

void
MainWindow::on_actionShare_On_Internet_triggered()
{
    if ( checkVideoLength() )
    {
        m_renderer->killRenderer();

        if( !renderVideoSettings( true ) )
            return;

        checkFolders();
        QString fileName = VLMC_GET_STRING( "general/TempFolderLocation" ) + "/" +
                           VLMC_PROJECT_GET_STRING( "general/ProjectName" ) +
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
    m_pWizard->restart();
    m_pWizard->show();
}

void
MainWindow::on_actionHelp_triggered()
{
    QDesktopServices::openUrl( QUrl( "http://vlmc.org" ) );
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
    if ( checked )
        showFullScreen();
    else
        showNormal();
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
        qCritical() << "Unknown tool. This should not happen !";
}

void
MainWindow::on_actionProject_Preferences_triggered()
{
  m_projectPreferences->show();
}

bool
MainWindow::saveSettings()
{
    GUIProjectManager   *pm = GUIProjectManager::getInstance();
    if ( pm->askForSaveIfModified() )
    {
        QSettings s;
        // Save the current geometry
        s.setValue( "MainWindowGeometry", saveGeometry() );
        // Save the current layout
        s.setValue( "MainWindowState", saveState() );
        s.setValue( "CleanQuit", true );
        s.sync();
        return true;
    }
    return false;
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
MainWindow::projectUpdated( const QString& projectName, bool savedStatus )
{
    QString title = tr( "VideoLAN Movie Creator" );
    title += " - ";
    title += projectName;
    if ( savedStatus == false )
        title += " *";
    setWindowTitle( title );
}

void
MainWindow::on_actionClose_Project_triggered()
{
    GUIProjectManager::getInstance()->closeProject();
}

void
MainWindow::on_actionUndo_triggered()
{
    UndoStack::getInstance( this )->undo();
}

void
MainWindow::on_actionRedo_triggered()
{
    UndoStack::getInstance( this )->redo();
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
    QSettings   s;
    bool        fileCreated = false;
    bool        ret = false;

    fileCreated = s.contains( "VlmcVersion" );
    if ( fileCreated == true )
    {
        bool        cleanQuit = s.value( "CleanQuit", true ).toBool();

        if ( cleanQuit == false )
        {
            QMessageBox::StandardButton res = QMessageBox::question( this, tr( "Crash recovery" ), tr( "VLMC didn't closed nicely. Do you want to recover your project?" ),
                                   QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes );
            if ( res == QMessageBox::Yes )
            {
                if ( GUIProjectManager::getInstance()->loadEmergencyBackup() == true )
                    ret = true;
                else
                    QMessageBox::warning( this, tr( "Can't restore project" ), tr( "VLMC didn't manage to restore your project. We apology for the inconvenience" ) );
            }
        }
    }
    s.setValue( "CleanQuit", false );
    s.sync();
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

