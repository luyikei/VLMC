#include "ThumbnailImageProvider.h"

#include "Workflow/MainWorkflow.h"
#include "Main/Core.h"

ThumbnailImageProvider::ThumbnailImageProvider()
    : QQuickImageProvider( QQuickImageProvider::Pixmap )
{
    connect( Core::instance()->workflow(), &MainWorkflow::thumbnailUpdated, this, [this]
             ( const QString& uuid, quint32 pos, const QPixmap& pixmap )
    {
        auto id = uuid + "/" + QString::number( pos );

        auto it = m_pixMap.find( id );
        if ( it == m_pixMap.end() )
            m_pixMap.insert( id, pixmap );

        emit imageReady( uuid, pos );
    }, Qt::DirectConnection );
}

QPixmap
ThumbnailImageProvider::requestPixmap( const QString& id, QSize* size, const QSize& requestedSize )
{
    QString tmp = id;
    tmp.replace( "%7B", "{" );
    tmp.replace( "%7D", "}" );

    auto it = m_pixMap.find( tmp );
    if ( it == m_pixMap.end() )
    {
        *size = QSize( requestedSize.width(), requestedSize.height() );
        return QPixmap( requestedSize );
    }

    auto pixmap = it.value();
    *size = pixmap.size();
    if ( requestedSize.isValid() == true )
        return pixmap.scaled( requestedSize );
    return pixmap;
}

bool
ThumbnailImageProvider::hasImage( const QString& uuid, quint32 pos )
{
    return m_pixMap.find( uuid + "/" + QString::number( pos ) ) != m_pixMap.end();
}
