#include "WidgetTarget.h"

#include "Backend/ISourceRenderer.h"

using namespace Backend;

WidgetTarget::WidgetTarget( void* id )
    : m_id( reinterpret_cast<quintptr>( id ) )
{
}

void
WidgetTarget::configure( ISourceRenderer *renderer )
{
    renderer->setOutputWidget( reinterpret_cast<void*>( m_id ) );
}
