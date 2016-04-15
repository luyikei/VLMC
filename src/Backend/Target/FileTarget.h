#ifndef FILETARGET_H
#define FILETARGET_H

#include "Backend/IRenderTarget.h"

namespace Backend
{
    class FileTarget : public IRenderTarget
    {
    public:
        FileTarget( const char* filePath );
        ~FileTarget();
        virtual void configure( ISourceRenderer *renderer ) override;
    private:
        const char* m_filePath;
    };
}

#endif // FILETARGET_H
