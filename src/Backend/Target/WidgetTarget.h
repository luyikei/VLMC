#ifndef WIDGETTARGET_H
#define WIDGETTARGET_H

#include <QtGlobal>
#include "Backend/IRenderTarget.h"

namespace Backend
{
    class WidgetTarget : public IRenderTarget
    {
    public:
        WidgetTarget( void* id );
        virtual void configure( ISourceRenderer *renderer ) override;
    private:
        quintptr m_id;
    };
}

#endif // WIDGETTARGET_H
