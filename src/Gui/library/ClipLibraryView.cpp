
#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "ClipLibraryView.h"

#include "Library/Library.h"
#include "Main/Core.h"
#include "Media/Clip.h"
#include "Media/Media.h"
#include "Tools/VlmcDebug.h"

#include <QUuid>
#include <QDrag>
#include <QMimeData>
#include <QtQuick/QQuickView>
#include <QtQml/QQmlContext>

ClipLibraryView::ClipLibraryView( QWidget* parent )
    : QObject( parent )
{
    setObjectName( QStringLiteral( "Clip Library" ) );
    auto view = new QQuickView;
    m_container = QWidget::createWindowContainer( view, parent );
    m_container->setMinimumSize( 100, 1 );
    m_container->setObjectName( objectName() );

    view->rootContext()->setContextProperty( QStringLiteral( "view" ), this );
    view->setSource( QUrl( QStringLiteral( "qrc:/QML/ClipLibraryView.qml" ) ) );
    view->setResizeMode( QQuickView::SizeRootObjectToView );

    connect( Core::instance()->library(), &Library::clipAdded, this, &ClipLibraryView::onClipAdded );
    connect( Core::instance()->library(), &Library::clipRemoved, this, &ClipLibraryView::clipRemoved );
}

QWidget*
ClipLibraryView::container()
{
    return m_container;
}

QJsonObject
ClipLibraryView::clip( const QString& uuid )
{
    auto clip = Core::instance()->library()->clip( uuid );
    return QJsonObject{
        { "uuid", uuid },
        { "isBaseClip", clip->media()->baseClip() == clip },
        { "title", clip->media()->title() },
        { "thumbnailPath", clip->media()->snapshot() },
        { "mediaId", clip->media()->id() },
        { "duration", clip->length() },
        { "onTimeline", clip->onTimeline() },
    };
}

void
ClipLibraryView::onClipAdded( const QString& uuid )
{
    auto clip = Core::instance()->library()->clip( uuid );
    connect( clip.data(), &Clip::onTimelineChanged, this,
             [this, clip]( bool onTimeline )
    {
        emit clipOnTimelineChanged( clip->uuid().toString(), onTimeline );
    } );
    emit clipAdded( uuid );
}

void
ClipLibraryView::onClipSelected( const QString& uuid )
{
    emit clipSelected( uuid );
}

void
ClipLibraryView::startDrag( const QString& uuid )
{
    auto clip = Core::instance()->library()->clip( uuid );
    if ( !clip )
    {
        vlmcCritical() << "Couldn't find a clip:" << uuid;
        return;
    }

    QDrag* drag = new QDrag( this );
    QMimeData* mimeData = new QMimeData;

    mimeData->setData( QStringLiteral( "vlmc/uuid" ), clip->uuid().toByteArray() );

    drag->setMimeData( mimeData );
    auto thumbnailPath = clip->media()->snapshot();
    drag->setPixmap( QPixmap( thumbnailPath.isEmpty() ? QStringLiteral( ":/images/vlmc" ) :
                                thumbnailPath ).scaled( 100, 100, Qt::KeepAspectRatio ) );
    drag->exec();
}
