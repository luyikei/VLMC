#include "EffectsListView.h"
#include "EffectsEngine.h"
#include "EffectWidget.h"

#include <QApplication>
#include <QDialog>
#include <QMouseEvent>
#include <QStandardItem>
#include <QVBoxLayout>

#include <QtDebug>

EffectsListView::EffectsListView(QWidget *parent) :
    QListView(parent)
{
    m_model = new QStandardItemModel( this );
    connect( EffectsEngine::getInstance(),
             SIGNAL( effectAdded( Effect*, const QString&, Effect::Type ) ),
             this,
             SLOT( effectAdded(Effect*, const QString&, Effect::Type) ) );
    setModel( m_model );
    connect( this, SIGNAL( activated( QModelIndex ) ),
             this, SLOT( effectActivated( QModelIndex ) ) );
}

void
EffectsListView::effectAdded( Effect *, const QString& name, Effect::Type type )
{
    if ( type == m_type )
        m_model->appendRow( new QStandardItem( name ) );
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

void
EffectsListView::effectActivated( const QModelIndex &index ) const
{
    if ( index.isValid() == false )
        return ;
    Effect  *effect = EffectsEngine::getInstance()->effect( m_model->data( index, Qt::DisplayRole ).toString() );
    QDialog         *dialog = new QDialog();
    QVBoxLayout     *layout = new QVBoxLayout( dialog );
    EffectWidget    *wid = new EffectWidget( dialog );
    layout->addWidget( wid );
    wid->setEffect( effect );
    dialog->setWindowTitle( tr( "%1 informations" ).arg( effect->name() ) );
    dialog->exec();
    delete dialog;
}
