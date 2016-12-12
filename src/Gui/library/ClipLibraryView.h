#ifndef CLIPLIBRARYVIEW_H
#define CLIPLIBRARYVIEW_H

#include <QWidget>
#include <QJsonObject>

class ClipLibraryView : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY( ClipLibraryView )

    public:
        explicit ClipLibraryView( QWidget* parent = 0 );

        QWidget*    container();

        Q_INVOKABLE
        QJsonObject clip( const QString& uuid );

    public slots:
        void    onClipAdded( const QString& uuid );
        void    onClipSelected( const QString& uuid );
        void    startDrag( const QString& uuid );

    private:
        QWidget*    m_container;

    signals:
        void    clipAdded( const QString& uuid );
        void    clipRemoved( const QString& uuid );

        void    clipSelected( const QString& uuid );
        void    clipOnTimelineChanged( const QString& uuid, bool onTimeline );
};

#endif // CLIPLIBRARYVIEW_H
