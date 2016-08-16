#include "ThumbnailWorker.h"

#include <QPixmap>
#include <QUuid>

#include "Backend/MLT/MLTInput.h"

ThumbnailWorker::ThumbnailWorker( QObject* parent )
    : QObject( parent )
{

}

void
ThumbnailWorker::run( const QString& uuid, const QString& filePath, qint64 pos, quint32 width, quint32 height )
{
    Backend::MLT::MLTInput input( qPrintable( filePath ) );
    input.setPosition( pos );
    auto image = input.image( width, height );
    QImage qImg( image, width, height,
                QImage::Format_RGBA8888, []( void* buf ){ delete[] (uchar*) buf; } );
    auto qPix = QPixmap::fromImage( qImg );

    // Use Qt::DirectConnection or the pixmap will be deleted!
    emit imageReady( uuid, pos, qPix );
}
