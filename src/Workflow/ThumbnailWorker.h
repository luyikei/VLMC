#ifndef THUMBNAILWORKER_H
#define THUMBNAILWORKER_H

#include <QObject>

class QPixmap;

class ThumbnailWorker : public QObject
{
    Q_OBJECT
public:
    explicit ThumbnailWorker( QObject* parent = 0 );

signals:
    void    imageReady( const QString& uuid, qint64 pos, const QPixmap& pixmap );

public slots:
    void    run( const QString& uuid, const QString& filePath, qint64 pos, quint32 width, quint32 height );
};

#endif // THUMBNAILWORKER_H
