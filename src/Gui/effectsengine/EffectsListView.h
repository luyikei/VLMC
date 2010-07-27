#ifndef EFFECTLISTVIEW_H
#define EFFECTLISTVIEW_H

#include "EffectsEngine/Effect.h"

#include <QListView>

class   QStandardItemModel;

class EffectsListView : public QListView
{
    Q_OBJECT

    public:
        explicit            EffectsListView(QWidget *parent = 0);
        void                setType( Effect::Type type );

    protected:
        void                mousePressEvent( QMouseEvent *event );
        void                mouseMoveEvent( QMouseEvent *event );

    private:
        QStandardItemModel  *m_model;
        Effect::Type        m_type;
        QPoint              m_dragStartPos;
    public slots:
        void                effectAdded( Effect *effect, const QString& name,
                                         Effect::Type type );
};

#endif // EFFECTLISTVIEW_H
