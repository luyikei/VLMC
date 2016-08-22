#include "ThumbnailWorker.h"

#include <QPixmap>
#include <QUuid>

#include "Backend/MLT/MLTInput.h"

ThumbnailWorker::ThumbnailWorker( const QString& uuid, const QString& filePath,
                                  qint64 pos, quint32 width, quint32 height, QObject* parent )
    : QObject( parent )
    , m_uuid( uuid )
    , m_filePath( filePath )
    , m_pos( pos )
    , m_width( width )
    , m_height( height )
{

}

void
ThumbnailWorker::run()
{
    Backend::MLT::MLTInput input( qPrintable( m_filePath ) );
    input.setPosition( m_pos );
    auto image = input.image( m_width, m_height );
    QImage qImg( image, m_width, m_height,
                QImage::Format_RGBA8888, []( void* buf ){ delete[] (uchar*) buf; } );
    auto qPix = QPixmap::fromImage( qImg );

    // Use Qt::DirectConnection or the pixmap will be deleted!
    emit imageReady( m_uuid, m_pos, qPix );
}
