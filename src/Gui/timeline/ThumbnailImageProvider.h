#ifndef THUMBNAILIMAGEPROVIDER_H
#define THUMBNAILIMAGEPROVIDER_H

#include <QMap>
#include <QQuickImageProvider>

class ThumbnailImageProvider : public QObject, public QQuickImageProvider
{
    Q_OBJECT

public:
    ThumbnailImageProvider();

    virtual QPixmap requestPixmap( const QString& id, QSize* size, const QSize& requestedSize ) override;

public slots:
    bool    hasImage( const QString& uuid, quint32 pos );

signals:
    void    imageReady( const QString& uuid, quint32 pos );

private:
    // uuid/pos, pixmap
    QMap<QString, QPixmap>  m_pixMap;
};

#endif // THUMBNAILIMAGEPROVIDER_H
