#include "EffectsListView.h"
#include "EffectsEngine/EffectsEngine.h"

#include <QApplication>
#include <QMouseEvent>
#include <QStandardItem>

#include <QtDebug>

EffectsListView::EffectsListView(QWidget *parent) :
    QListView(parent)
{
    m_model = new QStandardItemModel( this );
    connect( EffectsEngine::getInstance(), SIGNAL( effectAdded( Effect*, Effect::Type ) ),
             this, SLOT( effectAdded(Effect*,Effect::Type) ) );
    setModel( m_model );
    setEditTriggers( QListView::NoEditTriggers );
    setDragEnabled( true );
    setSelectionMode( QAbstractItemView::MultiSelection );
}

void
EffectsListView::effectAdded( Effect *effect, Effect::Type type )
{
    if ( type == m_type )
        m_model->appendRow( new QStandardItem( effect->name() ) );
}

void
EffectsListView::setType( Effect::Type type )
{
    m_type = type;
}

void
EffectsListView::mousePressEvent( QMouseEvent *event )
{
    QListView::mousePressEvent( event );

    if ( ( event->buttons() | Qt::LeftButton ) == Qt::LeftButton )
        m_dragStartPos = event->pos();
}

void
EffectsListView::mouseMoveEvent( QMouseEvent *event )
{
    if ( ( event->buttons() | Qt::LeftButton ) != Qt::LeftButton )
         return;

    if ( ( event->pos() - m_dragStartPos ).manhattanLength()
          < QApplication::startDragDistance() )
        return;

    QMimeData* mimeData = new QMimeData;
    mimeData->setData( "vlmc/effect_name", m_model->data( currentIndex() ).toByteArray() );
    QDrag* drag = new QDrag( this );
    drag->setMimeData( mimeData );
    drag->exec( Qt::CopyAction | Qt::MoveAction, Qt::CopyAction );
}
