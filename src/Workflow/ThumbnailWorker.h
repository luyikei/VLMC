#ifndef THUMBNAILWORKER_H
#define THUMBNAILWORKER_H

#include <QObject>

class QPixmap;

class ThumbnailWorker : public QObject
{
    Q_OBJECT
public:
    explicit ThumbnailWorker( const QString& uuid, const QString& filePath,
                              qint64 begin, qint64 pos, quint32 width,
                              quint32 height , QObject* parent = 0 );

signals:
    void    imageReady( const QString& uuid, qint64 pos, const QPixmap& pixmap );

public slots:
    void    run();

private:
    QString     m_uuid;
    QString     m_filePath;
    qint64      m_begin;
    qint64      m_pos;
    quint32     m_width;
    quint32     m_height;
};

#endif // THUMBNAILWORKER_H
